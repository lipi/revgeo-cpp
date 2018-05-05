#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "SQLiteCpp/SQLiteCpp.h"

int main() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::info);

    spdlog::get("console")->info("Opening DB...");
    SQLite::Database db("data/tiles.sqlite");
    std::cout << "Hello, World!" << std::endl;
    return 0;
}