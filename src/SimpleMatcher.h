//
// Created by Andras Lipoth on 13/05/18.
//

#ifndef ROAD_SEGMENT_CPP_SIMPLEMATCHER_H
#define ROAD_SEGMENT_CPP_SIMPLEMATCHER_H

#include "RoadData.h"

class SimpleMatcher {

public:
    SimpleMatcher(RoadData& roadData);
    ~SimpleMatcher() = default;

    std::pair<RoadData::rsid_t, double> Lookup(float lat, float lon);

private:
    RoadData& m_RoadData;
};
#endif //ROAD_SEGMENT_CPP_SIMPLEMATCHER_H
