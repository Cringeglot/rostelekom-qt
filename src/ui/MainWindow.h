#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "db/DatabaseManager.h"
#include "scene/GraphicsScene.h"
#include "scene/PrimitiveItem.h"
#include "ShapeCountDelegate.h"

#include <QMainWindow>
#include <QSqlTableModel>


#include <QGraphicsView>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QMessageBox>
#include <QRandomGenerator>



class MainWindow: public QMainWindow{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void applyFilters();
    void resetFilters();


    void onDeleteSelectedClicked();
    void onShapeSelectionChanged(int index);
    void onModeChanged();
    void onAddPrimitiveClicked();
    void onConnectionCreated(int id1, int id2);

private:
    DatabaseManager m_db;
    QSqlTableModel* m_model = nullptr;

    QGraphicsView* m_graphicsView = nullptr;
    GraphicsScene* m_scene = nullptr;

    QComboBox* m_createTypeCombo = nullptr;
    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QSpinBox* m_sidesSpin = nullptr;
    QPushButton* m_addBtn = nullptr;

    QRadioButton* m_moveModeRadio = nullptr;
    QRadioButton* m_connectModeRadio = nullptr;

    QTableView* m_tableView = nullptr;
    QLineEdit* m_filterIdEdit = nullptr;
    QComboBox* m_filterTypeCombo = nullptr;
    QSpinBox* m_filterCountSpin = nullptr;

    QPushButton* m_resetFilterBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
};


#endif