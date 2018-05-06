
#include "platform.h"
#include "RoadData.h"

RoadData::RoadData(std::string dbFileName) :
        m_TileCnt(0)
{

    m_pLog = spdlog::get("console");

    m_pLog->info("Opening DB...");
    try {
        std::make_unique<SQLite::Database>(dbFileName);
    }
    catch (const SQLite::Exception& ex) {
        fprintf(stderr, "%s : %s\n", dbFileName.c_str(), ex.what());
        return;
    }

    LoadTiles();
}

void RoadData::LoadTiles() {

    m_pLog->info("Loading tiles...");
    SQLite::Statement query(*m_pDb, "SELECT clat,clon,road FROM tile LIMIT 1000");


    while (query.executeStep())
    {
        int clat = query.getColumn(0);
        int clon = query.getColumn(1);
        std::string tileData = query.getColumn(2);
        m_TileCnt++;
    }
    m_pLog->info("Loaded {} tiles", m_TileCnt);

}