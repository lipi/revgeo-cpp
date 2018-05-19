
#include "platform.h"

#include "RoadData.h"

static const constexpr float INC_FACTOR = 1.25;

RoadData::RoadData(std::string dbFileName, size_t limit) :
        m_TileSize(1000),
        m_RoadSegmentSize(1000),
        m_AllowDuplicateRsids(false)
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

    m_pTiles = (uint32_t*)malloc(m_TileSize * sizeof(uint32_t));
    m_pRoadSegments = (uint32_t *)malloc(m_RoadSegmentSize * sizeof(uint32_t));

    LoadTiles(limit);
}

RoadData::~RoadData() {
    if (m_pTiles) {
        free(m_pTiles);
    }
    if (m_pRoadSegments) {
        free(m_pRoadSegments);
    }
}

void RoadData::LoadTiles(size_t limit) {

    m_pLog->info("Loading tiles...");

    SQLite::Statement query(*m_pDb, "SELECT clat,clon,road FROM tile");

    size_t remaining = limit;
    while (query.executeStep() && (limit == 0 || 0 < remaining-- ))
    {
        std::string tileData = query.getColumn(2);

        Document d; // TODO: use SOX parsing to speed it up
        d.Parse(tileData.c_str());
        std::vector<offset_t> offsets;
        for (const Value& f : d["features"].GetArray()) {
            int roadSegmentId = f["properties"]["road_segment_id"].GetInt();
            if (!m_AllowDuplicateRsids && 1 == m_Rsids.count(roadSegmentId)) {
                // refer to existing road segment
                uint32_t geometryOffset = m_Rsids[roadSegmentId];
                offsets.push_back(geometryOffset);
                m_pLog->debug("Using existing road segment ID {} at {}", roadSegmentId, geometryOffset);
            }
            else {
                // add new road segment
                const auto &coordinates = f["geometry"]["coordinates"].GetArray();

                uint32_t geometryOffset = AddRoadSegment(roadSegmentId, coordinates);
                offsets.push_back(geometryOffset);
                m_Rsids[roadSegmentId] = geometryOffset;
            }
        }

        cdegree_t clat = query.getColumn(0);
        cdegree_t clon = query.getColumn(1);
        key_t key = GetKey(clat, clon);

        offset_t tileOffset = AddTile(key, offsets.data(), offsets.size());
        m_Grid.insert(std::make_pair(key, tileOffset));
    }
    m_pLog->info("Loaded {} tiles", m_Grid.size());
}

std::vector<RoadData::RoadSegment*> RoadData::GetRoadSegments(float lat, float lon) {
    std::vector<RoadSegment*> roads;

    Tile* pTile = GetTile(lat, lon);
    if (nullptr == pTile) {
        return roads;
    }

    for (int i = 0; i < pTile->size; i++) {
        offset_t roadOffset = pTile->offsets[i];
        RoadSegment* pRoad = reinterpret_cast<RoadSegment*>(m_pRoadSegments + roadOffset);
        m_pLog->debug("Got road {} with {} points", pRoad->id, pRoad->size);
        roads.push_back(pRoad);
    }
    return roads;
}

RoadData::cdegree_t RoadData::CDegree(float degree) {
    return static_cast<RoadData::cdegree_t>(degree * 100);
}

RoadData::Tile* RoadData::GetTile(float lat, float lon) {
    cdegree_t clat = CDegree(lat);
    cdegree_t clon = CDegree(lon);
    key_t clatclon = GetKey(clat, clon);

    offset_t tileOffset = m_Grid[clatclon];
    Tile* pTile = reinterpret_cast<Tile*>(m_pTiles + tileOffset);
    assert(pTile);
    m_pLog->debug("Got tile [{}] ({}|{}) at offset {} with {} roads", clatclon, clat, clon, tileOffset, pTile->size);
    if (pTile->clatclon == clatclon) {
        return pTile;
    }
    else {
        return nullptr;
    }
}

RoadData::key_t RoadData::GetKey(cdegree_t clat, cdegree_t clon) {
    uint32_t lat = ((uint32_t)clat) << 16;
    uint32_t lon = ((uint32_t)clon & 0x0000ffff);
    key_t key = lat | lon;
    return key;
}

RoadData::offset_t RoadData::AddRoadSegment(rsid_t rsid, const GenericArray<true, Value>& points ) {
    // TODO: check if roadsegment exists (by looking it up in a map)

    m_pLog->debug("adding road segment {}", rsid);

    size_t sizeIncrement = 2 + points.Size() * 2;
    while (m_RoadSegmentSize <  m_RoadSegmentOffset + sizeIncrement) {
        m_pLog->info("road segments size {}, reallocating to {}",
                      m_RoadSegmentSize, (size_t)(m_RoadSegmentSize * INC_FACTOR));
        m_RoadSegmentSize = m_RoadSegmentSize * INC_FACTOR;
        m_pRoadSegments = (uint32_t*)realloc(m_pRoadSegments, m_RoadSegmentSize * sizeof(uint32_t));
        assert(nullptr != m_pRoadSegments);
    }

    offset_t offset = m_RoadSegmentOffset;

    m_pRoadSegments[m_RoadSegmentOffset++] = rsid;
    m_pRoadSegments[m_RoadSegmentOffset++] = points.Size();

    for (const Value& latlon : points) {
        float lat = latlon[0].GetFloat();
        float lon = latlon[1].GetFloat();
        reinterpret_cast<float*>(m_pRoadSegments)[m_RoadSegmentOffset++] = lat;
        reinterpret_cast<float*>(m_pRoadSegments)[m_RoadSegmentOffset++] = lon;
    }

    return offset;
}

RoadData::offset_t RoadData::AddTile(key_t clatclon, offset_t* pOffsets, size_t num) {

    m_pLog->debug("adding tile {}", clatclon);
    size_t sizeIncrement = 2 + num;
    while (m_TileSize < m_TileOffset + sizeIncrement) {
        m_pLog->info("tiles size {}, reallocating to {}",
                      m_TileSize, (size_t)(m_TileSize * INC_FACTOR));
        m_TileSize = m_TileSize * INC_FACTOR;
        m_pTiles = (uint32_t*)realloc(m_pTiles, m_TileSize * sizeof(uint32_t));
        assert(nullptr != m_pTiles);
    }

    offset_t offset = m_TileOffset;

    m_pTiles[m_TileOffset++] = clatclon;
    m_pTiles[m_TileOffset++] = num;

    for (int i = 0; i < num; i++) {
        m_pTiles[m_TileOffset++] = pOffsets[i];
    }

    return offset;
}