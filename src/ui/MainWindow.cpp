#include "MainWindow.h"
#include "scene/ConnectionLine.h"


MainWindow:: MainWindow(QWidget* parent): QMainWindow(parent){
    setWindowTitle("Графическое приложения для создание прмиитивов");
    resize(1250, 720);

    if(!m_db.initialize("primitives.db")){
        QMessageBox::critical(this, "Ошибка", "Не удалось подключится к базе данных" );
        return;
    }

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    //разделяю экран на пополам: слева сцена с графикой, справа фильтры
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    //Левая часть: графическая сцена + панель создания фигур
    auto* leftPanel = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftPanel);

    auto* createGroup = new QGroupBox("Добавление примитива", this);
    auto* createLayout = new QHBoxLayout(createGroup);

    m_createTypeCombo = new QComboBox(this);
    m_createTypeCombo->addItem(DatabaseManager::shapeTypeToString(ShapeType::Ellipse), static_cast<int>(ShapeType::Ellipse));
    m_createTypeCombo->addItem(DatabaseManager::shapeTypeToString(ShapeType::Rectangle), static_cast<int>(ShapeType::Rectangle));
    m_createTypeCombo->addItem(DatabaseManager::shapeTypeToString(ShapeType::Polygon), static_cast<int>(ShapeType::Polygon));

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(30, 300);
    m_widthSpin->setValue(90);

    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(30, 300);
    m_heightSpin->setValue(60);

    m_sidesSpin = new QSpinBox(this);
    m_sidesSpin->setRange(3, 20);
    m_sidesSpin->setValue(5);
    m_sidesSpin->setEnabled(false);

    m_addBtn = new QPushButton("Добавить на сцену", this);

    createLayout->addWidget(new QLabel("Форма:", this));
    createLayout->addWidget(m_createTypeCombo);
    createLayout->addWidget(new QLabel("Ширина:", this));
    createLayout->addWidget(m_widthSpin);
    createLayout->addWidget(new QLabel("Высота:", this));
    createLayout->addWidget(m_heightSpin);
    createLayout->addWidget(new QLabel("Вершин:", this));
    createLayout->addWidget(m_sidesSpin);
    createLayout->addWidget(m_addBtn);

    leftLayout->addWidget(createGroup);

    // Панель режима сцены
    auto* modeGroup = new QGroupBox("Режим работы со сценой", this);
    auto* modeLayout = new QHBoxLayout(modeGroup);

    m_moveModeRadio = new QRadioButton("Перемещение фигур", this);
    m_connectModeRadio = new QRadioButton("Создание связей (потяните линию от фигуры к фигуре)", this);
    m_moveModeRadio->setChecked(true);

    modeLayout->addWidget(m_moveModeRadio);
    modeLayout->addWidget(m_connectModeRadio);
    modeLayout->addStretch();

    leftLayout->addWidget(modeGroup);

    // Сама графическая сцена
    m_scene = new GraphicsScene(this);
    m_graphicsView = new QGraphicsView(m_scene, this);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    leftLayout->addWidget(m_graphicsView);

    splitter->addWidget(leftPanel);



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
    m_deleteBtn = new QPushButton("Удалить выбранную фигуру (и её связи)", this);

    actionsLayout->addStretch();
    actionsLayout->addWidget(m_deleteBtn);
    rightLayout->addLayout(actionsLayout);


    //Сигналы и слоты
    connect(m_createTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onShapeSelectionChanged);
    connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddPrimitiveClicked);

    connect(m_moveModeRadio, &QRadioButton::toggled, this, &MainWindow::onModeChanged);
    connect(m_connectModeRadio, &QRadioButton::toggled, this, &MainWindow::onModeChanged);
    connect(m_scene, &GraphicsScene::connectionCreated,
            this, &MainWindow::onConnectionCreated);
    connect(m_filterIdEdit, &QLineEdit::textChanged, this, &MainWindow::applyFilters);
    connect(m_filterTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::applyFilters);
    connect(m_filterCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::applyFilters);
    connect(m_resetFilterBtn, &QPushButton::clicked, this, &MainWindow::resetFilters);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSelectedClicked);




    //загрузка данных из бд в таблицу
    m_model = new QSqlTableModel(this, m_db.getDatabase());
    m_model->setTable("primitives");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_model->setHeaderData(0, Qt::Horizontal, "ID");
    m_model->setHeaderData(1, Qt::Horizontal, "Тип фигуры");
    m_model->setHeaderData(2, Qt::Horizontal, "Связи (ID)");
    m_model->setHeaderData(3, Qt::Horizontal, "Кол-во типа");

    m_model->select();
    m_tableView->setModel(m_model);
    m_tableView->setItemDelegateForColumn(3, new ShapeCountDelegate(m_scene, this));
    m_tableView->resizeColumnsToContents();



    // Восстанавливаем фигуры и связи из БД
    const QList<PrimitiveGeometry> saved = m_db.loadAllPrimitives();
    for (const auto& g : saved) {
        m_scene->addPrimitiveItem(g.id, g.type, g.width, g.height, QPointF(g.x, g.y), g.sides);
    }
    for (const auto& g : saved) {
        PrimitiveItem* start = m_scene->findPrimitiveById(g.id);
        for (int targetId : g.connections) {
            if (g.id < targetId && !m_scene->hasConnection(g.id, targetId)) {
                PrimitiveItem* end = m_scene->findPrimitiveById(targetId);
                if (start && end) {
                    m_scene->addItem(new ConnectionLine(start, end));
                }
            }
        }
    }
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
int idToDelete = -1;

    const QModelIndex current = m_tableView->currentIndex();
    if (current.isValid()) {
        idToDelete = m_model->data(m_model->index(current.row(), 0)).toInt();
    } else if (!m_scene->selectedItems().isEmpty()) {
        if (PrimitiveItem* prim = dynamic_cast<PrimitiveItem*>(m_scene->selectedItems().first())) {
            idToDelete = prim->id();
        }
    }

    if (idToDelete <= 0) {
        QMessageBox::information(this, "Удаление", "Выберите фигуру в таблице (или на сцене) для удаления.");
        return;
    }

    if (m_db.removePrimitive(idToDelete)) {
        m_scene->removePrimitiveById(idToDelete);
        m_model->select();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить фигуру из базы данных.");
    }
}



void MainWindow::onShapeSelectionChanged(int index)
{
    Q_UNUSED(index);
    ShapeType type = static_cast<ShapeType>(m_createTypeCombo->currentData().toInt());
    m_sidesSpin->setEnabled(type == ShapeType::Polygon);
}

void MainWindow::onModeChanged()
{
    if (m_connectModeRadio->isChecked()) {
        m_scene->setMode(GraphicsScene::Mode::Connect);
    } else {
        m_scene->setMode(GraphicsScene::Mode::Move);
    }
}

void MainWindow::onAddPrimitiveClicked()
{
    ShapeType type = static_cast<ShapeType>(m_createTypeCombo->currentData().toInt());
    int width = m_widthSpin->value();
    int height = m_heightSpin->value();
    int sides = m_sidesSpin->value();

    int newId = m_db.addPrimitive(type);
    if (newId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить фигуру в БД.");
        return;
    }

    qreal x = QRandomGenerator::global()->bounded(80, 520);
    qreal y = QRandomGenerator::global()->bounded(80, 440);

    m_scene->addPrimitiveItem(newId, type, width, height, QPointF(x, y), sides);
    m_db.saveGeometry(newId, x, y, width, height, sides);
    m_model->select();
}

void MainWindow::onConnectionCreated(int id1, int id2)
{
    if (m_db.addConnection(id1, id2)) {
        m_model->select();
    }
}