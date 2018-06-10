# !/usr/bin/python3

import sys
import os.path

import psycopg2
from tqdm import tqdm

osm_local = 'dbname=osm user=lipi host=localhost port=5432'


def query(cmd, server):

    conn = psycopg2.connect(server)
    cur = conn.cursor()
    cur.execute(cmd)
    result = cur.fetchall()
    return result


def get_json(rect, environment=osm_local):
    """
    Get map tile for [top_left, bottom_right] rectangle.

    Return result as a GeoJSON string
    """
    cmd = """
    SELECT osm_id, st_asgeojson(way)
    FROM planet_osm_roads
    WHERE st_intersects(st_setsrid(st_geomfromtext('POLYGON ((
    {lon1} {lat1},
    {lon2} {lat1},
    {lon2} {lat2},
    {lon1} {lat2},
    {lon1} {lat1}))'), 4326), way)
    """.format(lon1=rect['topleft']['lon'],
               lat1=rect['topleft']['lat'],
               lon2=rect['bottomright']['lon'],
               lat2=rect['bottomright']['lat'])
    
    result = query(cmd, environment)
    return result


def create_geojson(tile_info):
    """
    Add header/footer to JSON tile info

    Return proper GeoJSON with feature collection
    """
    
    tile_json = """
    {
    "type": "FeatureCollection",
    "features": [
    """

    for osm_id, way in tile_info:
        tile_json += """
        {
        "type": "Feature",
        "properties": {"""

        tile_json += """
            "osm_id" : {id}
            }},
        "geometry" : {road}
        }},
        """.format(id=osm_id, road=way)

    tile_json = tile_json[:-3]  # remove trailing newline and comma
    tile_json += """
    ]
    }
    """
    return tile_json


def centi(x):
    """
    Return centidegrees (100th of a degree)

    >>> centi(174.7145)
    17471
    >>> centi(-36.73)
    -3673
    """
    x = float(x)
    if x > 180.0:
        x -= 360.0
    return int((x * 1000)/10)  # necessary for correct floating point arithmetic


def rect_to_tiles(top_left, bottom_right):
    """Split rectangle to 0.01x0.01 degree tiles

    Tile vertices will be on a grid of .01 degrees lon by .01 degrees lat.

    Return a list of tile coordinates as dictionaries.
    """
    a, b = top_left
    c, d = bottom_right

    tiles = []
    for lat in range(centi(a), centi(c), -1):
        for lon in range(centi(b), centi(d), 1):
            tile = {'topleft': {}, 'bottomright':{}}
            tile['topleft']['lat'] = round(lat/100.0 ,2)
            tile['topleft']['lon'] = round(lon/100.0, 2)
            tile['bottomright']['lat'] = round(lat/100.0 - .01, 2)
            tile['bottomright']['lon'] = round(lon/100.0 + .01, 2)
            tiles.append(tile)
    
    return tiles


def test():
    import doctest
    doctest.testmod()

# entry point when the script is run standalone
if __name__ == "__main__":
    if len(sys.argv) != 5:
        print("Wrong argument count!")
        print("Format: top_left_lat top_left_lon bottom_right_lat bottom_right_lon")
        sys.exit(1)

    tl_lat, tl_lon, br_lat, br_lon = [float(x) for x in sys.argv[1:]]

    tiles = rect_to_tiles((tl_lat, tl_lon), (br_lat, br_lon))
        
    for tile in tqdm(tiles):
        # depot endpoint uses latlon scheme, no file extension
        filename = '{:d}_{:d}'.format(centi(tile['topleft']['lat']),
                                      centi(tile['topleft']['lon']))

        if os.path.exists(filename):
            print(filename + " already exists")
        else:
            tile_info = get_json(tile, osm_local)
            if len(tile_info) > 0:
                tile = create_geojson(tile_info)
                with open(filename, 'w') as outfile:
                    outfile.write(tile)
            else:
                # empty tile
                pass
