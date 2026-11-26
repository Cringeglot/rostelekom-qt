#ifndef PRIMITIVEITEM_H
#define PRIMITIVEITEM_H

#include <QGraphicsItem>
#include <QList>
#include <QColor>
#include "db/DatabaseManager.h"

class ConnectionLine;

// Базовый класс графического примитива
class PrimitiveItem : public QGraphicsItem
{
public:
    PrimitiveItem(int id, ShapeType type, qreal width, qreal height, QGraphicsItem* parent = nullptr);
    ~PrimitiveItem() override = default;

    // Фабричный метод создания нужного наследника
    static PrimitiveItem* create(int id, ShapeType type, qreal width, qreal height, int sides = 6);

    int id() const { return m_id; }
    ShapeType shapeType() const { return m_type; }
    qreal width() const { return m_width; }
    qreal height() const { return m_height; }

    QPointF centerInScene() const;

    void addConnection(ConnectionLine* line);
    void removeConnection(ConnectionLine* line);
    QList<ConnectionLine*> connections() const { return m_connections; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void drawGeometry(QPainter* painter) const = 0;
    virtual QColor fillColor() const = 0;

    int m_id;
    ShapeType m_type;
    qreal m_width;
    qreal m_height;

private:
    QList<ConnectionLine*> m_connections;
};


//ЭЛЛИПС
class EllipsePrimitive : public PrimitiveItem
{
public:
    EllipsePrimitive(int id, qreal width, qreal height, QGraphicsItem* parent = nullptr);
    QPainterPath shape() const override;

protected:
    void drawGeometry(QPainter* painter) const override;
    QColor fillColor() const override { return QColor(135, 206, 250, 200); }
};


//ПРЯМОУГЛЬНИК
class RectanglePrimitive : public PrimitiveItem
{
public:
    RectanglePrimitive(int id, qreal width, qreal height, QGraphicsItem* parent = nullptr);

protected:
    void drawGeometry(QPainter* painter) const override;
    QColor fillColor() const override { return QColor(144, 238, 144, 200); }
};

//МНОГОУГОЛЬНИК
class PolygonPrimitive : public PrimitiveItem
{
public:
    PolygonPrimitive(int id, qreal width, qreal height, int sides = 6, QGraphicsItem* parent = nullptr);
    QPainterPath shape() const override;
    int sides(){
        return m_sides;
    }

protected:
    void drawGeometry(QPainter* painter) const override;
    QColor fillColor() const override { return QColor(255, 182, 193, 200); }

private:
    QPolygonF buildPolygon() const;
    int m_sides;
};

#endif 