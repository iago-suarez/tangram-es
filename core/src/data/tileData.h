#pragma once

#include "glm/vec3.hpp"
#include "util/variant.h"

#include <vector>
#include <string>
#include <unordered_map>

/* Notes on TileData implementation:

Tile Coordinates:

  A point in the geometry of a tile is represented with 32-bit floating point x, y, and z coordinates. Coordinates represent
  normalized displacement from the origin (i.e. center) of a tile.

  (-1.0, 1.0) -------------------- (1.0, 1.0)
             |                    |
             |      +y ^          |
             |         | (0, 0)   |
             |       ----- > +x   |
             |         |          |
             |                    |
             |                    |
  (-1.0, -1.0)-------------------- (1.0, -1.0)

  Coordinates that fall outside the range [-1.0, 1.0] are permissible, as tile servers may chose not to clip certain geometries
  to tile boundaries, but in the future these points may be clipped in the client-side geometry processing.

  Z coordinates are expected to be normalized to the same scale as x asnd y coordinates.

Data heirarchy:

  TileData is a heirarchical container of structs modeled after the geoJSON spec: http://geojson.org/geojson-spec.html

  A <TileData> contains a collection of <Layer>s

  A <Layer> contains a name and a collection of <Feature>s

  A <Feature> contains a <GeometryType> denoting what variety of geometry is contained in the feature, a <Properties> struct
  describing the feature, and one collection each of <Point>s, <Line>s, and <Polygon>s. Only the geometry collection corresponding
  to the feature's geometryType should contain data.

  A <Properties> contains two key-value maps, one for string properties and one for numeric (floating point) properties.

  A <Polygon> is a collection of <Line>s representing the contours of a polygon. Contour winding rules follow the conventions of
  the OpenGL red book described here: http://www.glprogramming.com/red/chapter11.html

  A <Line> is a collection of <Point>s.

  A <Point> is 3 32-bit floating point coordinates representing x, y, and z (in that order).

*/
namespace Tangram {

enum GeometryType {
    unknown,
    points,
    lines,
    polygons
};

typedef glm::vec3 Point;

typedef std::vector<Point> Line;

typedef std::vector<Line> Polygon;

struct Properties {

    using Value = variant<none_type, std::string, float>;

    const Value& get(const std::string& key) const;

    void clear() { props.clear(); }

    bool contains(const std::string& key) const {
        return props.find(key) != props.end();
    }

    bool getNumeric(const std::string& key, float& value) const {
        auto it = props.find(key);
        if (it != props.end()) {
            // TODO: Cast if string?
            if (!it->second.is<float>()) { return false; }
            value = it->second.get<float>();
            return true;
        }
        return false;
    }

    float getNumeric(const std::string& key) const {
        auto it = props.find(key);
        if (it != props.end()) {
            if (!it->second.is<float>()) { return 0; }
            return it->second.get<float>();
        }
        return 0;
    }
    bool getString(const std::string& key, std::string& value) const {
        auto it = props.find(key);
        if (it != props.end()) {
            if (!it->second.is<std::string>()) { return false; }
            value = it->second.get<std::string>();
            return true;
        }
        return false;
    }

    std::string getString(const std::string& key) const {
        auto it = props.find(key);
        if (it != props.end()) {
            if (!it->second.is<std::string>()) { return ""; }
            return it->second.get<std::string>();
        }
        return "";
    }

    template <typename... Args> void add(std::string key, Args&&... args) {
        props.emplace(std::move(key), Value{std::forward<Args>(args)...});
    }
private:
    std::unordered_map<std::string, Value> props;

};

struct Feature {

    GeometryType geometryType = GeometryType::polygons;

    std::vector<Point> points;
    std::vector<Line> lines;
    std::vector<Polygon> polygons;

    Properties props;

};

struct Layer {

    Layer(const std::string& _name) : name(_name) {}

    std::string name;

    std::vector<Feature> features;

};

struct TileData {

    std::vector<Layer> layers;

};

}
