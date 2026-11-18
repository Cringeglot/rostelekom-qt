#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <algorithm>


DatabaseManager::  DatabaseManager(QObject* parent): QObject(parent){}

DatabaseManager:: ~DatabaseManager(){
    if(m_db.isOpen()) m_db.close();
}



bool DatabaseManager::initialize(const QString& dbPath){
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);

    if(!m_db.open()){
        qCritical() << "Error opening database " << m_db.lastError().text();
        return false;
    }

    QSqlQuery query(m_db);
    const QString sqlquery = R"(
        CREATE TABLE IF NOT EXISTS primitives(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            shape_type TEXT NOT NULL,
            connected_ids TEXT DEFAULT '',
            type_count INTEGER DEFAULT 0
        )
    )";

    if(!query.exec(sqlquery)){
        qCritical() << "Error creating table " << query.lastError().text();
        return false;
    }

    return true;

}


QSqlDatabase DatabaseManager:: getDatabase() const{
    return m_db;
}


int DatabaseManager:: addPrimitive(ShapeType type){
    if(!m_db.isOpen()){
        qCritical() << "Error opening database " << m_db.lastError().text();
        return -1;
    }

    m_db.transaction();

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO primitives(shape_type, connected_ids, type_count)" "VALUES(:type, '', 0)");
    query.bindValue(":type", shapeTypeToString(type));

    if(!query.exec()){
        qCritical()<<"Error add figure " << query.lastError().text();
        m_db.rollback();
        return -1;
    }

    int newId = query.lastInsertId().toInt();

    if(!recalculateTypeCounts()){
        m_db.rollback();
        return -1;
    }

    m_db.commit();
    return newId;
}

bool DatabaseManager:: recalculateTypeCounts(){
    QSqlQuery query(m_db);
    const QString sqlquery =R"(
    UPDATE primitives
    SET type_count = (
    SELECT COUNT(*)
    FROM primitives AS prcopy
    WHERE prcopy.shape_type = primitives.shape_type
    )
    )";

    if(!query.exec(sqlquery)){
        qCritical() << "Error Type Count" << query.lastError().text();
        return false;
    }

    return true;
}

QString DatabaseManager:: shapeTypeToString(ShapeType type){
    switch(type){
        case ShapeType::Ellipse: return "Эллипс";
        case ShapeType::Rectangle: return "Прямоугольник";
        case ShapeType::Polygon: return "Многоугольник";    
    }
    return "Неизвестная фигура";

}