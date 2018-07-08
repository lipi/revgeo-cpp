
from getsegments import *


def test_rasterise():
    assert rasterise(1.2345) == 1.23
    assert rasterise(-1.2345) == -1.23


def test_crossings():
    assert crossings(-1.019, -1.021) == [-1.02]
    assert crossings(-1.021, -1.019) == [-1.02]
    assert crossings(1.381, 1.379) == [1.38]
    assert crossings(1.379, 1.381) == [1.38]


def test_tiles():
    line = Line((0.007, 0.003), (0.036, 0.023))
    assert line.tiles().sort() == [
        (0.0, 0.0),
        (0.01, 0.0),
        (0.01, 0.01),
        (0.02, 0.01),
        (0.03, 0.01),
        (0.03, 0.02)
    ].sort()

