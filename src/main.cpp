#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
//#include "tqdm/tqdm.h"

#include "RoadData.h"
#include "SimpleMatcher.h"

static const char* DB_FILE = "tiles.sqlite";

int main(int argc, char* argv[]) {

    if ( argc < 2 ) {
        fprintf(stderr, "Usage: %s <latlon.csv>\n", argv[0]);
        return 1;
    }

    auto console = spdlog::stdout_color_mt("console");

#if 0
    spdlog::set_level(spdlog::level::debug);
    RoadData roadData(DB_FILE, 100);
#else
    spdlog::set_level(spdlog::level::info);
    RoadData roadData(DB_FILE);
#endif

    for (RoadData::RoadSegment* pRoad : roadData.GetRoadSegments(48.06f, -124.00f)) {
        for (int i = 0; i < pRoad->size; i++) {
            printf("[%f,%f] ",pRoad->points[i].lat, pRoad->points[i].lon);
        }
        printf("\n");
    }

    spdlog::get("console")->info("Reading CSV file {} ...", argv[1]);
    std::ifstream data(argv[1]);
    std::string line;
    std::vector<RoadData::Point> testPoints;
    float lat, lon;
    while(std::getline(data, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> parsedRow;
        while (std::getline(lineStream,cell,',')) {
            parsedRow.push_back(cell);
        }

        lat = stof(parsedRow[0], nullptr);
        lon = stof(parsedRow[1], nullptr);
        RoadData::Point point{lon, lat};
        testPoints.push_back(point);
    }

    spdlog::get("console")->info("Processing {} points...", testPoints.size());
    int hits = 0;
    SimpleMatcher matcher(roadData);
    for (const auto& point : testPoints) {
        std::pair<RoadData::rsid_t, double> result = matcher.Lookup(point.lat, point.lon);
        if (result.first != 0) {
            hits++;
        }
        console->debug("road segment ID: {} distance: {}",  result.first, result.second);
    }
    console->info("Done, {} roads matched", hits);

    while (true) {}

    return 0;
}
