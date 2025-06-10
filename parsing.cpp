#include "parsing.h"


bool CIFParser::parse(const std::string& filename) { // сам парсинг
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return false;
    }

    std::string line; // строка файла
    std::string current_layer; // текущий слой
    int id, width, id_parent = -100, call=0, num_id = 1; // num_id - текущий id, call - кол-во вызовов подпрограмм

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;

        // Удаляем комментарии в скобках
        // надо оптимизировать - чтобы удался только комментарий, а не весь остаток строки
        size_t comment_pos = line.find('(');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
            trim(line);
        }
        if (line.empty()) continue; // пропускаем пустую строку

        // Разбиваем строку по ';'
        std::istringstream line_stream(line);
        std::string part;

        while (std::getline(line_stream, part, ';')) {
            trim(part);
            if (part.empty()) continue;

            //if (AddObject(id, part, current_layer, num_id, id_parent, call)==0){continue;}///////////////////////////////////////////////////////////

            // /*
            // первая подпрограмма - родитель
            if (part.substr(0, 3) == "DS ") {
                id = std::stoi(parseLayer(part));
                if (id_parent == -100){id_parent = id;}
                continue;
            }

            // записываем слой
            if (part[0] == 'L') {//if (part.substr(0, 2) == "L ") {
                current_layer = parseLayer(part);
                continue;
            }

            // записываем полигон
            if (part[0] == 'P') {
                // нет слоя у полигона
                if (current_layer.empty()) {
                    std::cerr << "Polygon without layer\n";
                    continue;
                }
                Polygon poly = parsePolygon(part);
                poly.updateBoundingBox();
                if (id == id_parent){
                    objects_.push_back({num_id, id,current_layer, ObjectType::POLYGON, 0, poly, {}});
                }
                else { // подпрограмма - скорее всего будет вызвана через C (call)
                    objects_add.push_back({num_id, id,current_layer, ObjectType::POLYGON, 0, poly, {}});
                }
                num_id +=1;
                continue;
            }

            // записываем провод
            if (part[0] == 'W') {
                if (current_layer.empty()) {
                    std::cerr << "Wire without layer\n";
                    continue;
                }
                width = std::stoi(parseLayer(part));
                Polygon poly = parsePolygon(part);
                poly.updateBoundingBox();
                if (id == id_parent){
                    objects_.push_back({num_id, id,current_layer, ObjectType::POLYGON, width, poly, {}});
                }
                else { // подпрограмма - скорее всего будет вызвана через C (call)
                    objects_add.push_back({num_id, id,current_layer, ObjectType::POLYGON, width, poly, {}});
                }
                num_id +=1;
                continue;
            }

            if (part[0] == 'C') { // вызов подпрограмм
                if (current_layer.empty()) {
                    std::cerr << "Contact without layer\n";
                    continue;
                }
                //Point pt = parseContact(part);
                Contact cont;
                cont.id_layer = std::stoi(parseLayer(part));;
                cont.points = parseContact(part);
                if (id == id_parent){
                    objects_.push_back({num_id, id, current_layer, ObjectType::CONTACT, 0, {}, cont});
                }
                else { // вызов в подпрограмме - скорее всего будет вызвана через C (call) - вообще странная вещь и она не будет нормально обрабатываться, в уелом такого и не должно быть
                    // потому что счетчик подпрограмм был изначально сделан для основной программы, а не для подпрграмм
                    // стоит сделать обработку ошибок вместо этого возможно - или нормальную обработку
                    objects_add.push_back({num_id, id, current_layer, ObjectType::CONTACT, 0, {}, cont});
                }
                call += 1;
                continue;
            }//*/


            // Игнорируем DF, E и другие команды
        }
    }

    if (call>0){ // обрабатываем накопившиеся вызовы подпрограмм
        // обработка C
        size_t _len = objects_.size();;
        std::vector<int> id_list;
        for(auto& obj : objects_){
            // перебор
            if(obj.type==ObjectType ::CONTACT){
                id_list.push_back(obj.id); // ищем вызов подпрограммы в основной программе
                // добавляем id найденных контактов
                if ((obj.contact.id_layer != id_parent) && (obj.id_layer == id_parent)){
                    // слой - родитель, вызов - подпрограмма
                    for(auto& obj2 : objects_add){
                        // ищем вызываемую подпрограмму
                        if (obj2.id_layer == obj.contact.id_layer){
                            // перебор координат - создаем НОВЫЕ
                            std::vector<Point> _points;
                            Point _point;
                            const auto& pts = obj2.polygon.points;
                            for(const auto& pt : pts){
                                //std ::cout << "\t("<<pt.x<<", "<<pt.y<<")\n";
                                _point.x = pt.x + obj.contact.points.x;
                                _point.y = pt.y + obj.contact.points.y;
                                _points.push_back(_point);
                                //std ::cout << "\t("<<pt.x<<", "<<pt.y<<")\n";
                            }
                            // добавляем обьект из подпрограммы в основную программу с новыми координатами
                            objects_.push_back({num_id, id_parent, obj2.layer, obj2.type, obj2.width, {_points, obj2.polygon.minX, obj2.polygon.minY, obj2.polygon.maxX, obj2.polygon.maxY,obj2.polygon.isBoundingBoxSet}, obj2.contact});
                            _len++;
                            objects_[_len-1].polygon.updateBoundingBox();
                            num_id +=1;
                        }
                    }
                }
            }
        }
        // удаляем из основного списка обьекта вызовы подпрограмм, которые уже обработали
        for(const auto& id_ : id_list){
            removeObjectById(objects_, id_);
        }
        call -= 1;
    }
    return true;
}

const std::vector<CIFObject>& CIFParser::getObjects() const { return objects_; }
const std::vector<CIFObject>& CIFParser::getObjects_add() const { return objects_add; }

// Реализация статических методов...

void CIFParser::trim(std::string& s) { // обрезаем строку
    const char* ws = " \t\n\r\f\v"; // все символы пробелов
    s.erase(0, s.find_first_not_of(ws)); // удаляем предшествующие пробелы
    s.erase(s.find_last_not_of(ws) + 1); // удаляем последние пробелы
}

void CIFParser::removeObjectById(std::vector<CIFObject>& objects, const int& idToRemove) {
    // Используем std::remove_if для перемещения объектов, которые нужно удалить, в конец
    auto newEnd = std::remove_if(objects.begin(), objects.end(),
                                 [idToRemove](const CIFObject& obj) { return obj.id == idToRemove; }); // Условие удаления
    // Удаляем элементы из вектора
    objects.erase(newEnd, objects.end());
}

std::string CIFParser::parseLayer(const std::string& line) {
    // Формат: L NA_
    size_t pos_space = line.find(' ');
    if (pos_space == std::string::npos)
        return "";
    // Берём всё после пробела и убираем возможные пробелы по краям
    std::string layer = line.substr(pos_space + 1);
    trim(layer);
    return layer;
}

Polygon CIFParser::parsePolygon(const std::string& line) {
    // Формат:
    // P x1/y1 x2/y2 ... ;
    // W width x1/y1 x2/y2 ... ;

    Polygon poly;

    std::istringstream iss(line);
    char cmd;
    iss >> cmd;

    if(cmd == 'W'){
        int width;
        iss >> width; // ширина
    }

    while(true){
        std::string x, y;
        iss >> x >> y; // Формат: x y
        if(x.empty()) break;
        if(y.empty()) break;
        poly.points.push_back({std::stoi(x),std::stoi(y)});
        //iss >> token;
        //if(token.empty()) break; // прочли весь файл
        //auto p = parsePoint(token); // получаем координаты
        //poly.points.push_back(p);
    }
    return poly;
}

Point CIFParser::parseContact(const std::string& line) {
    // Формат: T x/y;
    static const std::regex re(R"(T\s*(-?\d+)/(-?\d+))");
    std::smatch match;
    if(std::regex_search(line, match, re)){
        int x_int = stoi(match[1]);
        int y_int = stoi(match[2]);
        return {x_int,y_int};
    }
    return {0,0};
}

Point CIFParser::parsePoint(const std::string& token) {
    // Формат: x/y
    size_t slash_pos = token.find('/'); // ищет /
    if(slash_pos == std::string::npos){
        return {0,0};
    }
    int x_int = stoi(token.substr(0, slash_pos));
    int y_int = stoi(token.substr(slash_pos+1));
    return {x_int,y_int};
}


