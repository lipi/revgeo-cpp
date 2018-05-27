#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
//#include "tqdm/tqdm.h"

#include "RoadData.h"
#include "SimpleMatcher.h"

//#define DEBUG 1

int main(int argc, char* argv[]) {

    if ( argc < 3 ) {
        fprintf(stderr, "Usage: %s <dbfile|binaryfile> <latlon.csv>\n", argv[0]);
        return 1;
    }

    auto console = spdlog::stdout_color_mt("console");

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
    RoadData roadData(argv[1], 100);

    for (RoadData::RoadSegment* pRoad : roadData.GetRoadSegments(48.06f, -124.00f)) {
        for (int i = 0; i < pRoad->size; i++) {
            printf("[%f,%f] ",pRoad->points[i].lat, pRoad->points[i].lon);
        }
        printf("\n");
    }
#else
    spdlog::set_level(spdlog::level::info);
    RoadData roadData(argv[1]);
#endif

    spdlog::get("console")->info("Reading CSV file {} ...", argv[2]);
    std::ifstream data(argv[2]);
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

    return 0;
}
