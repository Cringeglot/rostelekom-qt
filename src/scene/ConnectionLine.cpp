#include "ConnectionLine.h"
#include <QPen>

ConnectionLine:: ConnectionLine(PrimitiveItem* startItem, PrimitiveItem* endItem, QGraphicsItem* parent):QGraphicsLineItem(parent), m_endItem(endItem), m_startItem(startItem) {
    setPen(QPen(Qt:: darkGray, 2, Qt::SolidLine, Qt:: RoundCap, Qt:: RoundJoin));
    setZValue(-1);

    if(m_startItem) m_startItem->addConnection(this);
    if(m_endItem) m_endItem->addConnection(this);

    updatePosition();
}

void ConnectionLine:: updatePosition(){
    if (!m_startItem || !m_endItem) return;

    QLineF line(m_startItem->centerInScene(), m_endItem->centerInScene());
    setLine(line);
}

void ConnectionLine:: detach(){
    if(m_startItem){
        m_startItem->removeConnection(this);
        m_startItem=nullptr;
    }
    if (m_endItem) {
        m_endItem->removeConnection(this);
        m_endItem = nullptr;
    }
}

bool ConnectionLine:: connects(int id1, int id2) const {
    if(!m_startItem || !m_endItem) return false;
    int start = m_startItem->id();
    int end = m_endItem->id();

    return (id1 ==start && id2 == end) || (id2 ==start && id1 == end);
}