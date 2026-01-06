#ifndef PARSING_H
#define PARSING_H
#include "structures.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>
#include <unordered_map>
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
    static Polygon parsePolygon(const std::string& line, int _x, int _y);
    static Point parseContact(const std::string& line);
    static Point parsePoint(const std::string& token);
    bool addObject(std::string part, std::string current_layer, int& num_id, const int& id_parent, int id, std::ifstream& file, std::unordered_map<int, std::streampos>& ds_block_positions, int x, int y);
    bool addCall(std::string part, std::string current_layer, int& num_id, const int& id_parent, int id, std::ifstream& file, std::unordered_map<int, std::streampos>& ds_block_positions);
    void wireToPolygon(Polygon wire, int width, Polygon& pol);
};
#endif
