#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <vector>
#include <string>

struct Point { // P ./. ... ./.;
    int x=0;
    int y=0;

    // определение оператора ==
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Rect { // для bounding box
    Point dl; // нижняя левая точка
    Point ur; // верхняя правая точка
};

struct EdgeCoords{
    int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
};

struct EdgeList{
    int id=0;  // идентификатор объекта
    std::vector<EdgeCoords> edge;
};

struct EdgeId{
    bool access = true; //доступен? true - еще нет пары (откр-закр), поэтому доступен
    int id;
    int x1, y1, x2, y2;
};


struct Polygon {
    std::vector<Point> points;
    int minX = 0, minY=0, maxX = 0, maxY = 0;
    bool isBoundingBoxSet = false;

    void updateBoundingBox() {
        if (points.empty()) {
            minX = minY = maxX = maxY = 0;
            return;
        }

        const Point& first = points.front();
        int tmp_minX = first.x, tmp_maxX = first.x;
        int tmp_minY = first.y, tmp_maxY = first.y;

        for (const auto& pt : points) {
            if (pt.x < tmp_minX) tmp_minX = pt.x;
            if (pt.x > tmp_maxX) tmp_maxX = pt.x;
            if (pt.y < tmp_minY) tmp_minY = pt.y;
            if (pt.y > tmp_maxY) tmp_maxY = pt.y;
        }
        minX = tmp_minX; maxX = tmp_maxX;
        minY = tmp_minY; maxY = tmp_maxY;
        isBoundingBoxSet = true;
    }
};

struct Wire { // W (width) ./. ... ./.;
    int width;
    std::vector<Point> points;
};

struct Contact { // C (number object)   T ./.; //call object to point
    int id_layer; // идентификатор подпрограммы
    Point points;
};

enum class ObjectType {
    POLYGON,
    WIRE,
    CONTACT
    //BOX
    //ROUNDFLASH
};

// стоит оптимизировать структуру
struct CIFObject {
    int id;  // идентификатор объекта
    int id_layer; // идентификатор подпрограммы
    std::string layer; // слой объекта
    ObjectType type; // тип объекта
    int width; // для W - ширина
    Polygon polygon; // для P - вектор координат
    Contact contact;   // для C - координаты одной точки
};

struct SubroutineCIFObject {
    int id;  // идентификатор подпрограммы
    std::vector<CIFObject> objs;
};

struct ListCheck{
    std::string layer1;
    std::string layer2;
    int value;
};

#endif
