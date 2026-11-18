#include "db/DatabaseManager.h"
#include <QApplication>
#include <QDebug>
#include <QSqlQuery>

int main(int argc, char* argv[]){

{
    QApplication a(argc, argv);

    DatabaseManager db;
    if (!db.initialize("test_primitives.db")) {
        return -1;
    }

    int id1 = db.addPrimitive(ShapeType::Rectangle);
    int id2 = db.addPrimitive(ShapeType::Ellipse);
    int id3 = db.addPrimitive(ShapeType::Rectangle);

    qDebug() << "Созданы ID:" << id1 << id2 << id3;

    QSqlQuery q("SELECT id, shape_type, connected_ids, type_count FROM primitives");
    while (q.next()) {
        qDebug() << "Строка БД:"
                 << q.value(0).toInt()
                 << q.value(1).toString()
                 << "связи: "<< q.value(2).toString()
                 << "всего такого типа:" << q.value(3).toInt();
    }

    return 0;
}


}