
#include <math.h>
#include "SimpleMatcher.h"


static double distanceSquared(const RoadData::Point& a, const RoadData::Point& b) {
    // from http://www.movable-type.co.uk/scripts/latlong.html
    // the polar coordinate flat-earth formula can be used:
    // using the co-latitudes θ1 = π/2−φ1 and θ2 = π/2−φ2,
    // then d = R ⋅ √(θ1² + θ2² − 2 ⋅ θ1 ⋅ θ2 ⋅ cos Δλ)
    // where φ is latitude, λ is longitude, R is earth’s radius (mean radius = 6,371km)
    // note that angles need to be in radians to pass to trig functions!
    double phi1 = M_PI * a.lat / 180.0;
    double phi2 = M_PI * b.lat / 180.0;
    double dlambda = M_PI * (b.lon - a.lon) / 180.0;
    double t1 = M_PI_2 - phi1;
    double t2 = M_PI_2 - phi2;
    double R = 6371000.0;

    double distSquared =  R * R * (t1*t1 + t2*t2 - 2*t1*t2*cos(dlambda));
    // saving a sqrt, will be done on the final (shortest) distance
    return distSquared;
}

static RoadData::Point closestPoint(const RoadData::Point& a,
                                    const RoadData::Point& b,
                                    const RoadData::Point& p) {
    if ( a.lon == b.lon && a.lat == b.lat ) {
        return a;
    }

    double dLat = b.lat - a.lat;
    double dLon = b.lon - a.lon;
    double t = ((p.lat - a.lat) * dLat + (p.lon - a.lon) * dLon) / (dLat * dLat + dLon * dLon);

    if ( t < 0 ) {
        return a;
    }
    else if ( t > 1 ) {
        return b;
    }
    else {
        return {static_cast<float>(a.lon + t * dLon), static_cast<float>(a.lat + t * dLat)};
    }
}

static double distanceToLineSquared(const RoadData::Point& a,
                                    const RoadData::Point& b,
                                    const RoadData::Point& p) {
    RoadData::Point projection = closestPoint(a, b, p);
    double ds = distanceSquared(p, projection);
    return ds;
}

SimpleMatcher::SimpleMatcher(RoadData& roadData) :
        m_RoadData(roadData) {
}

std::pair<RoadData::rsid_t, double> SimpleMatcher::Lookup(float lat, float lon) {
    const std::vector<RoadData::RoadSegment*> roads = m_RoadData.GetRoadSegments(lat + 0.01, lon - 0.01);
    double dMin = 0; // note: will hold squared distance
    RoadData::rsid_t rsid;
    RoadData::Point point {lon, lat};
    for (auto* pRoad : roads) {
        for (int i = 0; i < pRoad->size - 1; i++) {
            double d = distanceToLineSquared(pRoad->points[i], pRoad->points[i+1], point);
            if (0 == dMin || d < dMin) {
                dMin = d;
                rsid = pRoad->id;
            }
        }
    }
    return std::make_pair(rsid, sqrt(dMin));
}
