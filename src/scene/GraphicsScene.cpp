#include "GraphicsScene.h"
#include "PrimitiveItem.h"
#include "ConnectionLine.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>


GraphicsScene::GraphicsScene(QObject* parent)
    : QGraphicsScene(parent)
{
    setSceneRect(0, 0, 700, 600);
}

void GraphicsScene:: setMode(Mode mode){
    m_mode = mode;
    if(m_tempLine){
        removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine = nullptr;
    }
    m_startItem = nullptr;
}

void GraphicsScene:: addPrimitiveItem(int id, ShapeType type, qreal width, qreal height, const QPointF& pos, int sides){
    PrimitiveItem* item = PrimitiveItem::create(id, type, width, height, sides);
    item->setPos(pos);
    addItem(item);
}

void GraphicsScene::removePrimitiveById(int id){
    PrimitiveItem* target = findPrimitiveById(id);
    if(!target) return;

    const QList<ConnectionLine*> lines = target->connections();
    for(ConnectionLine* line: lines){
        if(line){
            line->detach();
            removeItem(line);
            delete line;
        }
    }
    removeItem(target);
    delete target;
}

PrimitiveItem* GraphicsScene:: findPrimitiveById(int id) const{
    for(QGraphicsItem* item: items()){
        auto* prim = dynamic_cast<PrimitiveItem*>(item);
        if(prim&& prim->id() == id){
            return prim;
        }
    }
    return nullptr;
}

bool GraphicsScene:: hasConnection(int id1, int id2)const{
    PrimitiveItem* p = findPrimitiveById(id1);
    if (!p) return false;

    for (ConnectionLine* line : p->connections()) {
        if (line && line->connects(id1, id2)) {
            return true;
        }
    }
    return false;
}

PrimitiveItem* GraphicsScene::primitiveAt(const QPointF& pos) const{
    const QList<QGraphicsItem*> hitItems = items(pos);
    for(QGraphicsItem* item: hitItems){
        if(auto* prim =dynamic_cast<PrimitiveItem*>(item)){
            return prim;
        }
    }
    return nullptr;
}

void GraphicsScene:: mousePressEvent(QGraphicsSceneMouseEvent* event){
    if(m_mode == Mode::Connect && event->button() == Qt::LeftButton){
        PrimitiveItem* hit = primitiveAt(event->scenePos());
        if(hit){
            m_startItem = hit;
            m_tempLine = new QGraphicsLineItem(QLineF(hit->centerInScene(), event->scenePos()));
            m_tempLine->setPen(QPen(Qt::blue, 2, Qt::DashLine));
            addItem(m_tempLine);
        }
        return;
    }
    QGraphicsScene:: mousePressEvent(event);
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    if (m_mode == Mode::Connect && m_tempLine && m_startItem) {
        m_tempLine->setLine(QLineF(m_startItem->centerInScene(), event->scenePos()));
        return;
    }

    QGraphicsScene::mouseMoveEvent(event);
}


void GraphicsScene:: mouseReleaseEvent(QGraphicsSceneMouseEvent* event){
    if(m_mode==Mode::Connect&& m_tempLine){
        removeItem(m_tempLine);
        delete m_tempLine;
        m_tempLine = nullptr;
    
    if(m_startItem){
        PrimitiveItem* endItem = primitiveAt(event->scenePos());
            if(endItem && endItem!= m_startItem && !hasConnection(m_startItem->id(), endItem->id())){
                auto* line = new ConnectionLine(m_startItem, endItem);
                addItem(line);
                emit connectionCreated(m_startItem->id(), endItem->id());
            }
        }
        m_startItem = nullptr;
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}