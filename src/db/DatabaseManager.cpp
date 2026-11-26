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


    const QString geomSql = R"(
        CREATE TABLE IF NOT EXISTS primitive_geometry (
            id INTEGER PRIMARY KEY,
            x REAL NOT NULL,
            y REAL NOT NULL,
            width REAL NOT NULL,
            height REAL NOT NULL,
            sides INTEGER DEFAULT 6
        )
    )";
    if(!query.exec(geomSql)){
        qCritical() << "Ошибка создания таблицы геометрии:" << query.lastError().text();
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

QString DatabaseManager:: shapeTypeToString(const ShapeType type){
    switch(type){
        case ShapeType::Ellipse: return "Эллипс";
        case ShapeType::Rectangle: return "Прямоугольник";
        case ShapeType::Polygon: return "Многоугольник";    
    }
    return "Неизвестная фигура";

}

ShapeType DatabaseManager::stringToShapeType(const QString& string){
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

    QSqlQuery delGeom(m_db);
    delGeom.prepare("DELETE FROM primitive_geometry WHERE id = :correctid");
    delGeom.bindValue(":correctid", id);
    if (!delGeom.exec()) {
        qCritical() << "Error delete primitive geometry:" << delGeom.lastError().text();
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
    int rowid = selectquery.value(0).toInt();
    QList<int> connects = parseConnectionsString(selectquery.value(1).toString());

    if(connects.removeAll(id)>0){
        QSqlQuery newquery(m_db);
        newquery.prepare("UPDATE primitives SET connected_ids = :corcon WHERE id = :corid");
        newquery.bindValue(":corcon", formatConnectionsList(connects));
        newquery.bindValue(":corid", rowid);
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


bool DatabaseManager::saveGeometry(int id, qreal x, qreal y, qreal width, qreal height, int sides){
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO primitive_geometry (id, x, y, width, height, sides) VALUES (:id, :x, :y, :w, :h, :s)");

    q.bindValue(":id", id);
    q.bindValue(":x", x);
    q.bindValue(":y", y);
    q.bindValue(":w", width);
    q.bindValue(":h", height);
    q.bindValue(":s", sides);

    return q.exec();
}


QList<PrimitiveGeometry> DatabaseManager:: loadAllPrimitives() const {
     QList<PrimitiveGeometry> list;
    QSqlQuery q(m_db);
    const QString sql = R"(
        SELECT p.id, p.shape_type, p.connected_ids,
               COALESCE(g.x, 150), COALESCE(g.y, 150),
               COALESCE(g.width, 90), COALESCE(g.height, 60),
               COALESCE(g.sides, 6)
        FROM primitives p
        LEFT JOIN primitive_geometry g ON p.id = g.id
    )";
    if (!q.exec(sql)) return list;

    while (q.next()) {
        PrimitiveGeometry item;
        item.id = q.value(0).toInt();
        item.type = stringToShapeType(q.value(1).toString());
        item.connections = parseConnectionsString(q.value(2).toString());
        item.x = q.value(3).toReal();
        item.y = q.value(4).toReal();
        item.width = q.value(5).toReal();
        item.height = q.value(6).toReal();
        item.sides = q.value(7).toInt();
        list.append(item);
    }
    return list;
}