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

ShapeType DatabaseManager::stringToShapeType(QString& string){
    if(string == "Эллипс") return ShapeType:: Ellipse;
    else if( string == "Прямоугольник") return ShapeType:: Rectangle;
    return ShapeType::Polygon;
}


bool DatabaseManager::  addConnection(int id1, int id2){
    if(!m_db.isOpen() || id1 == id2 || id1<=0 || id2<=0){
        return false;
    }

    auto appendId = [&](int targetId, int linkedId)-> bool{
        QList<int> list = getConnections(targetId);
        if(!list.contains(targetId)){
            list.append(linkedId);
            std::sort(list.begin(), list.end());
        }

        QSqlQuery query(m_db);
        query.prepare("UPDATE primitives SET connected_ids = :con WHERE id = :correctid");
        query.bindValue(":con", formatConnectionsList(list));
        query.bindValue(":correctid", targetId);

        if(!query.exec()){
            qCritical() << "Error link update" << query.lastError().text();
            return false;
        }
        return true;
    };

    m_db.transaction();
    if(!appendId(id1, id2) || !appendId(id2, id1)){
        m_db.rollback();
        return false;
    }
    m_db.commit();
    return true;
}


QList<int> DatabaseManager:: getConnections(int id) const{
        QSqlQuery query(m_db);
        query.prepare("SELECT connected_ids FROM primitives WHERE id = :correctid");
        query.bindValue(":correctid", id);
        if(query.exec()&& query.next()){
            return parseConnectionsString(query.value(0).toString());
        }
}


QList<int> DatabaseManager:: parseConnectionsString(const QString& str) const{
    QList<int> result;
    const QStringList parts = str.split(',', Qt::SkipEmptyParts);

    for(const QString& part: parts){
        bool iscorrect = false;
        int val = part.trimmed().toInt(&iscorrect);
        if(iscorrect){
            result.append(val);
        }
    }

    return result;
}

QString DatabaseManager:: formatConnectionsList(const QList<int>& ids) const{
    QStringList parts;
    for( int id: ids){
        parts.append(QString::number(id));
    }
    return parts.join(", ");
}


bool DatabaseManager:: removePrimitive(int id){
    if(!m_db.isOpen() || id<=0) return false;

    m_db.transaction();

    //удаляем запись из бд
    QSqlQuery delquery(m_db);
    delquery.prepare("DELETE FROM primitives WHERE id = :correctid");
    delquery.bindValue(":correctid", id);
    if(!delquery.exec()){
        qCritical() << "Error delete primitive" << delquery.lastError().text();
        m_db.rollback();
        return false;
    }


//пересчитываем связи(минус фигура минус количество свзяей с другими)
QSqlQuery selectquery(m_db);
if(!selectquery.exec("SELECT id, connected_ids FROM primitives WHERE connected_ids != '' ")){
    qCritical() << "Connection selection error" << selectquery.lastError().text();
    m_db.rollback();
    return false;
}

while(selectquery.next()){
    int id = selectquery.value(0).toInt();
    QList<int> connects = parseConnectionsString(selectquery.value(1).toString());

    if(connects.removeAll(id)>0){
        QSqlQuery newquery(m_db);
        newquery.prepare("UPDATE primitives SET connected_ids = :corcon WHERE id = :corid");
        newquery.bindValue(":corcon", formatConnectionsList(connects));
        newquery.bindValue(":corid", id);
        if(!newquery.exec()){
            qCritical() << "Error connect clear" << newquery.lastError().text();
            m_db.rollback();
            return false;
        } 
    }
}

//пересчитываем общее количество фигур
if(!recalculateTypeCounts()){
    m_db.rollback();
    return false;
}

m_db.commit();
return true;

}