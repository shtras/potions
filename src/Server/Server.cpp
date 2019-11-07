#include <thread>

#include "spdlog_wrap.h"
#include "rapidjson/document.h"

#include "DB/DB.h"
#include "Utils/Utils.h"

#include "Server.h"

namespace Server
{
void Server::Start()
{
#ifdef DEBUG
    server = std::make_unique<HttpServer>();
#else
    server = std::make_unique<HttpServer>("server.crt", "server.key");
#endif
    server->config.port = 8080;
    server->resource["^/about$"]["GET"] = [&](std::shared_ptr<HttpServer::Response> response,
                                              std::shared_ptr<HttpServer::Request> request) {
        std::stringstream stream;
        stream << "<h1>Request from " << request->remote_endpoint().address().to_string() << ":"
               << request->remote_endpoint().port() << "</h1>";

        stream << request->method << " " << request->path << " HTTP/" << request->http_version;

        stream << "<h2>Query Fields</h2>";
        auto query_fields = request->parse_query_string();
        for (auto& field : query_fields)
            stream << field.first << ": " << field.second << "<br>";
        stream << "<h2>Header Fields</h2>";
        for (auto& field : request->header)
            stream << field.first << ": " << field.second << "<br>";
        response->write(stream);
    };

    server->resource["^/login$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                               std::shared_ptr<HttpServer::Request> request) {
        auto res = login(request.get());
        response->write(res.first, res.second, corsHeader_);
    };

    server->resource["^/game/create$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                     std::shared_ptr<HttpServer::Request> request) {
        createGame(response.get(), request.get());
    };

    server->resource["^/game/delete$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                     std::shared_ptr<HttpServer::Request> request) {
        deleteGame(response.get(), request.get());
    };

    server->resource["^/game/join$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                   std::shared_ptr<HttpServer::Request> request) {
        joinGame(response.get(), request.get());
    };

    server->resource["^/game/list$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                   std::shared_ptr<HttpServer::Request> request) {
        listGames(response.get(), request.get());
    };

    server->resource["^/game/start"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                   std::shared_ptr<HttpServer::Request> request) {
        startGame(response.get(), request.get());
    };

    server->resource["^/game/query"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                   std::shared_ptr<HttpServer::Request> request) {
        queryGame(response.get(), request.get());
    };

    server->resource["^/game/turn"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                  std::shared_ptr<HttpServer::Request> request) {
        makeTurn(response.get(), request.get());
    };

    server->resource["^/game/lastupdate"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                        std::shared_ptr<HttpServer::Request> request) {
        lastUpdate(response.get(), request.get());
    };

    server->resource["^/game/undo"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                                  std::shared_ptr<HttpServer::Request> request) {
        auto res = undo(request.get());
        response->write(res.first, res.second, corsHeader_);
    };

    server->resource["^/ping$"]["POST"] = [&](std::shared_ptr<HttpServer::Response> response,
                                              std::shared_ptr<HttpServer::Request> request) {
        ping(response.get(), request.get());
    };

    server_thread = std::thread([&]() {
        // Start server
        server->start();
    });
}

void Server::Stop()
{
    server->stop();
    server_thread.join();
}

std::pair<SimpleWeb::StatusCode, std::string> Server::login(HttpServer::Request* request)
{
    auto maybeD = tryParseRequest(request);
    if (!maybeD) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    auto d = (*maybeD).view();
    const auto& user = d["user"];
    if (!user || user.type() != bsoncxx::type::k_utf8) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    bsoncxx::builder::stream::document query;
    query << "user" << user.get_utf8().value;
    auto& db = DB::DB::Instance();
    auto userInfo = db.Get("users", query);
    if (!userInfo) {
        return {SimpleWeb::StatusCode::client_error_unauthorized, ""};
    }
    auto id = createSession((*userInfo).view());
    std::stringstream res;
    res << "{\"session_id\": \"" << id << "\"}";
    return {SimpleWeb::StatusCode::success_ok, res.str()};
}

Session* Server::validateRequest(bsoncxx::document::view& d)
{
    const auto& sessionId = d["session"];
    if (!sessionId || sessionId.type() != bsoncxx::type::k_utf8) {
        return nullptr;
    }
    auto session = getSession(std::string(sessionId.get_utf8().value));
    if (!session) {
        return nullptr;
    }
    return session;
}

Session* Server::validateRequest(HttpServer::Response* response, bsoncxx::document::view& d)
{
    const auto& sessionId = d["session"];
    if (!sessionId || sessionId.type() != bsoncxx::type::k_utf8) {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized, corsHeader_);
        return nullptr;
    }
    auto session = getSession(std::string(sessionId.get_utf8().value));
    if (!session) {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized, corsHeader_);
        return nullptr;
    }
    return session;
}

Session* Server::validateRequest(HttpServer::Response* response, HttpServer::Request* request, rapidjson::Document& d)
{
    d.Parse(request->content.string());
    if (d.HasParseError()) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, corsHeader_);
        return nullptr;
    }
    auto sessionO = Utils::GetT<std::string>(d, "session");
    if (!sessionO) {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized, corsHeader_);
        return nullptr;
    }
    auto session = getSession(*sessionO);
    if (!session) {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized, corsHeader_);
        return nullptr;
    }
    return session;
}

void Server::ping(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    if (!validateRequest(response, request, d)) {
        return;
    }
    response->write("{}", corsHeader_);
}

void Server::startGame(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    auto gameIdO = Utils::GetT<std::string>(d, "game_id");
    if (!gameIdO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    auto& gameId = *gameIdO;
    auto game = findGame(gameId);
    if (!game) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Game not found", corsHeader_);
        return;
    }
    game->Start();
    dumpGame(game);
    response->write("{}", corsHeader_);
}

void Server::createGame(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    if (session->games.size() > 5) {
        response->write(SimpleWeb::StatusCode::client_error_forbidden, "Too many games", corsHeader_);
        return;
    }
    auto nameO = Utils::GetT<std::string>(d, "name");
    if (!nameO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing name", corsHeader_);
        return;
    }
    auto name = *nameO;
    auto game = std::make_unique<Engine::Game>(std::move(name));
    bool ret = game->Init("../res/settings.json");
    if (!ret) {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
        return;
    }
    game->AddPlayer(session->user);
    auto& db = DB::DB::Instance();
    bsoncxx::builder::stream::document gameBson;
    bsoncxx::builder::stream::document bson;
    game->ToJson(gameBson);
    bson << "state" << gameBson << "moves" << bsoncxx::builder::stream::open_array
         << bsoncxx::builder::stream::close_array << "history" << bsoncxx::builder::stream::open_array
         << bsoncxx::builder::stream::close_array;
    auto gameId = db.Insert("games", bsoncxx::to_json(bson));
    if (gameId == "") {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
        return;
    }
    games_[gameId] = std::move(game);
    session->games.insert(gameId);

    std::stringstream filter;
    filter << "{\"user\": \"" << session->user << "\"}";
    bsoncxx::builder::stream::document queryBson;
    queryBson << "$push" << bsoncxx::builder::stream::open_document << "games" << gameId
              << bsoncxx::builder::stream::close_document;
    db.Update("users", filter.str(), queryBson);
    std::stringstream res;
    res << "{\"game_id\": \"" << gameId << "\"}";
    response->write(res, corsHeader_);
}

void Server::deleteGame(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    auto gameIdO = Utils::GetT<std::string>(d, "game_id");
    if (!gameIdO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    auto& gameId = *gameIdO;
    auto& db = DB::DB::Instance();
    std::stringstream filter;
    filter << "{\"games\": {\"$in\":[\"" << gameId << "\"]}}";
    bsoncxx::builder::stream::document queryBson;
    queryBson << "$pull" << bsoncxx::builder::stream::open_document << "games" << gameId
              << bsoncxx::builder::stream::close_document;
    db.Update("users", filter.str(), queryBson);

    std::stringstream query;
    query << "{\"_id\": { \"$oid\" : \"" << gameId << "\"}}";
    db.Delete("games", query.str());
    games_.erase(gameId);

    response->write("", corsHeader_);
}

void Server::lastUpdate(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    auto gameIdO = Utils::GetT<std::string>(d, "game_id");
    if (!gameIdO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    auto& gameId = *gameIdO;
    auto game = findGame(gameId);
    if (!game) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Game not found", corsHeader_);
        return;
    }
    std::stringstream res;
    res << "{\"updated\":" << game->LastUpdated() << "}";
    response->write(res.str(), corsHeader_);
}

std::pair<SimpleWeb::StatusCode, std::string> Server::undo(HttpServer::Request* request)
{
    auto maybeD = tryParseRequest(request);
    if (!maybeD) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    auto d = (*maybeD).view();
    auto session = validateRequest(d);
    if (!session) {
        return {SimpleWeb::StatusCode::client_error_unauthorized, ""};
    }
    auto gameId = d["game_id"];
    if (!gameId || gameId.type() != bsoncxx::type::k_utf8) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    auto& db = DB::DB::Instance();
    bsoncxx::builder::stream::document filter;
    filter << "_id" << bsoncxx::oid(gameId.get_utf8().value);
    auto gameOpt = db.Get("games", filter);
    if (!gameOpt) {
        return {SimpleWeb::StatusCode::client_error_bad_request, "Game not found"};
    }
    const auto& game = (*gameOpt).view();
    const auto& currState = game["state"];
    if (!currState || currState.type() != bsoncxx::type::k_document) {
        return {SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error"};
    }
    const auto& currPlayer = currState.get_document().view()["turn"];
    if (!currPlayer || currPlayer.type() != bsoncxx::type::k_utf8) {
        return {SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error"};
    }
    if (std::string(currPlayer.get_utf8().value) != session->user) {
        return {SimpleWeb::StatusCode::client_error_bad_request, "Can only undo my turns"};
    }
    const auto& history = game["history"];
    if (!history || history.type() != bsoncxx::type::k_array) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    const auto& histArr = history.get_array().value;
    auto l = std::distance(histArr.begin(), histArr.end());
    if (l < 2) {
        return {SimpleWeb::StatusCode::client_error_bad_request, ""};
    }
    const auto& newState = histArr[static_cast<uint32_t>(l - 2)];
    if (newState.type() != bsoncxx::type::k_document) {
        return {SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error"};
    }
    const auto& turn = newState["turn"];
    if (!turn || turn.type() != bsoncxx::type::k_utf8) {
        return {SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error"};
    }
    if (std::string(turn.get_utf8().value) != session->user) {
        return {SimpleWeb::StatusCode::client_error_bad_request, "Can only undo my turns"};
    }
    bsoncxx::builder::stream::document query;
    query << "$set" << bsoncxx::builder::stream::open_document << "state" << newState.get_document()
          << bsoncxx::builder::stream::close_document << "$pop" << bsoncxx::builder::stream::open_document << "moves"
          << 1 << "history" << 1 << bsoncxx::builder::stream::close_document;

    db.Update("games", bsoncxx::to_json(filter), query);
    games_.erase(std::string(gameId.get_utf8().value));
    return {SimpleWeb::StatusCode::success_ok, ""};
}

std::optional<bsoncxx::document::value> Server::tryParseRequest(HttpServer::Request* request)
{
    return Utils::ParseBson(request->content.string());
}

void Server::makeTurn(HttpServer::Response* response, HttpServer::Request* request)
{
    auto maybeD = tryParseRequest(request);
    if (!maybeD) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, corsHeader_);
        return;
    }
    auto d = (*maybeD).view();
    auto session = validateRequest(response, d);
    if (!session) {
        return;
    }
    const auto& gameId = d["game_id"];
    if (!gameId || gameId.type() != bsoncxx::type::k_utf8) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    std::string gameIdStr(gameId.get_utf8().value);
    auto game = findGame(gameIdStr);
    if (!game) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Game not found", corsHeader_);
        return;
    }
    const auto& turn = d["turn"];
    if (!turn || turn.type() != bsoncxx::type::k_document) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing turn", corsHeader_);
        return;
    }
    auto m = std::make_shared<Engine::Move>(session->user);
    if (!m->FromJson(turn.get_document().view())) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Invalid turn", corsHeader_);
        return;
    }
    if (!game->ValidateMove(m.get())) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Illegal turn", corsHeader_);
        return;
    }
    bsoncxx::builder::stream::document bson;
    m->ToJson(bson);
    spdlog::info("{}'s turn: {}", session->user, bsoncxx::to_json(bson));

    game->PerformMove(m);
    dumpGame(game, true);
    response->write("", corsHeader_);
}

void Server::queryGame(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    auto gameIdO = Utils::GetT<std::string>(d, "game_id");
    if (!gameIdO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    auto& gameId = *gameIdO;
    auto game = findGame(gameId);
    if (!game) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Game not found", corsHeader_);
        return;
    }
    bsoncxx::builder::stream::document res;
    bsoncxx::builder::stream::document gameBson;
    game->ToJson(gameBson, session->user);
    res << "game" << gameBson;
    bsoncxx::builder::stream::array moves;
    for (const auto& move : game->GetMoves()) {
        bsoncxx::builder::stream::document moveBson;
        move->ToJson(moveBson);
        moves << moveBson;
    }
    res << "moves" << moves;
    response->write(bsoncxx::to_json(res), corsHeader_);
}

void Server::listGames(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    bool showAll = false;
    auto allGamesO = Utils::GetT<bool>(d, "showPreparing");
    if (allGamesO && *allGamesO) {
        showAll = true;
    }
    bsoncxx::builder::stream::document query;
    if (showAll) {
        query << "state.state"
              << "preparing";
    } else {
        if (session->games.empty()) {
            response->write("{\"games\":[]}", corsHeader_);
            return;
        }
        bsoncxx::builder::stream::array arr;
        for (const auto& id : session->games) {
            bsoncxx::builder::stream::document idDoc;

            arr << bsoncxx::oid(id);
        }
        query << "_id" << bsoncxx::builder::stream::open_document << "$in" << arr
              << bsoncxx::builder::stream::close_document;
    }
    auto& db = DB::DB::Instance();
    auto games = db.Find("games", query);
    bsoncxx::builder::stream::document res;
    bsoncxx::builder::stream::array resArray;

    for (auto game : games.view()) {
        const auto& stateElm = game["state"];
        if (!stateElm || stateElm.type() != bsoncxx::type::k_document) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        const auto& state = stateElm.get_document().view();
        bsoncxx::builder::stream::document gameBson;
        const auto& id = game["_id"];
        if (!id || id.type() != bsoncxx::type::k_oid) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        gameBson << "id" << id.get_oid().value.to_string();

        const auto& name = state["name"];
        if (!name || name.type() != bsoncxx::type::k_utf8) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        gameBson << "name" << name.get_utf8().value;
        const auto& gameState = state["state"];
        if (!gameState || gameState.type() != bsoncxx::type::k_utf8) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        gameBson << "state" << gameState.get_utf8().value;
        const auto& players = state["players"];
        if (!players || players.type() != bsoncxx::type::k_array) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        bsoncxx::builder::stream::array playersArr;
        for (const auto& player : players.get_array().value) {
            if (player.type() != bsoncxx::type::k_document) {
                response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
                return;
            }
            const auto& playerDoc = player.get_document().view();
            const auto& user = playerDoc["user"];
            if (!user || user.type() != bsoncxx::type::k_utf8) {
                response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
                return;
            }
            playersArr << user.get_utf8();
        }
        gameBson << "players" << playersArr;
        resArray << gameBson;
    }
    res << "games" << resArray;
    response->write(bsoncxx::to_json(res), corsHeader_);
}

void Server::joinGame(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    auto session = validateRequest(response, request, d);
    if (!session) {
        return;
    }
    auto gameIdO = Utils::GetT<std::string>(d, "game_id");
    if (!gameIdO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing game_id", corsHeader_);
        return;
    }
    auto gameId = *gameIdO;
    auto game = findGame(gameId);
    if (!game) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Game not found", corsHeader_);
        return;
    }
    bool res = game->AddPlayer(session->user);
    if (!res) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Can't join", corsHeader_);
        return;
    }
    auto& db = DB::DB::Instance();
    session->games.insert(gameId);
    std::stringstream filter;
    filter << "{\"user\": \"" << session->user << "\"}";
    std::stringstream query;
    query << "{\"$push\":{\"games\":\"" << gameId << "\"}}";
    db.UpdateLegacy("users", filter.str(), query.str());
    dumpGame(game);
    response->write("", corsHeader_);
}

void Server::dumpGame(Engine::Game* g, bool pushState /* = false*/)
{
    auto& db = DB::DB::Instance();
    std::stringstream filter;
    filter << "{\"state.name\":\"" << g->GetName() << "\"}";
    bsoncxx::builder::stream::document query;
    bsoncxx::builder::stream::document gameBson;
    g->ToJson(gameBson);
    bsoncxx::builder::stream::array movesBson;
    for (const auto& move : g->GetMoves()) {
        bsoncxx::builder::stream::document moveBson;
        move->ToJson(moveBson);
        movesBson << moveBson;
    }
    query << "$set" << bsoncxx::builder::stream::open_document << "state" << gameBson << "moves" << movesBson
          << bsoncxx::builder::stream::close_document;
    if (pushState) {
        query << "$push" << bsoncxx::builder::stream::open_document << "history" << gameBson
              << bsoncxx::builder::stream::close_document;
    }
    int updated = db.Update("games", filter.str(), query);
    if (updated != 1) {
        spdlog::error("Wrong number of entries updated: {}", updated);
    }
}

Session* Server::getSession(std::string_view id)
{
    retireSessions();
    auto res = sessions_.find(id);
    if (res == sessions_.end()) {
        return nullptr;
    }
    auto session = res->second.get();
    extendSession(session);
    return session;
}

void Server::extendSession(Session* session)
{
    session->expiration = std::chrono::system_clock::now() + std::chrono::minutes(10);
}

std::string Server::createSession(const bsoncxx::document::view& userInfo)
{
    retireSessions();
    auto res = Utils::MakeUUID();
    sessions_[res] = std::make_unique<Session>();
    auto session = sessions_.at(res).get();
    session->id = res;
    extendSession(session);

    const auto& user = userInfo["user"];
    if (user && user.type() == bsoncxx::type::k_utf8) {
        session->user = std::string(user.get_utf8().value);
    }
    const auto& games = userInfo["games"];
    if (games && games.type() == bsoncxx::type::k_array) {
        for (auto elm : games.get_array().value) {
            if (elm.type() == bsoncxx::type::k_utf8) {
                session->games.insert(std::string(elm.get_utf8().value));
            }
        }
    }
    spdlog::info("Created session {} for {}", res, session->user);
    return res;
}

void Server::retireSessions()
{
    for (auto itr = sessions_.begin(); itr != sessions_.end();) {
        if (itr->second->expiration < std::chrono::system_clock::now()) {
            spdlog::info("Retiring session {} for {}", itr->second->id, itr->second->user);
            itr = sessions_.erase(itr);
        } else {
            ++itr;
        }
    }
}

Engine::Game* Server::findGame(std::string& id)
{
    if (games_.count(id) > 0) {
        return games_.at(id).get();
    }
    auto& db = DB::DB::Instance();
    bsoncxx::builder::stream::document query;
    query << "_id" << bsoncxx::oid(id);
    auto gameBson = db.Get("games", query);
    if (!gameBson) {
        return nullptr;
    }
    auto game = std::make_unique<Engine::Game>("temp");
    if (!game->Init("../res/settings.json")) {
        return nullptr;
    }
    if (!game->FromJson((*gameBson).view())) {
        return nullptr;
    }
    auto res = game.get();
    games_[id] = std::move(game);
    return res;
}

} // namespace Server
