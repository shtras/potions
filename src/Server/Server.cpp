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
    {
        Engine::Game g("test");
        std::string p1{"player1"};
        std::string p2{"player2"};
        g.Init("../res/settings.json");
        g.AddPlayer(p1);
        g.AddPlayer(p2);
        g.Start();
        bsoncxx::builder::stream::document d;
        g.ToJson(d);
        spdlog::info(bsoncxx::to_json(d));
    }

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
        login(response.get(), request.get());
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

void Server::login(HttpServer::Response* response, HttpServer::Request* request)
{
    rapidjson::Document d;
    d.Parse(request->content.string());
    if (d.HasParseError()) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, corsHeader_);
        return;
    }
    auto userO = Utils::GetT<std::string>(d, "user");
    if (!userO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, corsHeader_);
        return;
    }
    std::stringstream query;
    query << "{\"user\": \"" << *userO << "\"}";
    auto& db = DB::DB::Instance();
    auto userInfo = db.Get("users", query.str());
    if (!userInfo) {
        response->write(SimpleWeb::StatusCode::client_error_unauthorized, corsHeader_);
        return;
    }
    auto id = createSession((*userInfo).view());
    std::stringstream res;
    res << "{\"session_id\": \"" << id << "\"}";
    response->write(res, corsHeader_);
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
    bsoncxx::builder::stream::document bson;
    game->ToJson(bson);
    auto gameId = db.Insert("games", bsoncxx::to_json(bson));
    if (gameId == "") {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
        return;
    }
    games_[gameId] = std::move(game);
    session->games.insert(gameId);
    std::stringstream query;
    query << "{\"game_id\": \"" << gameId << "\",\"turns\":[]}";
    db.Insert("history", query.str());

    std::stringstream filter;
    filter << "{\"user\": \"" << session->user << "\"}";
    query.str("");
    query.clear();
    query << "{\"$push\":{\"games\":\"" << gameId << "\"}}";
    db.Update("users", filter.str(), query.str());
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
    std::stringstream query;
    query << "{\"$pull\":{\"games\":\"" << gameId << "\"}}";
    db.Update("users", filter.str(), query.str());

    query.str("");
    query.clear();
    query << "{\"_id\": { \"$oid\" : \"" << gameId << "\"}}";
    db.Delete("games", query.str());
    games_.erase(gameId);

    query.str("");
    query.clear();
    query << "{\"game_id\": \"" << gameId << "\"}";
    db.Delete("history", query.str());

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

void Server::makeTurn(HttpServer::Response* response, HttpServer::Request* request)
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
    auto turnO = Utils::GetT<rapidjson::Value::ConstObject>(d, "turn");
    if (!turnO) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Missing turn", corsHeader_);
        return;
    }
    Engine::Move m(session->user);
    if (!m.FromJson(*turnO)) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Invalid turn", corsHeader_);
        return;
    }
    if (!game->ValidateMove(m)) {
        response->write(SimpleWeb::StatusCode::client_error_bad_request, "Illegal turn", corsHeader_);
        return;
    }
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> w(s);
    m.ToJson(w);
    spdlog::info("{}'s turn: {}", session->user, s.GetString());
    auto& db = DB::DB::Instance();
    std::stringstream filter;
    filter << "{\"game_id\":\"" << gameId << "\"}";
    std::stringstream query;
    query << "{\"$push\":{\"turns\":" << s.GetString() << "}}";
    db.Update("history", filter.str(), query.str());
    game->PerformMove(m);
    dumpGame(game);
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
    bsoncxx::builder::stream::document bson;
    game->ToJson(bson, session->user);

    std::stringstream res;
    res << "{\"game\":" << bsoncxx::to_json(bson) << ",\"turns\":";
    auto& db = DB::DB::Instance();
    std::stringstream query;
    query << "{\"game_id\":\"" << gameId << "\"}";
    auto history = db.LegacyGet("history", query.str());
    res << history << "}";
    response->write(res.str(), corsHeader_);
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
    std::stringstream query;
    if (showAll) {
        query << "{\"state\": \"preparing\"}";
    } else {
        if (session->games.empty()) {
            response->write("[]", corsHeader_);
            return;
        }
        query << "{\"_id\":{\"$in\":[";
        auto itr = session->games.begin();
        query << "{\"$oid\":\"" << *itr << "\"}";
        ++itr;
        for (; itr != session->games.end(); ++itr) {
            query << ",{\"$oid\":\"" << *itr << "\"}";
        }
        query << "]}}";
    }
    auto& db = DB::DB::Instance();
    rapidjson::Document gamesDoc;
    gamesDoc.Parse(db.Find("games", query.str()));
    if (gamesDoc.HasParseError() || !gamesDoc.IsArray()) {
        response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
        return;
    }
    const auto& games = gamesDoc.GetArray();
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> w(s);
    w.StartArray();
    for (rapidjson::SizeType i = 0; i < games.Size(); ++i) {
        w.StartObject();
        const auto& game = games[i];
        auto idO = Utils::GetT<rapidjson::Value::ConstObject>(game, "_id");
        if (!idO) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        const auto& id = *idO;
        if (!id.HasMember("$oid")) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        auto nameO = Utils::GetT<std::string>(game, "name");
        if (!nameO) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        auto stateO = Utils::GetT<std::string>(game, "state");
        if (!stateO) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        auto playersO = Utils::GetT<rapidjson::Value::ConstArray>(game, "players");
        if (!playersO) {
            response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
            return;
        }
        w.Key("id");
        w.String(id["$oid"].GetString());
        w.Key("name");
        w.String(*nameO);
        w.Key("state");
        w.String(*stateO);
        w.Key("players");
        w.StartArray();
        const auto& players = *playersO;
        for (rapidjson::SizeType j = 0; j < players.Size(); ++j) {
            auto playerO = Utils::GetT<rapidjson::Value::ConstObject>(players[j]);
            if (!playerO) {
                response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
                return;
            }
            const auto& player = *playerO;
            if (!player.HasMember("user") || !player["user"].IsString()) {
                response->write(SimpleWeb::StatusCode::server_error_internal_server_error, "DB Error", corsHeader_);
                return;
            }
            w.String(player["user"].GetString());
        }
        w.EndArray();
        w.EndObject();
    }
    w.EndArray();
    response->write(s.GetString(), corsHeader_);
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
    response->write("", corsHeader_);
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
    db.Update("users", filter.str(), query.str());
    dumpGame(game);
    response->write("", corsHeader_);
}

void Server::dumpGame(Engine::Game* g)
{
    auto& db = DB::DB::Instance();
    std::stringstream filter;
    filter << "{\"name\":\"" << g->GetName() << "\"}";
    bsoncxx::builder::stream::document d;
    g->ToJson(d);
    db.Replace("games", filter.str(), d);
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

    auto user = userInfo["user"];
    if (user && user.type() == bsoncxx::type::k_utf8) {
        session->user = std::string(user.get_utf8().value);
    }
    auto games = userInfo["games"];
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
    std::stringstream query;
    query << "{\"_id\": { \"$oid\" : \"" << id << "\"}}";
    auto gameBson = db.Get("games", query.str());
    if (!gameBson) {
        return nullptr;
    }
    auto game = std::make_unique<Engine::Game>("temp");
    bool ret = game->Init("../res/settings.json");
    if (!ret) {
        return nullptr;
    }
    game->FromJson((*gameBson).view());
    auto res = game.get();
    games_[id] = std::move(game);
    return res;
}

} // namespace Server
