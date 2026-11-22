#include "MainWindow.h"




MainWindow:: MainWindow(QWidget* parent): QMainWindow(parent){
    setWindowTitle("Графическое приложения для создание прмиитивов");
    resize(1200, 680);

    if(!m_db.initialize("primitives.db")){
        QMessageBox::critical(this, "Ошибка", "Не удалось подключится к базе данных" );
        return;
    }

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    //разделяю экран на пополам: слева сцена с графикой, справа фильтры
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    //Левая часть: графическая сцена
    m_scene = new QGraphicsScene(this);
    m_scene -> setSceneRect(0, 0, 600, 600);

    m_graphicsView = new QGraphicsView(m_scene, this);
    m_graphicsView->setRenderHint(QPainter:: Antialiasing);

    
    splitter->addWidget(m_graphicsView);

    //Правая часть: фильтры, таблица и управление 
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    splitter->addWidget(rightPanel);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter);

    //Блок с фильтрами
    QGroupBox* filterGroup = new QGroupBox("Фильтры отображения", this);
    QHBoxLayout* filterLayout = new QHBoxLayout(filterGroup);

    m_filterIdEdit = new QLineEdit(this);
    m_filterIdEdit->setPlaceholderText("Id( например 1, 67)");

    m_filterTypeCombo = new QComboBox(this);
    m_filterTypeCombo-> addItem("Все типы");
    m_filterTypeCombo->addItem(DatabaseManager:: shapeTypeToString(ShapeType:: Ellipse));
    m_filterTypeCombo->addItem(DatabaseManager:: shapeTypeToString(ShapeType:: Rectangle));
    m_filterTypeCombo->addItem(DatabaseManager:: shapeTypeToString(ShapeType:: Polygon));


    m_filterCountSpin = new QSpinBox(this);
    m_filterCountSpin->setMinimum(0);
    m_filterCountSpin->setMaximum(777);
    m_filterCountSpin->setSpecialValueText("Все");
    
    m_resetFilterBtn = new QPushButton("Сбросить", this);

    filterLayout->addWidget(new QLabel("№: ", this));
    filterLayout->addWidget(m_filterIdEdit);
    filterLayout->addWidget(new QLabel("Тип: ", this));
    filterLayout->addWidget(m_filterTypeCombo);
    filterLayout->addWidget(new QLabel("Кол-во: ", this));
    filterLayout->addWidget(m_filterCountSpin);
    filterLayout->addWidget(m_resetFilterBtn);

    rightLayout->addWidget(filterGroup);

    //Таблица QTableView
    m_tableView = new QTableView(this);
    m_tableView->setSelectionBehavior(QAbstractItemView:: SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView:: SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView:: NoEditTriggers);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    rightLayout->addWidget(m_tableView);

    //Кнокпи управления в правой части
    QHBoxLayout* actionsLayout = new QHBoxLayout();
    m_testAddBtn = new QPushButton("Tect: добавить фигуру в бд", this);
    m_deleteBtn = new QPushButton("Удалить выбранную фигуру", this);

    actionsLayout->addWidget(m_testAddBtn);
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_deleteBtn);

    rightLayout->addLayout(actionsLayout);

    //Сигналы и слоты
    connect(m_filterIdEdit, &QLineEdit::textChanged, this, &MainWindow::applyFilters);
    connect(m_filterTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::applyFilters);
    connect(m_filterCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::applyFilters);
    connect(m_resetFilterBtn, &QPushButton::clicked, this, &MainWindow::resetFilters);

    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSelectedClicked);
    connect(m_testAddBtn, &QPushButton::clicked, this, &MainWindow::onTestAddClicked);




    //загрузка данных из бд в таблицу
    m_model = new QSqlTableModel(this, m_db.getDatabase());
    m_model->setTable("primitives");

    m_model->setHeaderData(0, Qt::Horizontal, "ID");
    m_model->setHeaderData(1, Qt::Horizontal, "Тип фигуры");
    m_model->setHeaderData(2, Qt::Horizontal, "Связи (ID)");
    m_model->setHeaderData(3, Qt::Horizontal, "Кол-во типа");

    m_model->select();
    m_tableView->setModel(m_model);
    m_tableView->resizeColumnsToContents();

}


void MainWindow:: applyFilters(){
    if(!m_model) return;

    QStringList conditions;

    //Фильтр по номерам фигур(можно веести один или несколько айди)
    const QString idText = m_filterIdEdit->text().trimmed();
    if(!idText.isEmpty()){
        QStringList validIds;
        const QStringList parts = idText.split(',', Qt::SkipEmptyParts);

        for(const QString& part: parts){
            bool ok = false;
            int id = part.trimmed().toInt(&ok);
            if(ok&& id>0){
                validIds.append(QString::number(id));
            }
        }

        if(!validIds.isEmpty()){
            conditions.append(QString("id IN (%1)").arg(validIds.join(",")));
        } else{
            conditions.append("1 = 0");
        }
    }


    //Фильтр по типу фигуры
    if (m_filterTypeCombo->currentIndex() > 0) {
        const QString typeStr = m_filterTypeCombo->currentText();
        conditions.append(QString("shape_type = '%1'").arg(typeStr));
    }

    //Фильтр по количеству фигур данного типа
    if (m_filterCountSpin->value() > 0) {
        conditions.append(QString("type_count = %1").arg(m_filterCountSpin->value()));
    }

    m_model->setFilter(conditions.join(" AND "));
    m_model->select();
}



void MainWindow::resetFilters()
{
    m_filterIdEdit->clear();
    m_filterTypeCombo->setCurrentIndex(0);
    m_filterCountSpin->setValue(0);
    applyFilters();
}



void MainWindow::onDeleteSelectedClicked()
{
    const QModelIndex current = m_tableView->currentIndex();
    if (!current.isValid()) {
        QMessageBox::information(this, "Удаление", "Выберите строку в таблице для удаления.");
        return;
    }

    int row = current.row();
    int id = m_model->data(m_model->index(row, 0)).toInt();

    if (m_db.removePrimitive(id)) {
        m_model->select();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить фигуру из базы данных.");
    }
}



void MainWindow::onTestAddClicked()
{
    int r = QRandomGenerator::global()->bounded(3);
    ShapeType type = static_cast<ShapeType>(r);
    m_db.addPrimitive(type);
    m_model->select();
}