#include "Server.h"
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;
namespace Server
{
void Server::Start()
{
    server = std::make_unique<HttpServer>();
    server->config.port = 8080;
    server->resource["^/info$"]["GET"] = [](std::shared_ptr<HttpServer::Response> response,
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
} // namespace Server
