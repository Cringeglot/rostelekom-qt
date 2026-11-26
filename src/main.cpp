#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QSqlQuery>

int main(int argc, char* argv[]){


    QApplication a(argc, argv);

    MainWindow window;
    window.show();

    return a.exec();


}