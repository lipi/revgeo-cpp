#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "RoadData.h"

static const char* DB_FILE = "tiles.sqlite";

int main() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::debug);

    RoadData roadData(DB_FILE, 100);

    for (RoadData::roadsegment_t* pRoad : roadData.GetRoadsegments(48.06, -124.00)) {
        for (int i = 0; i < pRoad->size; i++) {
            printf("[%f,%f] ",pRoad->points[i].lat, pRoad->points[i].lon);
        }
        printf("\n");
    }
    while (true) {}

    return 0;
}