#include  "functions.h"
//#include "mainwindow.h"
//#include <QApplication>

bool isPointOnSegment(const Point& pt, const Point& p1, const Point& p2){
    const double epsilon = 1e-9;
    // Проверяем, что точка лежит внутри квадрата, ограниченного концами сегмента
    bool withinX = (pt.x >= std::min(p1.x, p2.x) - epsilon) && (pt.x <= std::max(p1.x, p2.x) + epsilon);
    bool withinY = (pt.y >= std::min(p1.y, p2.y) - epsilon) && (pt.y <= std::max(p1.y, p2.y) + epsilon);
    return withinX && withinY;
}

std::vector<Point> findIntersection(const Point& circle, const double& r, const Point& p1, const Point& p2){
    std::vector<Point> intersection;

    auto checkAndPush = [&](double x, double y, const Point& p1, const Point& p2) {
        Point pt{(int)round(x), (int)round(y)};
        if (isPointOnSegment(pt, p1, p2))
            intersection.push_back(pt);
    };

    // Обработка вертикальной линии
    if (p2.x == p1.x) {
        // Вертикальная линия: x = p1.x
        double x = p1.x;
        // Расчёт по уравнению окружности: (x - cx)^2 + (y - cy)^2 = r^2
        double delta = r * r - (x - circle.x) * (x - circle.x);
        if (delta < 0) {
            // Нет пересечений
            return intersection;
        } else if (delta == 0) {
            // Одна точка пересечения
            double y = circle.y;
            checkAndPush(x, y, p1, p2);
            //intersection.push_back({(int)round(x), (int)round(y)});
        } else {
            // Две точки пересечения
            double sqrt_delta = sqrt(delta);
            double y1 = circle.y + sqrt_delta;
            double y2 = circle.y - sqrt_delta;
            checkAndPush(x, y1, p1, p2);
            checkAndPush(x, y2, p1, p2);
            //intersection.push_back({(int)round(x), (int)round(y1)});
            //intersection.push_back({(int)round(x), (int)round(y2)});
        }
        return intersection;
    }

    // Вычисляем коэффициент m и  b
    double m = static_cast<double>(p2.y - p1.y) / (p2.x - p1.x);
    double b = p1.y - m * p1.x;

    // Коэффициенты квадратного уравнения Ax^2 + Bx + C = 0
    double A = 1 + m * m;
    double B = -2 * circle.x + 2 * m * (b - circle.y);
    double C = circle.x * circle.x + (b - circle.y) * (b - circle.y) - r * r;

    // Вычисляем дискриминант
    double D = B * B - 4 * A * C;

    if (D > 0) {
        // Две точки пересечения
        double sqrt_D = sqrt(D);
        double x1 = (-B + sqrt_D) / (2 * A);
        double x2 = (-B - sqrt_D) / (2 * A);
        double y1 = m * x1 + b;
        double y2 = m * x2 + b;

        checkAndPush(x1, y1, p1, p2);
        checkAndPush(x2, y2, p1, p2);
        //intersection.push_back({(int)round(x1), (int)round(y1)});
        //intersection.push_back({(int)round(x2), (int)round(y2)});
    } else if (D == 0) {
        // Одна точка пересечения
        double x = -B / (2 * A);
        double y = m * x + b;

        checkAndPush(x, y, p1, p2);
        //intersection.push_back({(int)round(x), (int)round(y)});
    }
    return intersection;
}

std::vector<Point> pointsOfIntersection(const Polygon& p1, const Polygon& p2, const double& distance){
    std::vector<Point> result_points; // все точки пересечений - не отсортированные, есть точки на одном ребре

    // Вспомогенная лямбда для поиска пересечений окружности с ребром и добавления точек
    auto lambdaFindIntersection=[&](const Point& center, const Point& edgeStart, const Point& edgeEnd){
        auto pts=findIntersection(center, distance, edgeStart, edgeEnd);
        /*std ::cout << "center: ("<<center.x <<", "<<center.y<<"), edgeStart: ("<<edgeStart.x <<", "<<edgeStart.y<<"), edgeEnd:("<<edgeEnd.x <<", "<<edgeEnd.y<<")\n";
        for(const auto& pt : pts){
            std ::cout << "\t("<<pt.x<<", "<<pt.y<<")\n";
        }*/
        for(const auto& pt:pts){
            auto it = std::find(result_points.begin(), result_points.end(), pt);
            if (it == result_points.end()) {
                result_points.push_back(pt);
            }
        }
        if (pts.empty()){}
        else{auto it = std::find(result_points.begin(), result_points.end(), center);
            if (it == result_points.end()) {
                result_points.push_back(center);
            }}
    };

    // Перебираем все точки первого полигона как центры окружностей
    for(const auto& centerPoint:p1.points){
        size_t n=p2.points.size();
        for(size_t i=0;i<n;++i){
            const Point& edgeStart=p2.points[i];
            const Point& edgeEnd=p2.points[(i+1)%n]; // следующий по индексу или первый если последний

            lambdaFindIntersection(centerPoint, edgeStart, edgeEnd);
        }
    }
    // Аналогично — перебираем точки второго полигона как центры и ребра первого
    for(const auto& centerPoint:p2.points){
        size_t n=p1.points.size();
        for(size_t i=0;i<n;++i){
            const Point& edgeStart=p1.points[i];
            const Point& edgeEnd=p1.points[(i+1)%n]; // следующий по индексу или первый если последний

            lambdaFindIntersection(centerPoint, edgeStart, edgeEnd);
        }
    }
    return result_points;
}

// Функция для поиска опорной точки (минимальная y, при равенстве — минимальный x)
Point findPivot(const std::vector<Point>& points){
    // Инициализируем опорную точку первой в списке
    Point pivot = points[0];
    for (const auto& p : points) {
        // Проверяем, является ли текущая точка "лучше" (меньше по y или при равенстве y — по x)
        // Обновляем опорную точку, если нашли более подходящую
        if (p.y < pivot.y || (p.y == pivot.y && p.x < pivot.x)) {
            pivot = p;
        }
    }
    return pivot;
}

//векторное (или псевдоскалярное) произведение двух векторов
double cross(const Point& O, const Point& A, const Point& B){
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// Функция сортировки точек полигона
void sortPointsForPolygon(std::vector<Point>& points){
    /*
    // Находим крайние точки по X и Y
    auto minXIt = std::min_element(points.begin(), points.end(), [](const Point& a, const Point& b){ return a.x < b.x; });
    auto maxXIt = std::max_element(points.begin(), points.end(), [](const Point& a, const Point& b){ return a.x < b.x; });
    auto minYIt = std::min_element(points.begin(), points.end(), [](const Point& a, const Point& b){ return a.y < b.y; });
    auto maxYIt = std::max_element(points.begin(), points.end(), [](const Point& a, const Point& b){ return a.y < b.y; });

    Point dl; //down-left
    Point dr; //down-right
    Point ul; //up-left
    Point ur; //up-right
    */
}

bool isIntersect(const Rect& a, const Rect& b) {
    return !(a.ur.x < b.dl.x || b.ur.x < a.dl.x ||
             a.ur.y < b.dl.y || b.ur.y < a.dl.y);
}


// Возвращает true, если есть пересечение, и записывает результат в `intersection`
bool getAABBIntersection(const Rect& a, const Rect& b, Rect& intersection) {
    // Проверяем, есть ли пересечение
    if (!(isIntersect(a, b))){
        return false; // Нет пересечения
    }
    // Вычисляем границы пересечения
    intersection.ur.x = std::min(a.ur.x, b.ur.x);
    intersection.ur.y = std::min(a.ur.y, b.ur.y);
    intersection.dl.x = std::max(a.dl.x, b.dl.x);
    intersection.dl.y = std::max(a.dl.y, b.dl.y);
    return true;
}

/*std::vector<Wire> check_width(const std::vector<CIFObject>& objs, int min_width){ // проверка всех проводов на заданную ширину // Возможно стоит убрать, т.к. можно проверять при парсинге файла, или по выбору перед парсингом.
    std::vector<Wire> check_wire;
    for(const auto& obj : objs){
        if(obj.type==ObjectType ::WIRE){
        if (obj.width < min_width) {check_wire.push_back({obj.width, obj.polygon.points});} // width - вся ширина, а не 1/2
        }
    }
    return check_wire; // список проводов с неправильной шириной
}*/

//std::vector<Wire> check_width_poly(const std::vector<CIFObject>& objs, int min_width){ // проверка всех проводов на заданную ширину // Возможно стоит убрать, т.к. можно проверять при парсинге файла, или по выбору перед парсингом.
//    std::vector<Wire> check_wire;
/*for (size_t i = 0; i < objs.size(); ++i){
        if((objs[i].type==ObjectType ::POLYGON)&&(objs[i].layer=="SI_")){
            for (size_t j = i + 1; j < objs.size(); ++j) {
               if((objs[j].type==ObjectType ::POLYGON)&&(objs[j].layer=="SI_")){
                   //
               }
               else {continue;}
            }
        }
           else {continue;}
    }*/
/*    for(const auto& obj : objs){
        if((obj.type==ObjectType ::POLYGON)&&(obj.layer=="SI_")){
            const auto& pts = obj.polygon.points;
            for (size_t i = 0; i < pts.size()-1; ++i){
                for (size_t j = i + 2; j < pts.size()-1; ++j) {
                    if ((pts[i].y==pts[j].y)&&(abs(pts[i].x-pts[j].x)<min_width)){std ::cout << "ERROR!\n";}
                    else if ((pts[i].x==pts[j].x)&&(abs(pts[i].y-pts[j].y)<min_width)){std ::cout << "ERROR!\n";}
                }
            }
        }
    }
    for(const auto& obj : objs){
        if((obj.type==ObjectType ::POLYGON)&&(obj.layer=="SI_")){
        //if ((obj.layer=="SI_") || (obj.layer=="SN_") || (obj.layer=="SP_") || (obj.layer=="SZ_")){}
            //if (obj.width < min_width) {check_wire.push_back({obj.width, obj.polygon.points});} // width - вся ширина, а не 1/2
            const auto& pts = obj.polygon.points;
            for(const auto& pt : pts){
                //std ::cout << "\t("<<pt.x<<", "<<pt.y<<")\n";
            }
        }
    }
    return check_wire; // список проводов с неправильной шириной
}*/

bool check_gap(const Polygon& p1, const Polygon& p2, int min_gap){//std::vector<Point> check_gap(Polygon& p, Wire w, int min_gap){
    /*size_t count_ = w.points.size();
    Point first = w.points[0];
    Point last = w.points[count_-1];*/
    //...
    return false;
}

/*std::vector<Rect> check_box(std::vector<Rect> list_Rect, int min_dist){
    std::vector<Rect> result_rect;
    Rect box;
    Rect box2;
    size_t count_ = list_Rect.size();
    if (count_ >=2){
        for (size_t i = 0; i < list_Rect.size(); ++i) {
            box = list_Rect[i];
            for (size_t j = i + 1; j < list_Rect.size(); ++j) {
                box2 = list_Rect[j];
                //std ::cout << isIntersect({{box.dl.x-min_dist, box.dl.y-min_dist},{box.ur.x+min_dist, box.ur.y+min_dist}}, box2) << "\n";
                if ((box.dl.x!=box2.dl.x)&&(box.ur.x!=box2.ur.x)&&(box.dl.y!=box2.dl.y)&&(box.ur.y!=box2.ur.y)){
                    Rect intersection;
                    if (getAABBIntersection({{box.dl.x-min_dist, box.dl.y-min_dist},{box.ur.x+min_dist, box.ur.y+min_dist}}, box2, intersection)) {
                        result_rect.push_back({intersection.dl, intersection.ur});
                    }
                }
            }
        }
    }
    return result_rect; //список пересечений
}*/

std::vector<Polygon> check_polygon(std::vector<CIFObject>& objs, int min_dist){ //проверка всех полигонов на min_dist, фильтруя полигоны
    std::vector<Polygon> res_Pol;
    Polygon empty_Pol;
    //for(const auto& obj : objs){
    for (size_t i = 0; i < objs.size(); ++i){
        for (size_t j = i + 1; j < objs.size(); ++j) {
            if((objs[i].type==ObjectType ::POLYGON)&&(objs[j].type==ObjectType ::POLYGON)){
                if (objs[i].polygon.isBoundingBoxSet==false) {objs[i].polygon.updateBoundingBox();}
                if (objs[j].polygon.isBoundingBoxSet==false) {objs[i].polygon.updateBoundingBox();}
                if (isIntersect({{objs[i].polygon.minX-min_dist, objs[i].polygon.minY-min_dist},{objs[i].polygon.maxX+min_dist, objs[i].polygon.maxY+min_dist}}, {{objs[j].polygon.minX, objs[j].polygon.minY},{objs[j].polygon.maxX, objs[j].polygon.maxY}})){
                    auto pts = pointsOfIntersection(objs[i].polygon, objs[j].polygon, min_dist);
                    res_Pol.push_back({pts, empty_Pol.minX, empty_Pol.minY, empty_Pol.maxX, empty_Pol.maxY, empty_Pol.isBoundingBoxSet});
                }
            }
        }
    }
    return res_Pol;
}

std::vector<CIFObject> sort_x(std::vector<CIFObject> sort_list){
    //std::vector<CIFObject> sort_list;
    std::sort(sort_list.begin(), sort_list.end(), [](const CIFObject& a, const CIFObject& b){return a.polygon.minX < b.polygon.minX;});
    return sort_list;
}

void checker(const std::vector<CIFObject>& objs, int min_dist, int min_d, int min_w, int min_in, int min_out, std::vector<Polygon>& error,
             const bool& fl1, const bool& fl2, const bool& fl3, const bool& fl4,
             std::vector<ListCheck> list_d, std::vector<ListCheck> list_w, std::vector<ListCheck> list_in, std::vector<ListCheck> list_out){
    //предпологаем, что все элементы в списке - полигоны
    std::vector<CIFObject> span; //диапазон
    CIFObject element;
    size_t n = objs.size(), m = 0, k = 0; // n - размер списка обьектов, m - i поиска первого элемента-полигона, k - индекс первого элемента-полигона
    while(m<n){ //проход по всему списку (если нет полигонов...)
        if(objs[k].type == ObjectType ::POLYGON){break;} // нахожу первый элемент в списке, который является полигоном (если в списке есть не только полигоны...)
        else{k++;}
        m++;
    }
    if ((n>1)&&(k<n)){ // проверка для элементов списка, если список состоит из больше чем одного элемента и в нем есть полигоны
        span.push_back(objs[k]); // добавляю первый элемент в диапазон
        for(size_t i=k+1; i < n; ++i){
            if (objs[i].type == ObjectType ::POLYGON){
                //необходимо сформировать диапазон
                element = objs[i];
                //перебираю диапазон
                for(auto& obj:span){
                    //проверяем что фигуры подходят для проверки по y - через bounding box
                    if (isIntersect({{obj.polygon.minX-min_d, obj.polygon.minY-min_d},{obj.polygon.maxX+min_d, obj.polygon.maxY+min_d}}, {{element.polygon.minX,element.polygon.minY},{element.polygon.maxX,element.polygon.maxY}})){
                        //if (()||()){check_d_orthogonal(obj, element, min_d, error);}
                        if (fl1){ // если нужна проверка на расстояние
                            for (size_t _i=0; _i<list_d.size(); ++_i){
                                if (((obj.layer == list_d[_i].layer1) && (element.layer == list_d[_i].layer2))||((element.layer == list_d[_i].layer1) && (obj.layer == list_d[_i].layer2))){
                                    check_d_orthogonal(obj, element, list_d[_i].value, error);
                                }
                            }
                        }
                        /*if(fl4){
                            if((obj.layer=="SN")and(element.layer!="SN")){
                                std::cout << "\n" << "obj.layer=SI_ i: " << i << "\n\n";
                                check_out_orthogonal(obj, element, min_out, error);}
                            else if((element.layer=="SN")and(obj.layer!="SN")){
                                std::cout << "\n" << "element.layer=SN i: " << i << "\n\n";
                                check_out_orthogonal(element, obj, min_out, error);}
                        }*/
                        if (fl1||fl2||fl3){ // если нужна проверка на расстояние
                            int _m=-1, _n=-1, _l=-1;
                            for (size_t _i=0; _i<list_w.size(); ++_i){
                                if(((obj.layer == list_w[_i].layer1) || (element.layer == list_w[_i].layer2))||((element.layer == list_w[_i].layer1) || (obj.layer == list_w[_i].layer2))){_m=_i;break;}
                            }
                            for (size_t _i=0; _i<list_in.size(); ++_i){
                                if(((obj.layer == list_in[_i].layer1) && (element.layer == list_in[_i].layer2))||((element.layer == list_in[_i].layer1) && (obj.layer == list_in[_i].layer2))){_n=_i;break;}
                            }
                            for (size_t _i=0; _i<list_out.size(); ++_i){
                                if(((obj.layer == list_out[_i].layer1) && (element.layer == list_out[_i].layer2))||((element.layer == list_out[_i].layer1) && (obj.layer == list_out[_i].layer2))){_l=_i;break;}
                            }
                            if ((_m!=-1)&&(_n!=-1)&&(_l!=-1)){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, list_w[_m].value, list_in[_n].value, list_out[_l].value, error, fl2, fl3, fl4);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, list_w[_m].value, list_in[_n].value, list_out[_l].value, error, fl2, fl3, fl4);}
                            }
                            else if ((_n!=-1)&&(_l!=-1)){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, 0, list_in[_n].value, list_out[_l].value, error, false, fl3, fl4);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, 0, list_in[_n].value, list_out[_l].value, error, false, fl3, fl4);}
                            }
                            else if ((_m!=-1)&&(_l!=-1)){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, list_w[_m].value, 0, list_out[_l].value, error, fl2, false, fl4);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, list_w[_m].value, 0, list_out[_l].value, error, fl2, false, fl4);}
                            }
                            else if ((_m!=-1)&&(_n!=-1)){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, list_w[_m].value, list_in[_n].value, 0, error, fl2, fl3, false);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, list_w[_m].value, list_in[_n].value, 0, error, fl2, fl3, false);}
                            }
                            else if (_m!=-1){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, list_w[_m].value, 0, 0, error, fl2, false, false);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, list_w[_m].value, 0, 0, error, fl2, false, false);}
                            }
                            else if (_n!=-1){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, 0, list_in[_n].value, 0, error, false, fl3, false);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, 0, list_in[_n].value, 0, error, false, fl3, false);}
                            }
                            else if (_l!=-1){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, 0, 0, list_out[_l].value, error, false, false, fl4);}
                                if(element.layer.substr(0, 1) == "S"){checkBracket(obj, element, 0, 0, list_out[_l].value, error, false, false, fl4);}
                            }
                            //else{std::cout << "NOT S\n" << obj.layer << " " << element.layer << "\n" << obj.id << " " << element.id << "\n\n";}
                            /*if((((obj.layer == list_w[_i].layer1) && (element.layer == list_w[_i].layer2))||((element.layer == list_w[_i].layer1) && (obj.layer == list_w[_i].layer2)))&&
                                   (((obj.layer == list_in[_j].layer1) && (element.layer == list_in[_j].layer2))||((element.layer == list_in[_j].layer1) && (obj.layer == list_in[_j].layer2)))&&
                                   (((obj.layer == list_out[_k].layer1) && (element.layer == list_out[_k].layer2))||((element.layer == list_out[_k].layer1) && (obj.layer == list_out[_k].layer2)))){
                                if(obj.layer.substr(0, 1) == "S"){checkBracket(element, obj, list_w[_i].value, list_in[_j].value, list_out[_k].value, error, fl2, fl3, fl4);}
                                else if (element.layer.substr(0, 1) == "S"){checkBracket(obj, element, list_w[_i].value, list_in[_j].value, list_out[_k].value, error, fl2, fl3, fl4);}
                            }*/
                            /*if((obj.layer=="SN")and((element.layer!="SN"))){
                                std::cout << "\n" << "obj.layer=SI_ i: " << i << "\n" << obj.layer << " " << element.layer << "\n" << obj.id << " " << element.id << "\n\n";
                                checkBracket(element, obj, min_w, min_in, min_out, error, fl2, fl3, fl4);
                            }
                            else if((element.layer=="SN")and(obj.layer!="SN")){
                                std::cout << "\n" << "element.layer=SI_ i: " << i << "\n" << obj.layer << " " << element.layer << "\n" << obj.id << " " << element.id << "\n\n";
                                checkBracket(obj, element, min_w, min_in, min_out, error, fl2, fl3, fl4);
                            }*/
                        }
                    }
                }
            }
            //удалить лишние элементы из диапазона
            delete_span(span, element, min_dist);
            //добавить элемент в диапазон
            span.push_back(objs[i]);
        }
    }
}

void check_d_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_d, std::vector<Polygon>& error){
    //получаем список ребер двух многоугольников
    EdgeList edge1, edge2;
    int y_d1, y_d2, x_d1, x_d2, x1, x2, x3, x4, y1, y2, y3, y4;
    //bool fl4 = false;
    //проверяем фигуры подходят для проверки
    //if (isIntersect({{e1.polygon.minX-min_d, e1.polygon.minY-min_d},{e1.polygon.maxX+min_d, e1.polygon.maxY+min_d}}, {{e2.polygon.minX,e2.polygon.minY},{e2.polygon.maxX,e2.polygon.maxY}})){
    if((listEdge(e1.polygon, e1.id, edge1))and(listEdge(e2.polygon, e2.id, edge2))){
        //перебираем ребра
        for(auto& ed:edge1.edge){
            for(auto& el:edge2.edge){
                //std::cout << "ed, el:\n";
                //std::cout << ed.x1 << " " << ed.y1 << " "  << ed.x2 << " "  << ed.y2 << "\n";
                //std::cout << el.x1 << " " << el.y1 << " "  << el.x2 << " "  << el.y2 << "\n";
                //получаем x1, x2, x3, x4, y1, y2, y3, y4
                if((ed.x1<ed.x2) or (ed.y1>ed.y2)){x1=ed.x1, x2=ed.x2, y1=ed.y1, y2=ed.y2;}
                else{x1=ed.x2, x2=ed.x1, y1=ed.y2, y2=ed.y1;}
                if((el.x1<el.x2) or (el.y1>el.y2)){x3=el.x1, x4=el.x2, y3=el.y1, y4=el.y2;}
                else{x3=el.x2, x4=el.x1, y3=el.y2, y4=el.y1;}
                //std::cout << "x1-4, y1-4:\n";
                //std::cout << x1 << " " << y1 << " "  << x2 << " "  << y2 << "\n";
                //std::cout << x3 << " " << y3 << " "  << x4 << " "  << y4 << "\n\n\n";
                //проверка по вертикали
                if((x1==x2) and (x3==x4) and (abs(x1-x3)<min_d)){
                    //ситуация 1
                    if((y1==y3) and (y2==y4)){//std::cout << "!!!!!B1 " << e1.id << " " << e2.id << "\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x2,y2}, {x4,y4}, {x3,y3}}});}
                    //ситуация 2
                    if((y1>y3) and (y2<y4)){
                        //std::cout << "!!!!!B2_1\n";
                        error.push_back({std::vector<Point>{{x1,y3}, {x2,y4}, {x4,y4}, {x3,y3}}});
                        if (abs(x1 - x3)!=min_d){
                            y_d1=y3 + std::sqrt(min_d * min_d - (x1 - x3) * (x1 - x3));
                            y_d2=y4 - std::sqrt(min_d * min_d - (x2 - x4) * (x2 - x4));
                            if (y_d1>y1){y_d1=y1;}
                            if (y_d2<y2){y_d2=y2;}
                            error.push_back({std::vector<Point>{{x1,y3}, {x1,y_d1}, {x3,y3}}});
                            error.push_back({std::vector<Point>{{x2,y4}, {x2,y_d2}, {x4,y4}}});}
                    }
                    if((y1<y3) and (y2>y4)){
                        //std::cout << "!!!!!B2_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x2,y2}, {x4,y2}, {x3,y1}}});
                        if (abs(x1 - x3)!=min_d){
                            y_d1=y1 + std::sqrt(min_d * min_d - (x3 - x1) * (x3 - x1));
                            y_d2=y2 - std::sqrt(min_d * min_d - (x4 - x2) * (x4 - x2));
                            if (y_d1>y3){y_d1=y3;}
                            if (y_d2<y4){y_d2=y4;}
                            error.push_back({std::vector<Point>{{x1,y1}, {x3,y_d1}, {x3,y1}}});
                            error.push_back({std::vector<Point>{{x2,y2}, {x4,y_d2}, {x4,y2}}});}
                    }
                    //ситуация 3
                    if((y1>y3) and (y3>y2) and (y2>y4)){
                        //std::cout << "!!!!!B3_1\n";
                        error.push_back({std::vector<Point>{{x1,y3}, {x2,y2}, {x4,y2}, {x3,y3}}});
                        if (abs(x1 - x3)!=min_d){
                            y_d1=y3 + std::sqrt(min_d * min_d - (x1 - x3) * (x1 - x3));
                            y_d2=y2 - std::sqrt(min_d * min_d - (x4 - x2) * (x4 - x2));
                            if (y_d1>y1){y_d1=y1;}
                            if (y_d2<y4){y_d2=y4;}
                            error.push_back({std::vector<Point>{{x1,y3}, {x1,y_d1}, {x3,y3}}});
                            error.push_back({std::vector<Point>{{x2,y2}, {x4,y_d2}, {x4,y2}}});}
                    }
                    if((y1<y3) and (y1>y4) and (y2<y4)){
                        //std::cout << "!!!!!B3_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x2,y4}, {x4,y4}, {x3,y1}}});
                        if (abs(x1 - x3)!=min_d){
                            y_d1=y1 + std::sqrt(min_d * min_d - (x3 - x1) * (x3 - x1));
                            y_d2=y4 - std::sqrt(min_d * min_d - (x2 - x4) * (x2 - x4));
                            if (y_d1>y3){y_d1=y3;}
                            if (y_d2<y2){y_d2=y2;}
                            error.push_back({std::vector<Point>{{x1,y1}, {x3,y_d1}, {x3,y1}}});
                            error.push_back({std::vector<Point>{{x2,y4}, {x2,y_d2}, {x4,y4}}});}
                    }
                    //ситуация 4
                    if((y2>y3) and (y2-y3<min_d) and (x1!=x3)){//std::cout << "!!!!!B4_1\n";
                        error.push_back({std::vector<Point>{{x2,y2}, {x2,y3}, {x3,y3}, {x3,y2}}});}
                    if((y1<y4) and (y4-y1<min_d) and (x1!=x3)){//std::cout << "!!!!!B4_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x1,y4}, {x4,y4}, {x4,y1}}});}
                }
                //проверка по горизонтали
                if((y1==y2) and (y3==y4) and (abs(y1-y3)<min_d)){
                    //ситуация 1
                    if((x1==x3) and (x2==x4)){//std::cout << "!!!!!G1\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x2,y2}, {x4,y4}, {x3,y3}}});}
                    //ситуация 2
                    if((x1<x3) and (x2>x4)){
                        //std::cout << "!!!!!G2_1\n";
                        //std::cout << "x1: " << x1 << " y1: " << y1 << " x2: " << x2 << " y2: " << y2 << " x3: " << x3 << " y3: " << y3 << " x4: " << x4 << " y4: " << y4 << "\n";
                        error.push_back({std::vector<Point>{{x3,y1}, {x4,y2}, {x4,y4}, {x3,y3}}});
                        if (abs(y1 - y3)!=min_d){
                            x_d1=x3 - std::sqrt(min_d * min_d - (y1 - y3) * (y1 - y3));
                            x_d2=x4 + std::sqrt(min_d * min_d - (y2 - y4) * (y2 - y4));
                            if (x_d1<x1){x_d1=x1;}
                            if (x_d2>x2){x_d2=x2;}
                            error.push_back({std::vector<Point>{{x_d1,y1}, {x3,y1}, {x3,y3}}});
                            error.push_back({std::vector<Point>{{x_d2,y2}, {x4,y2}, {x4,y4}}});}
                    }
                    if((x1>x3) and (x2<x4)){
                        //std::cout << "!!!!!G2_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x2,y2}, {x2,y4}, {x1,y3}}});
                        if (abs(y1 - y3)!=min_d){
                            x_d1=x1 - std::sqrt(min_d * min_d - (y3 - y1) * (y3 - y1));
                            x_d2=x2 + std::sqrt(min_d * min_d - (y4 - y2) * (y4 - y2));
                            if (x_d1<x3){x_d1=x3;}
                            if (x_d2>x4){x_d2=x4;}
                            error.push_back({std::vector<Point>{{x1,y1}, {x1,y3}, {x_d1,y3}}});
                            error.push_back({std::vector<Point>{{x2,y2}, {x2,y4}, {x_d2,y4}}});}
                    }
                    //ситуация 3
                    if((x1<x3) and (x3<x2) and (x2<x4)){
                        //std::cout << "!!!!!G3_1\n";
                        error.push_back({std::vector<Point>{{x3,y1}, {x2,y2}, {x2,y4}, {x3,y3}}});
                        if (abs(y1 - y3)!=min_d){
                            x_d1=x3 - std::sqrt(min_d * min_d - (y1 - y3) * (y1 - y3));
                            x_d2=x2 + std::sqrt(min_d * min_d - (y4 - y2) * (y4 - y2));
                            if (x_d1<x1){x_d1=x1;}
                            if (x_d2>x4){x_d2=x4;}
                            error.push_back({std::vector<Point>{{x3,y3}, {x3,y1}, {x_d1,y1}}});
                            error.push_back({std::vector<Point>{{x2,y2}, {x2,y4}, {x_d2,y4}}});}
                    }
                    if((x1>x3) and (x1<x4) and (x2>x4)){
                        //std::cout << "!!!!!G3_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x4,y2}, {x4,y4}, {x1,y3}}});
                        if (abs(y1 - y3)!=min_d){
                            x_d1=x1 - std::sqrt(min_d * min_d - (y3 - y1) * (y3 - y1));
                            x_d2=x4 + std::sqrt(min_d * min_d - (y2 - y4) * (y2 - y4));
                            if (x_d1<x3){x_d1=x3;}
                            if (x_d2>x2){x_d2=x2;}
                            error.push_back({std::vector<Point>{{x1,y1}, {x1,y3}, {x_d1,y3}}});
                            error.push_back({std::vector<Point>{{x4,y4}, {x4,y2}, {x_d2,y2}}});}
                    }
                    //ситуация 4
                    if((x2<x3) and (x3-x2<min_d) and (y1!=y3)){//std::cout << "!!!!!G4_1\n";
                        error.push_back({std::vector<Point>{{x2,y2}, {x2,y3}, {x3,y3}, {x3,y2}}});}
                    if((x4<x1) and (x1-x4<min_d) and (y1!=y3)){//std::cout << "!!!!!G4_2\n";
                        error.push_back({std::vector<Point>{{x1,y1}, {x1,y4}, {x4,y4}, {x4,y1}}});}
                }
            }
        }
    }
    //}
}

void check_out_orthogonal(const CIFObject& g, const CIFObject& a, const int min_out, std::vector<Polygon>& error){
    //получаем список ребер двух многоугольников
    std::vector<Point> per; // на 1 точку - 2 ребра
    std::vector<int> _g; //id edge g
    std::vector<int> _a; //id edge a
    int edge1_id, edge2_id, id, k;
    EdgeCoords edg, edg1, edg2, ed, el; //ed - g, el - a;
    EdgeList edge1, edge2;
    int x1, x2, x3, x4, y1, y2, y3, y4, n=0; //n - кол-во точек пересечения (должно быть 4)
    if((listEdge(g.polygon, g.id, edge1))and(listEdge(a.polygon, a.id, edge2))){
        k = edge1.edge.size();
        //перебираем ребра
        for(size_t i=0; i<edge1.edge.size(); i++){
            ed=edge1.edge[i];
            for(size_t j=0; j<edge2.edge.size(); j++){
                el=edge2.edge[j];
                if((ed.x1<ed.x2) or (ed.y1>ed.y2)){x1=ed.x1, x2=ed.x2, y1=ed.y1, y2=ed.y2;}
                else{x1=ed.x2, x2=ed.x1, y1=ed.y2, y2=ed.y1;}
                if((el.x1<el.x2) or (el.y1>el.y2)){x3=el.x1, x4=el.x2, y3=el.y1, y4=el.y2;}
                else{x3=el.x2, x4=el.x1, y3=el.y2, y4=el.y1;}
                //вертикальный затвор
                if((x1==x2)and(y3==y4)){
                    if((x1>x3)and(x1<x4)and(y3<y1)and(y3>y2)){
                        n++;
                        per.push_back({x1,y3});
                        _g.push_back(i);
                        _a.push_back(j);
                    }
                }
                //горизонтальный затвор
                if((y1==y2)and(x3==x4)){
                    if((x3>x1)and(x3<x2)and(y1<y3)and(y1>y4)){
                        n++;
                        per.push_back({x3,y1});
                        _g.push_back(i);
                        _a.push_back(j);
                    }
                }
            }
        }
        //проверим расстояниие
        if (n==4){
            //пройтись по _g - найти одинаковые индексы
            for(int i=0; i<4; i++){
                for(int j=i+1; j<4; j++){
                    if(_a[i]==_a[j]){
                        //определить индексы ребер затвора
                        edge1_id = _g[i]; edge2_id = _g[j];
                        //определить и найти ребро между найденными ребрами затвора
                        if(((edge1_id==0)and(edge2_id==(k-2)))or((edge1_id==(k-2))and(edge2_id==0))){
                            id = k-1;
                        }
                        else{id = (edge1_id+edge2_id)/2;}
                        edg = edge1.edge[id];
                        //подсчитать расстояние и определить, если нарушение
                        //вертикальный затвор
                        if(edg.y1==edg.y2){
                            if(abs(edg.y1-edge2.edge[_a[i]].y1)<min_out){
                                error.push_back({std::vector<Point>{{edg.x1, edg.y1}, {edg.x1, edge2.edge[_a[i]].y1}, {edg.x2, edge2.edge[_a[i]].y2}, {edg.x2, edg.y2}}});
                            }
                        }
                        //горизонтальный затвор
                        if(edg.x1==edg.x2){
                            if(abs(edg.x1-edge2.edge[_a[i]].x1)<min_out){
                                error.push_back({std::vector<Point>{{edg.x1, edg.y1}, {edge2.edge[_a[i]].x1, edg.y1}, {edge2.edge[_a[i]].x2, edg.y2}, {edg.x2, edg.y2}}});
                            }
                        }
                    }
                }
            }
        }
        else if(n>0){
            std::cout << "Error: check min_out (number of points not 4).\n";
        }
    }
}

bool listEdge(const Polygon& p, const int id, EdgeList& new_list) {
    new_list.edge.clear();
    new_list.id = id;
    size_t n = p.points.size();
    if (n < 2) {
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = p.points[i];
        const Point& p2 = p.points[(i + 1) % n]; // следующая точка, с циклом
        new_list.edge.push_back({p1.x,p1.y,p2.x,p2.y});
    }
    return true;
}

void delete_span(std::vector<CIFObject>& objs, CIFObject& element, int min_dist){
    int elem_min_x = element.polygon.minX;
    auto it = objs.begin();
    while (it != objs.end()) {
        // Проверяем, нужно ли удалить этот элемент
        if ((elem_min_x - it->polygon.minX > min_dist) &&
            (elem_min_x - it->polygon.maxX > min_dist)) {
            ++it;  // Удаляем этот элемент
        } else {
            break; // Прекращаем удаление
        }
    }
    // Удаляем все элементы от начала до текущей позиции
    objs.erase(objs.begin(), it);
}

void check_m_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_w, std::vector<Polygon>& error){}
void check_in_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_in, std::vector<Polygon>& error){}

void bracketSequence(std::vector<EdgeCoords>& _list, std::vector<EdgeId>& open, std::vector<EdgeId>& close, const bool& fl){
    open.clear();
    close.clear();
    int id=0;
    for(size_t i=0; i<_list.size(); ++i){
        if(open.empty()){
            open.push_back({true, id++, _list[i].x1, _list[i].y1, _list[i].x2, _list[i].y2});
        }
        else{
            condition(open, close, _list[i], id, fl);
        }
    }
    /*std::cout << "Open edges:\n";
    for (const auto& edge : open) std::cout
        << edge.id << ", " << edge.access << ": "
        << edge.x1 << "," << edge.y1 << " -> " << edge.x2 << "," << edge.y2 << "\n";
    std::cout << "Closed edges:\n";
    for (const auto& edge : close) std::cout
        << edge.id << ", " << edge.access << ": "
        << edge.x1 << "," << edge.y1 << " -> " << edge.x2 << "," << edge.y2 << "\n";*/
}

void condition(std::vector<EdgeId>&  open, std::vector<EdgeId>& close, const EdgeCoords& _li, int& id, const bool& fl){
    EdgeId temp1, temp2, temp3, op;
    EdgeCoords tmp;
    for(size_t j=0; j<open.size(); ++j){
        op = open[j];
        if(op.access){
            //вертикальные ребра
            if(fl){
                if((_li.y1==op.y1)and(_li.y2==op.y2)){
                    open[j].access=false;
                    close.push_back({false, op.id, _li.x1, _li.y1, _li.x2, _li.y2});
                    return;
                }
                if ((_li.y1==op.y1)and(_li.y2<op.y2)){
                    temp1={false, id, op.x1, op.y1, op.x2,_li.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    temp2={true, id++, op.x1, _li.y2, op.x2, op.y2};
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.y1>open[j].y1)and(_li.y2==open[j].y2)){
                    temp1={true, id++, op.x1, op.y1, op.x2, _li.y1};
                    temp2={false, id, op.x1, _li.y1, op.x2, op.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.y1>open[j].y1)and(_li.y2<open[j].y2)){
                    temp1={true, id++, op.x1, op.y1, op.x2,_li.y1};
                    temp2={false, id, op.x1,_li.y1, op.x2,_li.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    temp3={true, id++, op.x1, _li.y2, op.x2, op.y2};
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.push_back(temp3);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.y1==op.y1)and(_li.y2>op.y2)){
                    open[j].access=false;
                    close.push_back({false, op.id, _li.x1, _li.y1, _li.x2, op.y2});
                    tmp = {_li.x1, op.y2, _li.x2, _li.y2};
                    condition(open, close, tmp, id, fl);
                    return;
                }
                if ((_li.y1<op.y1)and(_li.y2==op.y2)){
                    open[j].access=false;
                    tmp = {_li.x1, _li.y1, _li.x2, op.y1};
                    close.push_back({false, op.id, _li.x1, op.y1, _li.x2, _li.y2});
                    condition(open, close, tmp, id, fl);
                    return;
                }
                if ((_li.y1<op.y1)and(_li.y2>op.y2)){
                    open[j].access=false;
                    close.push_back({false, op.id, _li.x1, op.y1, _li.x2, op.y2});
                    tmp = {_li.x1, _li.y1, _li.x2, op.y1};
                    condition(open, close, tmp, id, fl);
                    tmp = {_li.x1, op.y2, _li.x2, _li.y2};
                    condition(open, close, tmp, id, fl);
                    return;
                }
            }
            //горизонтальные ребра
            else{
                if((_li.x1==op.x1)and(_li.x2==op.x2)){
                    open[j].access=false;
                    close.push_back({false, op.id, _li.x1, _li.y1, _li.x2, _li.y2});
                    return;
                }
                if ((_li.x1==op.x1)and(_li.x2<op.x2)){
                    temp1={false, id, op.x1, op.y1, _li.x2, op.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    temp2={true, id++, _li.x2, op.y1, op.x2, op.y2};
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.x1>open[j].x1)and(_li.x2==open[j].x2)){
                    temp1={true, id++, op.x1, op.y1, _li.x1, op.y2};
                    temp2={false, id, _li.x1, op.y1, op.x2, op.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.x1>open[j].x1)and(_li.x2<open[j].x2)){
                    temp1={true, id++, op.x1, op.y1, _li.x1, op.y2};
                    temp2={false, id, _li.x1, op.y1, _li.x2, op.y2};
                    close.push_back({false, id++, _li.x1, _li.y1, _li.x2, _li.y2});
                    temp3={true, id++, _li.x2, op.y1, op.x2, op.y2};
                    open.push_back(temp1);
                    open.push_back(temp2);
                    open.push_back(temp3);
                    open.erase(open.begin() + j);
                    return;
                }
                if ((_li.x1==op.x1)and(_li.x2>op.x2)){
                    open[j].access=false;
                    close.push_back({false, op.id, _li.x1, _li.y1, op.x2, _li.y2});
                    tmp = {op.x2, _li.y1, _li.x2, _li.y2};
                    condition(open, close, tmp, id, fl);
                    return;
                }
                if ((_li.x1<op.x1)and(_li.x2==op.x2)){
                    open[j].access=false;
                    tmp = {_li.x1, _li.y1, op.x1, _li.y2};
                    close.push_back({false, op.id, op.x1, _li.y1, _li.x2, _li.y2});
                    condition(open, close, tmp, id, fl);
                    return;
                }
                if ((_li.x1<op.x1)and(_li.x2>op.x2)){
                    open[j].access=false;
                    close.push_back({false, op.id, op.x1, _li.y1, op.x2, _li.y2});
                    tmp = {_li.x1, _li.y1, op.x1, _li.y2};
                    condition(open, close, tmp, id, fl);
                    tmp = {op.x2, _li.y2, _li.x2, _li.y2};
                    condition(open, close, tmp, id, fl);
                    return;
                }
            }
        }
    }
    open.push_back({true, id++, _li.x1, _li.y1, _li.x2, _li.y2});
    //std::cout << _li.x1 << " " << _li.y1 << " " << _li.x2 << " " << _li.y2 << "\n";
}

bool sortEdge(const Polygon& p, const int id, EdgeList& new_vertical, EdgeList& new_horizontal) {
    new_vertical.edge.clear();
    new_vertical.id = id;
    new_horizontal.edge.clear();
    new_horizontal.id = id;
    size_t n = p.points.size();
    if (n < 2) {
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        const Point& p1 = p.points[i];
        const Point& p2 = p.points[(i + 1) % n]; // следующая точка, с циклом
        if(p1.x==p2.x){
            if (p1.y<p2.y){new_vertical.edge.push_back({p1.x,p1.y,p2.x,p2.y});}
            else{new_vertical.edge.push_back({p2.x,p2.y,p1.x,p1.y});}}
        if(p1.y==p2.y){
            if (p1.x<p2.x){new_horizontal.edge.push_back({p1.x,p1.y,p2.x,p2.y});}
            else {new_horizontal.edge.push_back({p2.x,p2.y,p1.x,p1.y});}}
    }
    std::sort(new_vertical.edge.begin(), new_vertical.edge.end(), [](const EdgeCoords& a, const EdgeCoords& b){return a.x1 < b.x1;});
    std::sort(new_horizontal.edge.begin(), new_horizontal.edge.end(), [](const EdgeCoords& a, const EdgeCoords& b){return a.y1 < b.y1;});
    return true;
}

void checkBracket(const CIFObject& e1, const CIFObject& e2, const int min_w, const int min_in, const int min_out, std::vector<Polygon>& error, const bool& fl2, const bool& fl3, const bool& fl4){
    EdgeList e1_vertical, e1_horizontal, e2_vertical, e2_horizontal;
    std::vector<EdgeId> op1_vertical, op1_horizontal, op2_vertical, op2_horizontal;
    std::vector<EdgeId> cl1_vertical, cl1_horizontal, cl2_vertical, cl2_horizontal;
    if ((sortEdge(e1.polygon, e1.id,  e1_vertical, e1_horizontal)) && (sortEdge(e2.polygon, e2.id,  e2_vertical, e2_horizontal))){
        bracketSequence(e1_vertical.edge, op1_vertical, cl1_vertical, 1);
        bracketSequence(e1_horizontal.edge, op1_horizontal, cl1_horizontal, 0);
        bracketSequence(e2_vertical.edge, op2_vertical, cl2_vertical, 1);
        bracketSequence(e2_horizontal.edge, op2_horizontal, cl2_horizontal, 0);
        //e1 - активная область, e2 - затвор
        for(auto& op2:op2_vertical){
            auto it2 = std::find_if(cl2_vertical.begin(), cl2_vertical.end(), [op2](const EdgeId& obj) {return obj.id == op2.id;});
            //w open-close op2
            if (fl2){
                if((it2->x1 - op2.x1) < min_w){
                    error.push_back({std::vector<Point>{{op2.x1, op2.y1}, {it2->x1, it2->y1}, {it2->x2, it2->y2}, {op2.x2, op2.y2}}});
                }
            }

            for(auto& op1:op1_vertical){
                auto it1 = std::find_if(cl1_vertical.begin(), cl1_vertical.end(), [op1](const EdgeId& obj) {return obj.id == op1.id;});
                //in
                if(fl3){
                    //op1 - op2 - cl2 - cl1
                    if((op1.x1 < op2.x1)&&
                        (((op1.y1 <= op2.y1) && (op2.y1 <= op1.y2)) ||
                         ((op1.y1 <= op2.y2) && (op2.y2 <= op1.y2)) ||
                         ((op2.y1 <= op1.y1) && (op1.y1 <= op2.y2)) ||
                         ((op2.y1 <= op1.y2) && (op1.y2 <= op2.y2)))){
                        if((op2.x1 - op1.x1) < min_in){
                            if((op2.y1 >= op1.y1)&&(op2.y2 <= op1.y2)){
                                error.push_back({std::vector<Point>{{op1.x1, op2.y1}, {op2.x1, op2.y1}, {op2.x2, op2.y2}, {op1.x2, op2.y2}}});
                            }
                            if((op2.y1 <= op1.y1)&&(op2.y2 >= op1.y2)){
                                error.push_back({std::vector<Point>{{op1.x1, op1.y1}, {op2.x1, op1.y1}, {op2.x2, op1.y2}, {op1.x2, op1.y2}}});
                            }
                            if((op2.y1 < op1.y1)&&(op2.y2 < op1.y2)){
                                error.push_back({std::vector<Point>{{op1.x1, op1.y1}, {op2.x1, op1.y1}, {op2.x2, op2.y2}, {op1.x2, op2.y2}}});
                            }
                            if((op2.y1 > op1.y1)&&(op2.y2 > op1.y2)){
                                error.push_back({std::vector<Point>{{op1.x1, op2.y1}, {op2.x1, op2.y1}, {op2.x2, op1.y2}, {op1.x2, op1.y2}}});
                            }
                        }
                    }
                    if((((it1->y1 <= it2->y1) && (it2->y1 <= it1->y2)) ||
                         ((it1->y1 <= it2->y2) && (it2->y2 <= it1->y2)) ||
                         ((it2->y1 <= it1->y1) && (it1->y1 <= it2->y2)) ||
                         ((it2->y1 <= it1->y2) && (it1->y2 <= it2->y2)))
                        &&(it2->x1 < it1->x1)){
                        if((it1->x1 - it2->x1) < min_in){
                            if((it2->y1 >= it1->y1)&&(it2->y2 <= it1->y2)){
                                error.push_back({std::vector<Point>{{it2->x1, it2->y1}, {it1->x1, it2->y1}, {it1->x2, it2->y2}, {it2->x2, it2->y2}}});
                            }
                            if((it2->y1 <= it1->y1)&&(it2->y2 >= it1->y2)){
                                error.push_back({std::vector<Point>{{it2->x1, it1->y1}, {it1->x1, it1->y1}, {it1->x2, it1->y2}, {it2->x2, it1->y2}}});
                            }
                            if((it2->y1 < it1->y1)&&(it2->y2 < it1->y2)){
                                error.push_back({std::vector<Point>{{it2->x1, it1->y1}, {it1->x1, it1->y1}, {it1->x2, it2->y2}, {it2->x2, it2->y2}}});
                            }
                            if((it2->y1 > it1->y1)&&(it2->y2 > it1->y2)){
                                error.push_back({std::vector<Point>{{it2->x1, it2->y1}, {it1->x1, it2->y1}, {it1->x2, it1->y2}, {it2->x2, it1->y2}}});
                            }
                        }
                    }
                }


                //out op2 - op1 - cl2
                if(fl4){
                    if((op2.x1 < op1.x1)&&(op2.y1>op1.y1)&&(op2.y2<op1.y2)&&(op1.x1 < it2->x1)){
                        //std::cout << "OUT_1_1 " << e1.id << " " << e2.id << "\n";
                        if((op1.x1 - op2.x1) < min_out){
                            //std::cout << "OUT_1_2 " << e1.id << " " << e2.id << "\n";
                            error.push_back({std::vector<Point>{{op2.x2, op2.y2}, {op1.x2, op2.y2}, {op1.x1, op2.y1}, {op2.x1, op2.y1}}});
                        }
                    }
                    if((it1->x1 < it2->x1)&&(it2->y1>it1->y1)&&(it2->y2<it1->y2)&&(op2.x1 < it1->x1)){
                        //std::cout << "OUT_1_3 " << e1.id << " " << e2.id << "\n";
                        if((it2->x1 - it1->x1) < min_out){
                            //std::cout << "OUT_1_4 " << e1.id << " " << e2.id << "\n";
                            error.push_back({std::vector<Point>{{it1->x2, it2->y2}, {it2->x2, it2->y2}, {it2->x1, it2->y1}, {it1->x1, it2->y1}}});
                        }
                    }
                }
            }
        }
        for(auto& op2:op2_horizontal){
            auto it2 = std::find_if(cl2_horizontal.begin(), cl2_horizontal.end(), [op2](const EdgeId& obj) {return obj.id == op2.id;});
            //w open-close op2
            if(fl2){
                if((it2->y1 - op2.y1) < min_w){
                    error.push_back({std::vector<Point>{{op2.x1, op2.y1}, {op2.x2, op2.y2}, {it2->x2, it2->y2}, {it2->x1, it2->y1}}});
                }
            }

            for(auto& op1:op1_horizontal){
                auto it1 = std::find_if(cl1_horizontal.begin(), cl1_horizontal.end(), [op1](const EdgeId& obj) {return obj.id == op1.id;});
                //in
                if(fl3){
                    //op1 - op2 - cl2 - cl1
                    if((op1.y1 < op2.y1)&&
                        (((op1.x1 <= op2.x1) && (op2.x1 <= op1.x2)) ||
                         ((op1.x1 <= op2.x2) && (op2.x2 <= op1.x2)) ||
                         ((op2.x1 <= op1.x1) && (op1.x1 <= op2.x2)) ||
                         ((op2.x1 <= op1.x2) && (op1.x2 <= op2.x2)))){
                        if((op2.y1 - op1.y1) < min_in){
                            if((op2.x1 >= op1.x1)&&(op2.x2 <= op1.x2)){
                                error.push_back({std::vector<Point>{{op2.x1, op1.y1}, {op2.x2, op1.y2}, {op2.x2, op2.y2}, {op2.x1, op2.y1}}});
                            }
                            if((op2.x1 <= op1.x1)&&(op2.x2 >= op1.x2)){
                                error.push_back({std::vector<Point>{{op1.x1, op1.y1}, {op1.x2, op1.y2}, {op1.x2, op2.y2}, {op1.x1, op2.y1}}});
                            }
                            if((op2.x1 < op1.x1)&&(op2.x2 < op1.x2)){
                                error.push_back({std::vector<Point>{{op1.x1, op1.y1}, {op2.x2, op1.y2}, {op2.x2, op2.y2}, {op1.x1, op2.y1}}});
                            }
                            if((op2.x1 > op1.x1)&&(op2.x2 > op1.x2)){
                                error.push_back({std::vector<Point>{{op2.x1, op1.y1}, {op1.x2, op1.y2}, {op1.x2, op2.y2}, {op2.x1, op2.y1}}});
                            }
                        }
                    }
                    if((((it1->x1 <= it2->x1) && (it2->x1 <= it1->x2)) ||
                         ((it1->x1 <= it2->x2) && (it2->x2 <= it1->x2)) ||
                         ((it2->x1 <= it1->x1) && (it1->x1 <= it2->x2)) ||
                         ((it2->x1 <= it1->x2) && (it1->x2 <= it2->x2)))
                        &&(it2->y1 < it1->y1)){
                        if((it1->y1 - it2->y1) < min_in){
                            if((it2->x1 >= it1->x1)&&(it2->x2 <= it1->x2)){
                                error.push_back({std::vector<Point>{{it2->x1, it2->y1}, {it2->x2, it2->y2}, {it2->x2, it1->y2}, {it2->x1, it1->y1}}});
                            }
                            if((it2->x1 <= it1->x1)&&(it2->x2 >= it1->x2)){
                                error.push_back({std::vector<Point>{{it1->x1, it2->y1}, {it1->x2, it2->y2}, {it1->x2, it1->y2}, {it1->x1, it1->y1}}});
                            }
                            if((it2->x1 < it1->x1)&&(it2->x2 < it1->x2)){
                                error.push_back({std::vector<Point>{{it1->x1, it2->y1}, {it2->x2, it2->y2}, {it2->x2, it1->y2}, {it1->x1, it1->y1}}});
                            }
                            if((it2->x1 > it1->x1)&&(it2->x2 > it1->x2)){
                                error.push_back({std::vector<Point>{{it2->x1, it2->y1}, {it1->x2, it2->y2}, {it1->x2, it1->y2}, {it2->x1, it1->y1}}});
                            }
                        }
                    }
                }

                //out op2 - op1
                if(fl4){
                    if((op1.y1 > op2.y1)&&(op2.x1 > op1.x1)&&(op2.x2 < op1.x2)&&(it2->y1 > op1.y1)){
                        //std::cout << "OUT_2_1 " << e1.id << " " << e2.id << "\n";
                        if((op1.y1 - op2.y1) < min_out){
                            //std::cout << "OUT_2_2 " << e1.id << " " << e2.id << "\n";
                            error.push_back({std::vector<Point>{{op2.x2, op1.y2}, {op2.x2, op2.y2}, {op2.x1, op2.y1}, {op2.x1, op1.y1}}});
                        }
                    }
                    if((it2->y1 > it1->y1)&&(it2->x1 > it1->x1)&&(it2->x2 < it1->x2)&&(it1->y1 > op2.y1)){
                        //std::cout << "OUT_2_3 " << e1.id << " " << e2.id << "\n";
                        if((it2->y1 - it1->y1) < min_out){
                            //std::cout << "OUT_2_4 " << e1.id << " " << e2.id << "\n";
                            error.push_back({std::vector<Point>{{it2->x2, it2->y2}, {it2->x2, it1->y2}, {it2->x1, it1->y1}, {it2->x1, it2->y1}}});
                        }
                    }
                }
            }
        }
    }
}


void read(std::string& filename, std::vector<ListCheck>& list_d, std::vector<ListCheck>& list_w, std::vector<ListCheck>& list_in, std::vector<ListCheck>& list_out){
    // Очищаем векторы перед заполнением
    list_d.clear();
    list_w.clear();
    list_in.clear();
    list_out.clear();

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    std::vector<ListCheck>* current_list = nullptr;

    while (std::getline(file, line)) {
        // Удаляем пробелы в начале и конце строки
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.empty()) continue;  // Пропускаем пустые строки

        if (line == "d;") {
            current_list = &list_d;
        } else if (line == "w;") {
            current_list = &list_w;
        } else if (line == "in;") {
            current_list = &list_in;
        } else if (line == "out;") {
            current_list = &list_out;
        } else if (current_list != nullptr && line.back() == ';') {
            // Удаляем точку с запятой в конце
            line.pop_back();

            std::istringstream iss(line);
            ListCheck item;

            // Читаем два строковых значения и одно числовое
            if (iss >> item.layer1 >> item.layer2 >> item.value) {
                current_list->push_back(item);
                //std::cout<<item.layer1<<" "<<item.layer2<<" "<<item.value<<"\n";
            }
        }
    }
}

void write(std::string& filename, const std::vector<Polygon>& error){
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // Записываем слой ошибок
    file << "L ER_;\n";

    // Записываем каждый полигон
    for (const auto& polygon : error) {
        file << "P";
        for (const auto& point : polygon.points) {
            file << " " << point.x << " " << point.y;
        }
        file << ";\n";
    }
}

// Функция для безопасного ввода имени файла
std::string get_filename_from_user(const bool& fl) {
    std::string filename;
    bool is_valid = false;

    while (!is_valid) {
        if (fl){std::cout << "Enter the file name (with the .cif extension)): ";}
        else{std::cout << "Enter the file name (with the .txt extension)): ";}
        std::getline(std::cin, filename);

        // Проверяем, не пустое ли имя
        if (filename.empty()) {
            std::cout << "Error: file name cannot be empty!\n";
            continue;
        }

        if (fl){
            // Проверяем расширение .cif
            if (filename.size() < 4 ||
                filename.substr(filename.size() - 4) != ".cif") {
                std::cout << "Error: File must have extension .cif!\n";
                continue;
            }
        }
        else {
            // Проверяем расширение .txt
            if (filename.size() < 4 ||
                filename.substr(filename.size() - 4) != ".txt") {
                std::cout << "Error: File must have extension .txt!\n";
                continue;
            }
        }

        // Проверяем, существует ли файл
        std::ifstream test_file(filename);
        if (!test_file.good()) {
            std::cout << "Error: File does not exist or is not readable!\n";
            continue;
        }
        test_file.close();

        is_valid = true;
    }

    return filename;
}

int findMaxValue(const std::vector<ListCheck>& list_d) {
    if (list_d.empty()) {
        // Обработка случая с пустым вектором
        throw std::runtime_error("Vector is empty!");
    }

    // Находим элемент с максимальным value
    auto max_it = std::max_element(
        list_d.begin(),
        list_d.end(),
        [](const ListCheck& a, const ListCheck& b) {
            return a.value < b.value;
        }
        );

    return max_it->value;
}

std::vector<CIFObject> menu(std::vector<Polygon>& error, std::vector<ListCheck>& list_d, std::vector<ListCheck>& list_w, std::vector<ListCheck>& list_in, std::vector<ListCheck>& list_out, bool& fl1, bool& fl2, bool& fl3, bool& fl4){
    std::string filename, filename1, filename2;
    int min_d, min_w, min_in, min_out;
    std::vector<CIFObject> objs, _objs;
    while(true) {
        // Выводим меню
        std::cout << "\n=== Menu ===" << std::endl;
        std::cout << "\nEnter the name of the design rules file." << std::endl;
        filename="DRC.txt";//
        //filename = get_filename_from_user(false);
        std::cout << "\nEnter the name of the file with the scheme." << std::endl;
        filename1="cheker.cif";//"transistor.cif";//"cheker.cif"
        //filename1 = get_filename_from_user(true);
        CIFParser parser;
        if(!parser.CIFParser::parse(filename1)){
            std::cout<<"\nError parsing file!";
            break;
        }
        objs = parser.getObjects();
        for (const auto& _obj:objs){
            std::cout << _obj.id << "  " << _obj.id_layer << "  " << _obj.layer << "  " << _obj.width << "\n";// << _obj.polygon << "/n" << _obj.contact << "/n";
            if (_obj.type == ObjectType::POLYGON){
                for (const auto& pts:_obj.polygon.points){
                std::cout << pts.x << "  " << pts.y << "\n";
            }}
            else{
            std::cout << "C:\n" << _obj.contact.points.x << "  " << _obj.contact.points.y << "\n\n";}
        }

        _objs=sort_x(objs);
        //std::cout << "\nWhere do you want to output violations? \n1) in a file \n2) in the graphical interface" << std::endl;
        // Получаем выбор пользователя
        int choice=1;
        //std::cin >> choice;
        // Очищаем буфер ввода
        //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        read(filename, list_d, list_w, list_in, list_out);
        min_d = findMaxValue(list_d);
        min_w = findMaxValue(list_w);
        min_in = findMaxValue(list_in);
        min_out = findMaxValue(list_out);

        checker(_objs, min_d, min_d, min_w, min_in, min_out, error, fl1, fl2, fl3, fl4, list_d, list_w, list_in, list_out);

        if (choice==1){
            std::cout << "\nEnter the name of the file." << std::endl;
            filename2="write_error.cif";//
            //filename2 = get_filename_from_user(true);
            write(filename2, error);
            std::cout << "\n" << error.size() <<"\n";
            break;
        }
        /*else if (choice==2){

            MainWindow w(_objs, error);
            w.show();
        }*/
    }
    return objs;
}
