#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <cmath>
#include <optional>
#include <cstdlib>
#include "parsing.h"

bool isPointOnSegment(const Point& pt, const Point& p1, const Point& p2);
std::vector<Point> findIntersection(const Point& circle, const double& r, const Point& p1, const Point& p2);
std::vector<Point> pointsOfIntersection(const Polygon& p1, const Polygon& p2, const double& distance);
Point findPivot(const std::vector<Point>& points);
double cross(const Point& O, const Point& A, const Point& B);
void sortPointsForPolygon(std::vector<Point>& points);
bool isIntersect(const Rect& a, const Rect& b);
bool getAABBIntersection(const Rect& a, const Rect& b, Rect& intersection);
std::vector<Wire> check_width(const std::vector<CIFObject>& objs, int min_width);
std::vector<Wire> check_width_poly(const std::vector<CIFObject>& objs, int min_width);
bool check_gap(const Polygon& p1, const Polygon& p2, int min_gap);
//std::vector<Rect> check_box(std::vector<Rect> list_Rect, int min_dist);
std::vector<Polygon> check_polygon(std::vector<CIFObject>& objs, int min_dist);
std::vector<CIFObject> sort_x(std::vector<CIFObject> sort_list);
void checker(const std::vector<CIFObject>& objs, int min_dist, int min_d, int min_w, int min_in, int min_out, std::vector<Polygon>& error,
              const bool& fl1, const bool& fl2, const bool& fl3, const bool& fl4,
              std::vector<ListCheck> list_d, std::vector<ListCheck> list_w, std::vector<ListCheck> list_in, std::vector<ListCheck> list_out);
//void check_d_orthogonal(const std::vector<CIFObject>& objs, const CIFObject& element, const int min_d, std::vector<Polygon>& error);
void check_d_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_d, std::vector<Polygon>& error);
void check_out_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_out, std::vector<Polygon>& error);
bool listEdge(const Polygon& p, const int id, EdgeList& new_list);
void delete_span(std::vector<CIFObject>& objs, CIFObject& element, int min_dist);
void check_m_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_w, std::vector<Polygon>& error);
void check_in_orthogonal(const CIFObject& e1, const CIFObject& e2, const int min_in, std::vector<Polygon>& error);
void condition(std::vector<EdgeId>&  open, std::vector<EdgeId>& close, const EdgeCoords& _li, int& id, const bool& fl);
void bracketSequence(std::vector<EdgeCoords>& _list, std::vector<EdgeId>& open, std::vector<EdgeId>& close, const bool& fl);
void checkBracket(const CIFObject& e1, const CIFObject& e2, const int min_w, const int min_in, const int min_out, std::vector<Polygon>& error, const bool& fl2, const bool& fl3, const bool& fl4);
void read(std::string& filename, std::vector<ListCheck>& list_d, std::vector<ListCheck>& list_w, std::vector<ListCheck>& list_in, std::vector<ListCheck>& list_out);
void write(std::string& filename, const std::vector<Polygon>& error);
std::string get_filename_from_user(const bool& fl);
int findMaxValue(const std::vector<ListCheck>& list_d);
std::vector<CIFObject> menu(std::vector<Polygon>& error, std::vector<ListCheck>& list_d, std::vector<ListCheck>& list_w, std::vector<ListCheck>& list_in, std::vector<ListCheck>& list_out, bool& fl1, bool& fl2, bool& fl3, bool& fl4);

#endif
