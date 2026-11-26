#ifndef SHAPECOUNTDELEGATE_H
#define SHAPECOUNTDELEGATE_H

#include <QStyledItemDelegate>

class GraphicsScene;

class ShapeCountDelegate: public QStyledItemDelegate{
    Q_OBJECT
public:
    explicit ShapeCountDelegate(GraphicsScene* scene, QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    GraphicsScene* m_scene = nullptr;

};


#endif