#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include "RoadData.h"
#include "SimpleMatcher.h"

static const char* DB_FILE = "tiles.sqlite";

int main() {
    auto console = spdlog::stdout_color_mt("console");
    spdlog::set_level(spdlog::level::debug);

    RoadData roadData(DB_FILE, 100);

    for (RoadData::RoadSegment* pRoad : roadData.GetRoadSegments(48.06, -124.00)) {
        for (int i = 0; i < pRoad->size; i++) {
            printf("[%f,%f] ",pRoad->points[i].lat, pRoad->points[i].lon);
        }
        printf("\n");
    }


    SimpleMatcher matcher(roadData);
    std::pair<RoadData::rsid_t, double> result = matcher.Lookup(48.055F, -123.995F);
    printf("Closest road: %d distance:%.2f meters\n", result.first, result.second);

    // busy loop to allow seeing memory use of process
    while (true) {}

    return 0;
}