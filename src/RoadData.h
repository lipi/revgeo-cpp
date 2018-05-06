
#ifndef ROAD_SEGMENT_CPP_ROADDATA_H
#define ROAD_SEGMENT_CPP_ROADDATA_H

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "SQLiteCpp/SQLiteCpp.h"

class RoadData {

public:
    explicit RoadData(std::string dbFileName);
    ~RoadData() = default;

private:

    void LoadTiles();

    std::unique_ptr<SQLite::Database> m_pDb;
    std::shared_ptr<spdlog::logger> m_pLog;
    int m_TileCnt;
};

#endif //ROAD_SEGMENT_CPP_ROADDATA_H
