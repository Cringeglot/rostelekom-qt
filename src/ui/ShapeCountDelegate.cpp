#include "ShapeCountDelegate.h"
#include "db/DatabaseManager.h"
#include "scene/GraphicsScene.h"
#include "scene/PrimitiveItem.h"

#include <QApplication>
#include <QPainter>
#include <QPolygonF>
#include <cmath>

ShapeCountDelegate:: ShapeCountDelegate(GraphicsScene* scene, QObject* paret): QStyledItemDelegate(paret), m_scene(scene)
{
}


void ShapeCountDelegate:: paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const{

    //настройка визуала
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();

    const QWidget* widget = option.widget;
    QStyle* style = widget? widget->style(): QApplication:: style();

    style->drawControl(QStyle:: CE_ItemViewItem, &opt, painter, widget);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::black, 1.2));

    const int iconSize = qBound(12, option.rect.height() - 8, 18);
    const int spaicng = 6;
    const int startX = option.rect.left() +8;
    const int startY = option.rect.top()+ (option.rect.height() - iconSize)/2;

    //настройка типа фигуры
    QString typeStr = index.sibling(index.row(), 1).data(Qt::DisplayRole).toString();
    const ShapeType shapeType = DatabaseManager::stringToShapeType(typeStr);

    // Определяем число вершин для многоугольника по объекту на сцене
    int polygonSides = 6;
    if (shapeType == ShapeType::Polygon && m_scene) {
        int id = index.sibling(index.row(), 0).data(Qt::DisplayRole).toInt();
        if (auto* polyItem = dynamic_cast<PolygonPrimitive*>(m_scene->findPrimitiveById(id))) {
            polygonSides = polyItem->sides();
        }
    }

    //считаем колько нужно нарисовать и рисуем
    int count = index.data(Qt::DisplayRole).toInt();
    if(count <=0){
        return;
    }
    int iconCount;
    if(count<=3){
        iconCount = 1;
    } else  if(count <=10){
        iconCount = 2;
    } else{
        iconCount = 3;
    }


    for(int i = 0; i<iconCount; i++){
        QRectF rect(startX+i*(iconSize +spaicng), startY, iconSize, iconSize);

        switch (shapeType){
            case ShapeType:: Ellipse:
            painter->setBrush(QColor(135, 206, 250));
            painter->drawEllipse(rect);
            break;

            case ShapeType::Rectangle:
            painter->setBrush(QColor(144, 238, 144));
            painter->drawRect(rect);
            break;
            
            case ShapeType::Polygon:
            painter->setBrush(QColor(255, 182, 193));
            QPolygonF poly;
            const qreal rx = rect.width()/2.0;
            const qreal ry = rect.height()/2.0;
            const QPointF center = rect.center();
            const qreal pi = 3.14159265358979323846;

            for (int s = 0; s < polygonSides; ++s) {
                qreal angle = (2.0 * pi * s) / polygonSides - pi / 2.0;
                poly << QPointF(center.x() + rx * std::cos(angle), center.y() + ry * std::sin(angle));
            }
            painter->drawPolygon(poly);
            break;
            

        }
    }

    painter->restore();

}


QSize ShapeCountDelegate:: sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const{
    QSize base = QStyledItemDelegate::sizeHint(option, index);
    return QSize(qMax(base.width(), 95), qMax(base.height(), 28));
}
