#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>

enum class ShapeType{
    Ellipse,
    Rectangle,
    Polygon
};

struct PrimitiveGeometry {
    int id = -1;
    ShapeType type = ShapeType::Rectangle;
    qreal x = 100.0;
    qreal y = 100.0;
    qreal width = 90.0;
    qreal height = 60.0;
    int sides = 6;
    QList<int> connections;
};


class DatabaseManager: public QObject{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject* parent = nullptr);
    ~DatabaseManager();

    bool initialize(const QString& dbPath ="primitives");

    int addPrimitive(ShapeType type);

    bool addConnection(int id1, int id2);

    bool removePrimitive(int id);


    QList<int> getConnections(int id) const;

    QSqlDatabase getDatabase() const;

    static QString shapeTypeToString(const ShapeType type);
    static ShapeType stringToShapeType(const QString& string);


    bool saveGeometry(int id, qreal x, qreal y, qreal width, qreal height, int sides);
    QList<PrimitiveGeometry> loadAllPrimitives() const;



private:
    QString formatConnectionsList(const QList<int>& ids) const;
    QList<int> parseConnectionsString(const QString& str) const;

    bool recalculateTypeCounts();

    QSqlDatabase m_db;
};



#endif