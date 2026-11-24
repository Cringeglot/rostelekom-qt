#ifndef CONNECTIONLINE_H
#define CONNECTIONLINE_H

#include <QGraphicsLineItem>
#include "PrimitiveItem.h"

class ConnectionLine: public QGraphicsLineItem{
    public:
    ConnectionLine(PrimitiveItem* startItem, PrimitiveItem* endItem, QGraphicsItem* parent = nullptr );
    ~ConnectionLine() override = default;

    void updatePosition();
    void detach();

    PrimitiveItem* startItem() const {return m_startItem;}
    PrimitiveItem* endItem() const { return m_endItem; }

    bool connects(int id1, int id2) const;

private:
    PrimitiveItem* m_startItem = nullptr;
    PrimitiveItem* m_endItem = nullptr;
};



#endif