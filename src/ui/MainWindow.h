#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "db/DatabaseManager.h"

#include <QMainWindow>
#include <QSqlTableModel>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
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
    void onTestAddClicked();

private:
    DatabaseManager m_db;
    QSqlTableModel* m_model = nullptr;

    QGraphicsView* m_graphicsView = nullptr;
    QGraphicsScene* m_scene = nullptr;

    QTableView* m_tableView = nullptr;
    QLineEdit* m_filterIdEdit = nullptr;
    QComboBox* m_filterTypeCombo = nullptr;
    QSpinBox* m_filterCountSpin = nullptr;

    QPushButton* m_resetFilterBtn = nullptr;
    QPushButton* m_deleteBtn = nullptr;
    QPushButton* m_testAddBtn = nullptr;
};


#endif