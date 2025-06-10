#ifndef PARSING_H
#define PARSING_H

#include "structures.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

class CIFParser { // парсинг CIF файла
public:
    bool parse(const std::string& filename);
    const std::vector<CIFObject>& getObjects() const;
    const std::vector<CIFObject>& getObjects_add() const;
private:
    std::vector<CIFObject> objects_; // основной список обьектов
    std::vector<CIFObject> objects_add; // список обьектов подпрограмм
    static void trim(std::string& s);
    static void removeObjectById(std::vector<CIFObject>& objects, const int& idToRemove);
    static std::string parseLayer(const std::string& line);
    static Polygon parsePolygon(const std::string& line);
    static Point parseContact(const std::string& line);
    static Point parsePoint(const std::string& token);
};
#endif
