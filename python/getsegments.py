#!/usr/bin/python3

from collections import defaultdict

import records
from shapely import wkb
import numpy as np
import geojson

"""
Go through all lines in of all road segments in table and
add their IDs to tiles being crossed by the line.

There is no overlap between tiles.xf
"""

RESOLUTION = 0.01
GRID_PER_DEGREE = 100


def get_road_segments():
    db = records.Database('postgres://lipi@localhost:5432/osm')
    result = db.query('select osm_id, way from planet_osm_roads where osm_id = 138875524 limit 1')
    return result


def rasterise(x, gpd=GRID_PER_DEGREE):
    return int(float(x) * gpd) / gpd


def crossings(a, b, res=RESOLUTION):
    start, end = (a, b) if a < b else (b, a)

    result = []
    x = start + res if start > 0 else start
    while rasterise(x) <= end:
        result.append(rasterise(x))
        x += res
    return result


class Point:

    def __init__(self, *args):
        if len(args) == 1:
            self.lon, self.lat = args[0][0], args[0][1]
        if len(args) == 2:
            self.lon = args[0]
            self.lat = args[1]

        self.x, self.y = self.lon, self.lat



class Line:

    def __init__(self, a, b):
        self.a = Point(a)
        self.b = Point(b)

    def __repr__(self):
        return '{},{} --> {},{}'.format(self.a.x, self.a.y, self.b.x, self.b.y)

    def gradient(self):
        dx = self.b.x - self.a.x
        dy = self.b.y - self.a.y
        return dy/dx

    def tiles(self):
        """
        Return list of tiles which are intersected by the line
        """
        m = self.gradient()
        result = [(rasterise(self.a.x), rasterise(self.a.y))]

        for x in crossings(self.a.x, self.b.x):
            y = self.a.y + (x - self.a.x) * m
            result.append((rasterise(x), rasterise(y)))

        for y in crossings(self.a.y, self.b.y):
            x = self.a.x + (y - self.a.y) / m
            result.append((rasterise(x), rasterise(y)))

        return result


def tiles_to_geojson(tiles, res=RESOLUTION):

    result = []
    for lon, lat in tiles:
        dlon = -res if lon < 0 else res
        dlat = -res if lat < 0 else res

        polygon = geojson.Polygon([[
            (lon, lat),
            (lon + dlon, lat),
            (lon + dlon, lat + dlat),
            (lon, lat + dlat),
            (lon, lat)]])
        result.append(geojson.Feature(geometry=polygon, properties={'lon': lon, 'lat': lat}))
    return result


if __name__ == '__main__':

    tile_map = defaultdict(lambda: defaultdict(list))
    line_features = []

    for road in get_road_segments():
        linestring = wkb.loads(road.way, hex=True)
        coords = linestring.coords
        for i in range(len(coords)-1):
            line = Line(coords[i], coords[i + 1])
            crossed_tiles = line.tiles()
            print('line: {} | crossed tiles: {}'.format(line, list(crossed_tiles)))
            for tile in crossed_tiles:
                tile_map[tile][road.osm_id].append(line)

        for tile in tile_map.keys():
            print('tile: {}'.format(tile))
            for osm_id in tile_map[tile]:
                print('\tosm_id: {}'.format(osm_id))
                for line in tile_map[tile][osm_id]:
                    print('\t\tline: {}'.format(line))

        line_features.append(geojson.Feature(geometry=linestring, properties={'osm_id': road.osm_id}))

    tile_features = tiles_to_geojson(tile_map.keys())

    featureColl = geojson.FeatureCollection(features=tile_features + line_features)
    with open('x.geojson', 'w') as f:
        f.write(geojson.dumps(featureColl, indent=4, sort_keys=True))
