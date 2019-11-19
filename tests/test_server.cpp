#include "spdlog_wrap.h"
#include "bsoncxx_wrap.h"
#include "catch.hpp"
#include "Engine/Game.h"
#include "Server/Server.h"
#include "DB/DB.h"
/*
TEST_CASE("Server", "[server]")
{
    spdlog::set_level(spdlog::level::off);
    auto& db = DB::DB::Instance();
    db.SetDbName("test");
    db.Delete("users", "{}");
    db.Insert("users", "{\"user\":\"user1\"}");
    db.Insert("users", "{\"user\":\"user2\"}");
    db.Delete("games", "{}");
    Server::Server s;
    s.Start();
    using HttpClient = SimpleWeb::Client<SimpleWeb::HTTP>;
    HttpClient client("localhost:8080");
    SECTION("Login")
    {
        auto r = client.request("POST", "/login", "{\"user\":\"baduser\"}");
        REQUIRE(r->status_code != "200 OK");
        r = client.request("POST", "/login", "{\"user\":\"user1\"}");
        REQUIRE(r->status_code == "200 OK");
    }
    SECTION("List games")
    {
        auto r = client.request("POST", "/login", "{\"user\":\"user1\"}");
        REQUIRE(r->status_code == "200 OK");
        std::stringstream ss;
        ss << r->content.rdbuf();
        auto res = bsoncxx::from_json(ss.str());
        const auto& res1 = res.view();
        std::string sessionId(res1["session_id"].get_utf8().value);
        std::stringstream body;
        body << "{\"session\":\"" << sessionId << "\"}";
        r = client.request("POST", "/game/list", body.str());
        REQUIRE(r->status_code == "200 OK");
    }
    s.Stop();
}
*/