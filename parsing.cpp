#include "parsing.h"
bool CIFParser::parse(const std::string& filename) { // сам парсинг - необходимо исправить обработку call (пока работает на костыле)
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << "\n";
        return false;
    }
    std::string line; // строка файла
    std::string current_layer; // текущий слой
    int id, width, id_parent = -100, call=0, num_id = 1; // num_id - текущий id, call - кол-во вызовов подпрограмм, id - текущий номер подпрограммы, id_parent - id родительской подпрограммы (1-ой)
    std::unordered_map<int, std::streampos> ds_block_positions;
    bool flag_df = false;
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
                else flag_df = true;
                continue;
            }
            // записываем слой
            if (part[0] == 'L') {//if (part.substr(0, 2) == "L ") {
                current_layer = parseLayer(part);
                continue;
            }

            // Тут можно вызвать addObject, только предварительно нужно сохранить адресс строки + передать в функцию ЗНАЧЕНИЕ строки - не адрес (к нему, если что нужно будет вернуться). А значит остальные параметры (id_parent и layer нужно также передавать ЗНАЧЕНИЕ, а не ссылку.)
            if (flag_df == false) addObject(part, current_layer, num_id, id_parent, id, file, ds_block_positions, 0, 0);

            if (line.substr(0, 2) == "DF") {
                std::cout << "9\n";
                flag_df = true;
                break;  // Конец блока
            }

            // записываем полигон
            /*if (part[0] == 'P') {
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
            }*/ //*/
            // Игнорируем DF, E и другие команды
        }
        if (flag_df) break;
    }
    /*if (call>0){ // обрабатываем накопившиеся вызовы подпрограмм
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
    }*/
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

Polygon CIFParser::parsePolygon(const std::string& line, int _x, int _y) {
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
      poly.points.push_back({(std::stoi(x)+_x),(std::stoi(y)+_y)});
      //iss >> token;
      //if(token.empty()) break; // прочли весь файл
      //auto p = parsePoint(token); // получаем координаты
      //poly.points.push_back(p);
    }
    return poly;
}
Point CIFParser::parseContact(const std::string& line) {
  /*// Формат: T x/y;
  std::cout<<"line:  "<<line<<"\n";
  static const std::regex re(R"(T\s*(-?\d+)/(-?\d+))");
  std::smatch match;
  if(std::regex_search(line, match, re)){
      int x_int = stoi(match[1]);
      int y_int = stoi(match[2]);
      return {x_int,y_int};
  }
  return {0,0};*/
    // Формат: T x y;
    std::istringstream iss(line);
    std::string x, y;
    int x_=0, y_=0;
    iss>>x>>x>>x; // C (id); T
    iss>>x>>y; // Формат: x y
    if (x.empty()==false)x_=std::stoi(x);
    if (y.empty()==false)y_=std::stoi(y);
    return {x_,y_};
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

bool CIFParser::addObject(std::string part, std::string current_layer, int& num_id, const int& id_parent, int id, std::ifstream& file, std::unordered_map<int, std::streampos>& ds_block_positions, int x, int y){
    int width;
    if (part[0] == 'P') {
        // нет слоя у полигона
        if (current_layer.empty()) {
            std::cerr << "Polygon without layer\n";
            return false;
        }
        Polygon poly = parsePolygon(part, x, y);
        poly.updateBoundingBox();
        objects_.push_back({num_id, id_parent,current_layer, ObjectType::POLYGON, 0, poly, {}});
        num_id +=1;
        return true;
    }

    // записываем провод
    if (part[0] == 'W') {
        if (current_layer.empty()) {
            std::cerr << "Wire without layer\n";
            return false;
        }
        width = std::stoi(parseLayer(part));
        Polygon poly = parsePolygon(part, x, y);
        poly.updateBoundingBox();
        Polygon pol;
        wireToPolygon(poly, width, pol);
        objects_.push_back({num_id, id_parent, current_layer, ObjectType::POLYGON, width, pol});
        num_id +=1;
        return true;
    }

    if (part[0] == 'C') {
        if (addCall(part, current_layer, num_id, id_parent, id, file, ds_block_positions)){
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

bool CIFParser::addCall(std::string part, std::string current_layer, int& num_id, const int& id_parent, int id, std::ifstream& file, std::unordered_map<int, std::streampos>& ds_block_positions){
    // Получаем текущую позицию
    std::streampos return_pos = file.tellg();

    // Проверяем на ошибку
    if (return_pos == std::streampos(-1)) {
        file.clear(); // Сбрасываем флаги ошибок
        std::cout<<"Ошибка: file.tellg() = streampos(-1)\n";
    }

    // Пытаемся отступить на 1 символ назад
    if (return_pos > 0) {
        if (!file.seekg(-1, std::ios::cur)) {
            file.clear(); // Если seekg не удался
            std::cout<<"Ошибка: file.seekg(-1, std::ios::cur) = false\n";
        }
        return_pos = file.tellg(); // Обновляем позицию
    }

    if (current_layer.empty()) {
        std::cerr << "Contact without layer\n";
        return false;
    }
    Contact cont;
    cont.id_layer = std::stoi(parseLayer(part));;
    cont.points = parseContact(part);

    // Если позиция уже в кэше - используем её
    if (ds_block_positions.find(cont.id_layer) != ds_block_positions.end()) {
        file.seekg(ds_block_positions[cont.id_layer]);
    } else {
        // Иначе ищем DS-блок с начала файла
        file.seekg(0);
        std::string line;
        while (getline(file, line)) {
            std::istringstream line_stream(line);
            std::string token;
            if (line_stream >> token && token == "DS") {

                std::string current_ds_id;
                if (line_stream >> current_ds_id){
                    try {
                        int num = std::stoi(current_ds_id);  // Парсим строку в число
                        if (num == cont.id_layer) {
                            ds_block_positions[cont.id_layer] = file.tellg();
                            break;
                        }
                    } catch (...) {
                        std::cerr << "Ошибка: номер подпрограммы записан не числами!" << std::endl;
                    }
                }
            }
        }
    }

    // Обрабатываем DS-блок до конца (условный пример)
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("DF") != std::string::npos) {
            break;  // Конец блока
        }
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
        while (std::getline(line_stream, part, ';')) {
            trim(part);
            if (part.empty()) continue;

            // записываем слой
            if (part[0] == 'L') {
                current_layer = parseLayer(part);
                continue;
            }

            if (addObject(part, current_layer, num_id, id_parent, id, file, ds_block_positions, cont.points.x, cont.points.y)==false){
                    return false;}  // Рекурсивная обработка
        }
    }

    // Возвращаемся на сохранённую позицию
    if (!file.seekg(return_pos)) {
        file.clear();
        std::cout<<"Ошибка: file.seekg(return_pos) = false\n";
    }
    return true;
}

void CIFParser::wireToPolygon(Polygon wire, int width, Polygon& pol){
    int num = wire.points.size();
    switch(num)
    {
        case 1: {
            std::cout<<"case_1\n";
            pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y-width/2});
            pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y+width/2});
            pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y+width/2});
            pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y-width/2});
            pol.updateBoundingBox();
            break;
        };
        case 2: {
            std::cout<<"case_2\n";
            if ((wire.points[0].x==wire.points[1].x)&&(wire.points[0].y>wire.points[1].y)) {
                std::cout<<"1:\n";
                pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y+width/2});
                pol.points.push_back({wire.points[1].x-width/2, wire.points[1].y-width/2});
                pol.points.push_back({wire.points[1].x+width/2, wire.points[1].y-width/2});
                pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y+width/2});
            }
            if ((wire.points[0].x==wire.points[1].x)&&(wire.points[0].y<wire.points[1].y)){
                std::cout<<"2:\n";
                pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y-width/2});
                pol.points.push_back({wire.points[1].x+width/2, wire.points[1].y+width/2});
                pol.points.push_back({wire.points[1].x-width/2, wire.points[1].y+width/2});
                pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y-width/2});

            }//
            if ((wire.points[0].x<wire.points[1].x)&&(wire.points[0].y==wire.points[1].y)){
                std::cout<<"3:\n";
                pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y-width/2});
                pol.points.push_back({wire.points[1].x+width/2, wire.points[1].y-width/2});
                pol.points.push_back({wire.points[1].x+width/2, wire.points[1].y+width/2});
                pol.points.push_back({wire.points[0].x-width/2, wire.points[0].y+width/2});

            }
            if ((wire.points[0].x>wire.points[1].x)&&(wire.points[0].y==wire.points[1].y)){
                std::cout<<"4:\n";
                pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y+width/2});
                pol.points.push_back({wire.points[1].x-width/2, wire.points[1].y+width/2});
                pol.points.push_back({wire.points[1].x-width/2, wire.points[1].y-width/2});
                pol.points.push_back({wire.points[0].x+width/2, wire.points[0].y-width/2});
            }
            pol.updateBoundingBox();
            break;
        };
        case 3: {
            std::cout<<"case_3\n";
            //
            break;
        };

        default: {
            std::cout<<"default\n";
            //
            break;
        };
    }
}
