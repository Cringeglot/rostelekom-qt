#include "PrimitiveItem.h"
#include "ConnectionLine.h"

#include <QPainter>
#include <QPainterPath>
#include <cmath>

PrimitiveItem::PrimitiveItem(int id, ShapeType type, qreal width, qreal height, QGraphicsItem* parent ):
     QGraphicsItem(parent),
     m_id(id),
     m_type(type),
     m_width(width),
     m_height(height)
{
setFlags(ItemIsMovable | ItemIsSelectable| ItemSendsGeometryChanges);
}



QPointF PrimitiveItem::centerInScene() const
{
    return scenePos();
}


void PrimitiveItem:: removeConnection(ConnectionLine* line){
    m_connections.removeAll(line);
}


void PrimitiveItem:: addConnection(ConnectionLine* line){
    if(line && !m_connections.contains(line)) m_connections.append(line);
}


QRectF PrimitiveItem:: boundingRect()const{
    qreal margin = 3.0;
    return QRectF(-m_width / 2.0 - margin, -m_height / 2.0 - margin,
                   m_width + 2.0 * margin, m_height + 2.0 * margin);
}


PrimitiveItem* PrimitiveItem:: create(int id, ShapeType type, qreal width, qreal height, int sides){
    switch(type){
    case ShapeType::Ellipse :
        return new EllipsePrimitive(id, width, height);
    case ShapeType::Polygon :
        return new PolygonPrimitive(id, width, height, sides);
    case ShapeType::Rectangle :
        return new RectanglePrimitive(id, width, height);
    }
    return new RectanglePrimitive(id, width, height);
    
}


void PrimitiveItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen(isSelected()? Qt::blue: Qt::black, isSelected()? 2.5: 1.5);
    painter->setPen(pen);
    painter->setBrush(fillColor());
    drawGeometry(painter);

    //подпись ID внутри фигуры
    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);

    painter->drawText(QRectF(-m_width / 2.0, -m_height / 2.0, m_width, m_height), Qt::AlignCenter, QString("#%1").arg(m_id));
}


QVariant PrimitiveItem:: itemChange(GraphicsItemChange change, const QVariant& value){
    if(change ==ItemPositionHasChanged){
        for(ConnectionLine* line: m_connections){
            if(line) line->updatePosition();
        }
    }
    return QGraphicsItem:: itemChange(change, value);
}



//----------------ЭЛЛИПС----------------//
EllipsePrimitive::EllipsePrimitive(int id, qreal width, qreal height, QGraphicsItem* parent) : PrimitiveItem(id, ShapeType::Ellipse, width, height, parent)
{
}

QPainterPath EllipsePrimitive:: shape() const{
    QPainterPath path;
    path.addEllipse(-m_width / 2.0, -m_height / 2.0, m_width, m_height);
    return path;
}

void EllipsePrimitive:: drawGeometry(QPainter* painter) const{
    painter->drawEllipse(QRectF(-m_width / 2.0, -m_height / 2.0, m_width, m_height));
}



//----------------ПРЯМОУГОЛЬНИК----------------//
RectanglePrimitive::RectanglePrimitive(int id, qreal width, qreal height, QGraphicsItem* parent)
    : PrimitiveItem(id, ShapeType::Rectangle, width, height, parent)
{
}

void RectanglePrimitive::drawGeometry(QPainter* painter) const
{
    painter->drawRect(QRectF(-m_width / 2.0, -m_height / 2.0, m_width, m_height));
}



//----------------МНОГОУГОЛЬНИК----------------//
PolygonPrimitive::PolygonPrimitive(int id, qreal width, qreal height, int sides, QGraphicsItem* parent)
    : PrimitiveItem(id, ShapeType::Polygon, width, height, parent), m_sides(qMax(3, sides))
{
}

QPolygonF PolygonPrimitive::buildPolygon() const
{
    QPolygonF poly;
    const qreal rx = m_width / 2.0;
    const qreal ry = m_height / 2.0;
    const qreal pi = 3.14159265358979323846;

    for (int i = 0; i < m_sides; ++i) {
        qreal angle = (2.0 * pi * i) / m_sides - pi / 2.0;
        poly << QPointF(rx * std::cos(angle), ry * std::sin(angle));
    }
    return poly;
}

QPainterPath PolygonPrimitive::shape() const
{
    QPainterPath path;
    path.addPolygon(buildPolygon());
    path.closeSubpath();
    return path;
}

void PolygonPrimitive::drawGeometry(QPainter* painter) const
{
    painter->drawPolygon(buildPolygon());
}




