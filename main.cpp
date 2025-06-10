#include "mainwindow.h"
#include <QApplication>
//#include "parsing.h"
#include "functions.h"

// ПАРАМЕТРЫ
#define FILENAME "cheker_p.cif"//"cheker.cif"//"bounding_box.cif"//"wire_width.cif"//"examination.cif" //"transistor.cif" //sum.cif
#define MIN_DIST 120
#define MIN_D 120
#define MIN_W 120
#define MIN_IN 120
#define MIN_OUT 120

int main(int argc, char *argv[])
{
    std::vector<Polygon> error;
    bool fl1 = true, fl2 = true, fl3 = true, fl4 = true; // true, false
    std::vector<ListCheck> list_d, list_w, list_in, list_out;
    menu(error, list_d, list_w, list_in, list_out, fl1, fl2, fl3, fl4);

    //QApplication a(argc, argv);
    //MainWindow w(box, objs, error);
    //w.show();
    //return a.exec();
}
