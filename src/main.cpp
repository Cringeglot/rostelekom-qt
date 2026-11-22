#include "db/DatabaseManager.h"
#include "ui/MainWindow.h"
#include <QApplication>
#include <QDebug>
#include <QSqlQuery>

int main(int argc, char* argv[]){

    // QApplication a(argc, argv);

    // DatabaseManager db;
    // if (!db.initialize("test_primitives.db")) {
    //     return -1;
    // }

    // int id1 = db.addPrimitive(ShapeType::Rectangle);
    // int id2 = db.addPrimitive(ShapeType::Ellipse);
    // int id3 = db.addPrimitive(ShapeType::Rectangle);

    // db.addConnection(id1, id2);
    // db.addConnection(id2, id3);

    // qDebug() << "Созданы ID:" << id1 << id2 << id3;
    // qDebug() << "Созданы связи:" << id2 << ": "<< db.getConnections(id2);

    // db.removePrimitive(id2);

    // QSqlQuery q("SELECT id, shape_type, connected_ids, type_count FROM primitives");
    // while (q.next()) {
    //     qDebug() << "Строка БД:"
    //              << q.value(0).toInt()
    //              << q.value(1).toString()
    //              << "связи: "<< q.value(2).toString()
    //              << "всего такого типа:" << q.value(3).toInt();
    // }

    QApplication a(argc, argv);

    MainWindow window;
    window.show();

    return a.exec();


}