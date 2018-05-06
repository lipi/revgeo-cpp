#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "RoadData.h"

static const char* DB_FILE = "data/tiles.sqlite";

int main() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::info);

    RoadData roadData(DB_FILE);

    return 0;
}