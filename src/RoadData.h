
#ifndef ROAD_SEGMENT_CPP_ROADDATA_H
#define ROAD_SEGMENT_CPP_ROADDATA_H

#include <cstdint>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "rapidjson/document.h"

using namespace rapidjson;

class RoadData {

public:
    typedef uint32_t rsid_t;
    typedef uint32_t length_t;

    // TODO: constructor, to use latlon for consistency
    // (user shouldn't need to know about implementation detail)
    struct Point {
        float lon; // geoJSON uses lon-lat order, array layout follows that
        float lat;
    };

    struct RoadSegment {
        rsid_t id;
        length_t size;
        Point points[1];
    };

    explicit RoadData(std::string dbFileName, size_t limit = 0);
    ~RoadData();
    std::vector<RoadSegment*> GetRoadSegments(float lat, float lon);

private:

    typedef uint32_t key_t;
    typedef int32_t cdegree_t; // centi-degree, i.e. hundreth-degreee
    typedef uint32_t offset_t;

    struct Tile {
        key_t clatclon;
        length_t size;
        offset_t offsets[1];
    };

    void LoadTiles(size_t limit = 0);
    offset_t AddTile(key_t clatclon, offset_t* pOffsets, size_t num);
    offset_t AddRoadSegment(rsid_t rsid, const GenericArray<true, Value>& points );
    key_t GetKey(cdegree_t clat, cdegree_t clon);
    Tile* GetTile(float lat, float lon);
    cdegree_t CDegree(float degree);

    std::unique_ptr<SQLite::Database> m_pDb;
    std::shared_ptr<spdlog::logger> m_pLog;
    std::unordered_map<key_t, offset_t> m_Grid;
    uint32_t* m_pTiles;
    offset_t m_TileSize;
    offset_t m_TileOffset;
    uint32_t* m_pRoadSegments;
    offset_t m_RoadSegmentSize;
    offset_t m_RoadSegmentOffset;
};

#endif //ROAD_SEGMENT_CPP_ROADDATA_H
