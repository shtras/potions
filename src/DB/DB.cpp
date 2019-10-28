#include "DB.h"

#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/types.hpp>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

namespace DB
{
void A::test()
{
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::uri uri("mongodb://localhost:27017");
    mongocxx::client client(uri);
    mongocxx::database db = client["mydb"];
    mongocxx::collection coll = db["test"];

    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value =
        builder << "name"
                << "MongoDB"
                << "type"
                << "database"
                << "count" << 1 << "versions" << bsoncxx::builder::stream::open_array << "v3.2"
                << "v3.0"
                << "v2.6" << bsoncxx::builder::stream::close_array << "info" << bsoncxx::builder::stream::open_document
                << "x" << 203 << "y" << 102 << bsoncxx::builder::stream::close_document
                << bsoncxx::builder::stream::finalize;

    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        coll.insert_one(bsoncxx::builder::basic::make_document(bsoncxx::builder::basic::kvp("test", 1)));
}
}