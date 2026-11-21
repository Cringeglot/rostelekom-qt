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

    static QString shapeTypeToString(ShapeType type);
    static ShapeType stringToShapeType(QString& string);



private:
    QString formatConnectionsList(const QList<int>& ids) const;
    QList<int> parseConnectionsString(const QString& str) const;

    bool recalculateTypeCounts();

    QSqlDatabase m_db;
};



#endif