#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>
#include "db/DatabaseManager.h"

class PrimitiveItem;
class ConnectionLine;

class GraphicsScene: public QGraphicsScene{
    Q_OBJECT
public:
    enum class Mode{
        Move,
        Connect
    };

    explicit GraphicsScene(QObject* parent = nullptr);

    void setMode(Mode mode);
    Mode mode() const {return m_mode;}

    void addPrimitiveItem(int id, ShapeType type, qreal width, qreal height, const QPointF& pos, int sides =6);
    void removePrimitiveById(int id);

    PrimitiveItem* findPrimitiveById(int id) const;
    bool hasConnection(int id1, int id2) const;

signals:
    void connectionCreated(int id1, int id2);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;


private:
    PrimitiveItem* primitiveAt(const QPointF& pos) const;

    Mode m_mode = Mode::Move;
    PrimitiveItem* m_startItem = nullptr;
    QGraphicsLineItem* m_tempLine = nullptr;

};


#endif