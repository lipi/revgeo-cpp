
#include "platform.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "RoadData.h"

using namespace rapidjson;

RoadData::RoadData(std::string dbFileName) :
        m_TileCnt(0)
{

    m_pLog = spdlog::get("console");

    m_pLog->info("Opening DB...");
    try {
        m_pDb = std::make_unique<SQLite::Database>(dbFileName);
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

        Document d; // TODO: use SOX parsing to speed it up
        d.Parse(tileData.c_str());
        for (const Value& f : d["features"].GetArray()) {
            int roadSegmentId = f["properties"]["road_segment_id"].GetInt();
            printf("id: %d\n", roadSegmentId);
            for (const Value& c : f["geometry"]["coordinates"].GetArray() ) {
                float lat = c[0].GetFloat();
                float lon = c[1].GetFloat();
                printf("%f,%f ", lat, lon);
            }
        }
    }
    m_pLog->info("Loaded {} tiles", m_TileCnt);
}