#include "directormainwindow.h"
#include "ui_directormainwindow.h"
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QMessageBox>
#include <QSqlQuery>

using namespace std;

DirectorMainWindow::DirectorMainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::DirectorMainWindow), _checkExistConnectionsTimer(this) {
    ui->setupUi(this);
    Init();
    SetupUI();
}

void DirectorMainWindow::Init() {
    _contractFilterModel.setSourceModel(&_sqlContractModel);
    _supplierFilterModel.setSourceModel(&_sqlSupplierModel);
    _orderFilterModel.setSourceModel(&_sqlOrderModel);
    _stockFilterModel.setSourceModel(&_sqlStockModel);

    InitConnects();
}

void DirectorMainWindow::InitConnects() {
    connect(ui->contractsTable, &QTableView::doubleClicked, this, &DirectorMainWindow::OnContractsDoubleClicked);
    connect(ui->suppliersTable, &QTableView::doubleClicked, this, &DirectorMainWindow::OnSuppliersDoubleClicked);
    connect(ui->ordersTable, &QTableView::doubleClicked, this, &DirectorMainWindow::OnOrdersDoubleClicked);

    connect(ui->supplierGoodCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        _supplierFilterModel.SetCategoryFilter(ui->supplierGoodCategory->currentText());
        ui->suppliersTable->selectRow(_supplierFilterModel.rowCount() - 1);
        ui->suppliersTable->resizeRowsToContents();
    });

    connect(ui->stockGoodCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        _stockFilterModel.SetCategoryFilter(ui->stockGoodCategory->currentText());
    });

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [=](int newTab) {
        if (_lastTab == 5) // Если сменили вкладку "магазина "О магазине" на другую, то если данные не сохранили, то таблица восстановит
            _sqlStoreModel.revertAll(); // То, что было до ввода пользователем. Если данные сохранили, ничего не произойдёт
        _lastTab = newTab;
    });

    _checkExistConnectionsTimer.start(500);
    connect(&_checkExistConnectionsTimer, &QTimer::timeout, this, [=]() {
        QList<QList<QMetaObject::Connection>::iterator> removableIterators;
        for (int i = 0; i < _danglingConnections.size(); i++) {
            if (!_danglingConnections[i]) {         // проверяем по таймингу "висящие" коннекты у закрытых окон и закрываем их, удаляя из списка
                disconnect(_danglingConnections[i]);
                removableIterators.push_back(_danglingConnections.begin() + i);
            }
        }
        for (auto& iterator : removableIterators) // Опасно удалять из списка во время прохождения по нему (неопределённое поведение), поэтому так
            _danglingConnections.erase(iterator);
    });

    QVector<QCheckBox*> contractTypes = {ui->expiring, ui->acting, ui->archival};
    for (const auto& type : contractTypes) {
        connect(type, &QCheckBox::stateChanged, this, [=]() {
            if (type == ui->expiring)
                _contractFilterModel.SetUseExpiringFilter(type->isChecked());
            if (type == ui->acting)
                _contractFilterModel.SetUseActingFilter(type->isChecked());
            if (type == ui->archival)
                _contractFilterModel.SetUseArchivalFilter(type->isChecked());
            ui->contractsTable->resizeRowsToContents(); // При фильтрации без явного ресайза строки иногда меньше по высоте, чем должны быть
        });
    }

    QVector<QCheckBox*> orderTypes = {ui->paid, ui->delivered};
    for (const auto& type : orderTypes) {
        connect(type, &QCheckBox::stateChanged, this, [=]() {
            if (type == ui->paid)
                _orderFilterModel.SetUsePaidFilter(type->isChecked());
            if (type == ui->delivered)
                _orderFilterModel.SetUseDeliveredFilter(type->isChecked());
            ui->ordersTable->resizeRowsToContents(); // При фильтрации без явного ресайза строки иногда меньше по высоте, чем должны быть
        });
    }
}

void DirectorMainWindow::SetupUI() {
    SetupTables();

    ui->contractsTable->horizontalHeader()->resizeSection(0, 75);
    ui->contractsTable->horizontalHeader()->resizeSection(1, 180);
    ui->contractsTable->horizontalHeader()->resizeSection(2, 150);
    ui->contractsVerticalLayout->setAlignment(ui->contractTypes, Qt::AlignCenter);
    ui->contractsVerticalLayout->setAlignment(ui->addContract, Qt::AlignCenter);

    ui->suppliersTable->horizontalHeader()->resizeSection(0, 130);
    ui->suppliersTable->horizontalHeader()->resizeSection(1, 130);
    ui->suppliersTable->horizontalHeader()->resizeSection(2, 150);
    ui->suppliersTable->horizontalHeader()->resizeSection(3, 130);
    ui->suppliersVerticalLayout->setAlignment(ui->supplierGoodCategoryGroup, Qt::AlignCenter);
    ui->suppliersVerticalLayout->setAlignment(ui->addSupplier, Qt::AlignCenter);

    ui->ordersTable->horizontalHeader()->resizeSection(0, 60);
    ui->ordersTable->horizontalHeader()->resizeSection(1, 140);
    ui->ordersTable->horizontalHeader()->resizeSection(2, 85);
    ui->ordersTable->horizontalHeader()->resizeSection(3, 100);
    ui->ordersTable->horizontalHeader()->resizeSection(4, 80);
    ui->ordersVerticalLayout->setAlignment(ui->orderTypes, Qt::AlignCenter);
    ui->ordersVerticalLayout->setAlignment(ui->addOrder, Qt::AlignCenter);

    ui->stockTable->horizontalHeader()->resizeSection(2, 250);
    ui->stockTable->horizontalHeader()->resizeSection(3, 100);

    ui->stockVerticalLayout->setAlignment(ui->stockGoodCategoryGroup, Qt::AlignCenter);
    ui->stockVerticalLayout->setAlignment(ui->stockPrint, Qt::AlignCenter);

    ui->analysisVerticalLayout->setAlignment(ui->reportTypeGroup, Qt::AlignCenter);
    ui->analysisVerticalLayout->setAlignment(ui->analysisOk, Qt::AlignCenter);

    ui->store->layout()->setAlignment(ui->saveStoreInfo, Qt::AlignCenter);

    ui->expiring->setCheckState(Qt::Unchecked);
    ui->acting->setCheckState(Qt::Checked);
    ui->archival->setCheckState(Qt::Unchecked);
}

void DirectorMainWindow::SetupTables() {
    SetupContracts();
    SetupSuppliers();
    SetupOrders();
    SetupStock();
    SetupStore();
}

void DirectorMainWindow::SetupContracts() {
    _sqlContractModel.setQuery("SELECT num, name, conclusion_date, completion_date "
                               "FROM contracts JOIN suppliers "
                               "ON contracts.supplier_code = suppliers.code "
                               "ORDER BY substring(num, '[0-9]+')::int");
    _sqlContractModel.setHeaderData(0, Qt::Horizontal, "№");
    _sqlContractModel.setHeaderData(1, Qt::Horizontal, "Поставщик");
    _sqlContractModel.setHeaderData(2, Qt::Horizontal, "Дата заключения");
    _sqlContractModel.setHeaderData(3, Qt::Horizontal, "Дата окончания");

    ui->contractsTable->setModel(&_contractFilterModel);
    QFont font = ui->contractsTable->horizontalHeader()->font();
    font.setBold(true);
    ui->contractsTable->horizontalHeader()->setFont(font);
    ui->contractsTable->setWordWrap(true);
    ui->contractsTable->resizeRowsToContents();
    connect(ui->contractsTable->horizontalHeader(), &QHeaderView::sectionResized, ui->contractsTable, &QTableView::resizeRowsToContents);
    connect(&_sqlContractModel, &QAbstractTableModel::dataChanged, ui->contractsTable, &QTableView::resizeRowsToContents);
    ui->contractsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
    ui->contractsTable->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки
    ui->contractsTable->horizontalHeader()->setFixedHeight(50);
}

void DirectorMainWindow::SetupSuppliers() {
    _sqlSupplierModel.setQuery("SELECT s.code, s.name, STRING_AGG (distinct c.name, '; ') goods_list, s.director, s.phone "
                               "FROM suppliers AS s JOIN catalog_sup AS c ON c.supplier_code = s.code "
                               "GROUP BY c.supplier_code, s.name, s.code, s.director, s.phone "
                               "ORDER BY substring(s.code, '[0-9]+')::int");
    _sqlSupplierModel.setHeaderData(1, Qt::Horizontal, "Название");
    _sqlSupplierModel.setHeaderData(2, Qt::Horizontal, "Категории товаров");
    _sqlSupplierModel.setHeaderData(3, Qt::Horizontal, "Директор");
    _sqlSupplierModel.setHeaderData(4, Qt::Horizontal, "Телефон");

    _supplierFilterModel.setSourceModel(&_sqlSupplierModel);
    ui->suppliersTable->setModel(&_supplierFilterModel);
    QFont font = ui->suppliersTable->horizontalHeader()->font();
    font.setBold(true);
    ui->suppliersTable->horizontalHeader()->setFont(font);
    ui->suppliersTable->hideColumn(0);
    ui->suppliersTable->setWordWrap(true);
    ui->suppliersTable->resizeRowsToContents();

    connect(ui->suppliersTable->horizontalHeader(), &QHeaderView::sectionResized, ui->suppliersTable, &QTableView::resizeRowsToContents);
    connect(&_sqlSupplierModel, &QAbstractTableModel::dataChanged, ui->suppliersTable, &QTableView::resizeRowsToContents);
    ui->suppliersTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
    ui->suppliersTable->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки :/
    ui->suppliersTable->horizontalHeader()->setFixedHeight(50);
}

void DirectorMainWindow::SetupOrders() {
    _sqlOrderModel.setQuery("SELECT o.num, s.name, o.contract_num, o.registration_date, o.paid, o.sent, s.code "
                            "FROM orders_sup AS o "
                            "JOIN contracts AS c ON c.num = o.contract_num "
                            "JOIN suppliers AS s ON s.code = c.supplier_code "
                            "WHERE c.completion_date >= NOW() "
                            "ORDER BY substring(o.num, '[0-9]+')::int");
    _sqlOrderModel.setHeaderData(0, Qt::Horizontal, "№");
    _sqlOrderModel.setHeaderData(1, Qt::Horizontal, "Поставщик");
    _sqlOrderModel.setHeaderData(2, Qt::Horizontal, "№ Договора");
    _sqlOrderModel.setHeaderData(3, Qt::Horizontal, "Дата создания");
    _sqlOrderModel.setHeaderData(4, Qt::Horizontal, "Оплачен");
    _sqlOrderModel.setHeaderData(5, Qt::Horizontal, "Доставлен");

    ui->ordersTable->setModel(&_orderFilterModel);
    ui->ordersTable->setItemDelegateForColumn(4, &_sqlOrderBooleanDelegate);
    ui->ordersTable->setItemDelegateForColumn(5, &_sqlOrderBooleanDelegate);
    ui->ordersTable->hideColumn(6);
    QFont font = ui->ordersTable->horizontalHeader()->font();
    font.setBold(true);
    ui->ordersTable->horizontalHeader()->setFont(font);
    ui->ordersTable->setWordWrap(true);
    ui->ordersTable->resizeRowsToContents();

    connect(ui->ordersTable->horizontalHeader(), &QHeaderView::sectionResized, ui->ordersTable, &QTableView::resizeRowsToContents);
    connect(&_sqlOrderModel, &QAbstractTableModel::dataChanged, ui->ordersTable, &QTableView::resizeRowsToContents);
    ui->ordersTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
    ui->ordersTable->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки :/
    ui->ordersTable->horizontalHeader()->setFixedHeight(50);
}

void DirectorMainWindow::SetupStock() {
    _sqlStockModel.setQuery("SELECT cat.name, st.code, cat.description, st.price, st.number "
                            "FROM stock AS st "
                            "JOIN catalog_sup AS cat ON st.code = cat.code "
                            "WHERE st.number > 0 "
                            "ORDER BY st.number DESC");

    _sqlStockModel.setHeaderData(1, Qt::Horizontal, "Код товара");
    _sqlStockModel.setHeaderData(2, Qt::Horizontal, "Наименование");
    _sqlStockModel.setHeaderData(3, Qt::Horizontal, "Цена");
    _sqlStockModel.setHeaderData(4, Qt::Horizontal, "Количество");

    ui->stockTable->setModel(&_stockFilterModel);
    QFont font = ui->stockTable->horizontalHeader()->font();
    font.setBold(true);
    ui->stockTable->horizontalHeader()->setFont(font);
    ui->stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers | QAbstractItemView::DoubleClicked);
    ui->stockTable->hideColumn(0);
    ui->stockTable->setItemDelegateForColumn(3, &_stockCostDelegate);

    connect(ui->stockTable->horizontalHeader(), &QHeaderView::sectionResized, ui->stockTable, &QTableView::resizeRowsToContents);
    ui->stockTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
    ui->stockTable->resizeColumnsToContents();
    ui->stockTable->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки :/
    ui->stockTable->horizontalHeader()->setFixedHeight(50);
}

void DirectorMainWindow::SetupStore() {
    _sqlStoreModel.setTable("stores");
    _sqlStoreModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    _sqlStoreModel.select();
    _sqlStoreModel.setHeaderData(1, Qt::Horizontal, "Название");
    _sqlStoreModel.setHeaderData(2, Qt::Horizontal, "Адрес");
    _sqlStoreModel.setHeaderData(3, Qt::Horizontal, "Телефон");
    _sqlStoreModel.setHeaderData(4, Qt::Horizontal, "Факс");
    _sqlStoreModel.setHeaderData(5, Qt::Horizontal, "Директор");
    _sqlStoreModel.setHeaderData(6, Qt::Horizontal, "Бухгалтер");
    _sqlStoreModel.setHeaderData(7, Qt::Horizontal, "Расчетный счет");
    _sqlStoreModel.setHeaderData(8, Qt::Horizontal, "ИНН");

    _sqlStoreTransposeModel.setSourceModel(&_sqlStoreModel);

    ui->storeTable->setModel(&_sqlStoreTransposeModel);
    ui->storeTable->hideRow(0); // Код магазина
    ui->storeTable->selectRow(1);
    ui->storeTable->setItemDelegateForRow(3, &_storePhoneDelegate); // Телефон
    ui->storeTable->setItemDelegateForRow(4, &_storePhoneDelegate); // Факс
    ui->storeTable->setItemDelegateForRow(8, &_storeInnDelegate); // ИНН
    QFont font = ui->storeTable->verticalHeader()->font();
    font.setBold(true);
    ui->storeTable->verticalHeader()->setFont(font);
}

void DirectorMainWindow::on_addContract_clicked() {
    if (_sqlSupplierModel.rowCount() == 0) {
        QMessageBox::critical(this, "Ошибка создания договора", "Нужен хотя бы один поставщик для заключения договора");
        return;
    }

    int contractNumber = 1;
    if (_sqlContractModel.rowCount() > 0)
        contractNumber = _sqlContractModel.index(_sqlContractModel.rowCount() - 1, 0).data().toString().replace("ДОГ-", "").toInt() + 1;

   AddContractWindow* acw = new AddContractWindow(contractNumber);
    ui->mdiArea->addSubWindow(acw);
    ui->addContract->setEnabled(false);
    acw->open();
    _danglingConnections.push_back(
        connect(acw, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            _sqlContractModel.setQuery(_sqlContractModel.query().lastQuery());
            ui->contractsTable->scrollToBottom();
            ui->contractsTable->resizeRowsToContents();
            ui->contractsTable->selectRow(_contractFilterModel.rowCount() - 1);
        }
        ui->addContract->setEnabled(true);
    }));
}

void DirectorMainWindow::OnContractsDoubleClicked(const QModelIndex& index) {
    ViewContractWindow* vcw = new ViewContractWindow(_contractFilterModel.data(_contractFilterModel.index(index.row(), 0)).toString());
    ui->mdiArea->addSubWindow(vcw);
    vcw->exec();
}

void DirectorMainWindow::on_addSupplier_clicked() {
    SupplierWindow* sw = new SupplierWindow;
    ui->mdiArea->addSubWindow(sw);
    ui->addSupplier->setEnabled(false);
    sw->open();
    _danglingConnections.push_back(
        connect(sw, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            _sqlSupplierModel.setQuery(_sqlSupplierModel.query().lastQuery());
            ui->suppliersTable->scrollToBottom();
            ui->suppliersTable->resizeRowsToContents();
            ui->suppliersTable->selectRow(_supplierFilterModel.rowCount() - 1);
        }
        ui->addSupplier->setEnabled(true);
    }));
}

void DirectorMainWindow::OnSuppliersDoubleClicked(const QModelIndex& index) {
    SupplierWindow* sw = new SupplierWindow(_supplierFilterModel.data(_supplierFilterModel.index(index.row(), 0)).toString());
    ui->mdiArea->addSubWindow(sw);
    sw->open();
    _danglingConnections.push_back(
        connect(sw, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            _sqlSupplierModel.setQuery(_sqlSupplierModel.query().lastQuery());
            _sqlContractModel.setQuery(_sqlContractModel.query().lastQuery()); // Чтоб обновился поставщик в договорах
            _sqlOrderModel.setQuery(_sqlOrderModel.query().lastQuery());       // Чтоб обновился поставщик в заказах
            ui->suppliersTable->scrollTo(_supplierFilterModel.index(index.row(), 0));
            ui->suppliersTable->resizeRowsToContents();
            ui->suppliersTable->selectRow(index.row());
        }
    }));
}

void DirectorMainWindow::on_addOrder_clicked() {
    if (_sqlSupplierModel.rowCount() == 0) {
        QMessageBox::critical(this, "Ошибка добавления заказа", "Нужен хотя бы один поставщик для добавления заказа");
        return;
    }

    int orderNumber = 1;
    if (_sqlOrderModel.rowCount() > 0)
        orderNumber = _sqlOrderModel.index(_sqlOrderModel.rowCount() - 1, 0).data().toString().replace("ЗАК-", "").toInt() + 1;

    AddOrderWindow* aow = new AddOrderWindow(orderNumber, ui->mdiArea, ui->mdiArea);
    ui->mdiArea->addSubWindow(aow);
    ui->addOrder->setEnabled(false);
    aow->open();
    _danglingConnections.push_back(
        connect(aow, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            _sqlOrderModel.setQuery(_sqlOrderModel.query().lastQuery());
            ui->ordersTable->scrollToBottom();
            ui->ordersTable->resizeRowsToContents();
            ui->ordersTable->selectRow(_orderFilterModel.rowCount() - 1);
        }
        ui->addOrder->setEnabled(true);
    }));
}

void DirectorMainWindow::OnOrdersDoubleClicked(const QModelIndex& index) {
    const ViewOrderInfo orderInfo = {
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 1)).toString(),
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 2)).toString(),
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 3)).toString(),
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 4)).toString(),
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 5)).toString(),
        _orderFilterModel.data(_orderFilterModel.index(index.row(), 6)).toString()
    };

    ViewOrderWindow* vow = new ViewOrderWindow(_orderFilterModel.data(_orderFilterModel.index(index.row(), 0)).toString(), orderInfo);
    ui->mdiArea->addSubWindow(vow);
    vow->open();
    _danglingConnections.push_back(
        connect(vow, &QDialog::finished, this, [=](int result) {
        if (result == QDialog::Accepted) {
            _sqlOrderModel.setQuery(_sqlOrderModel.query().lastQuery());
            _sqlStockModel.setQuery(_sqlStockModel.query().lastQuery()); // Обновление склада с товарами
        }
    }));
}

void DirectorMainWindow::on_analysisOk_clicked() {
    if(ui->periodFrom->date() > ui->periodTo->date()) {
        QMessageBox::critical(nullptr, "Ошибка создания отчета", "Некорректно указаны сроки составления отчета."
                    "\nУкажите дату начала периода не позднее даты окончания");
        return;
    }

    ReportInfo info = {ui->reportCategory->currentText(), ui->periodFrom->text(), ui->periodTo->text()};
    ReportWindow* rw = new ReportWindow(info);
    ui->mdiArea->addSubWindow(rw);
    ui->analysisOk->setEnabled(false);
    rw->open();
    connect(rw, &QDialog::finished, this, [=]() {
        ui->analysisOk->setEnabled(true);
    });
}

void DirectorMainWindow::on_stockPrint_clicked() {
    QPrinter printer;
    QPrintPreviewDialog printWindow(&printer, nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    printWindow.setWindowTitle("Просмотр печати товаров на складе");
    printWindow.resize(1000, 600);
    connect(&printWindow, &QPrintPreviewDialog::paintRequested, this, &DirectorMainWindow::PaintStockPages);
    printWindow.exec();
}

void DirectorMainWindow::PaintStockPages(QPrinter* printer) {
    QPainter painter;
    painter.begin(printer);

    int pageWidth = printer->pageLayout().paintRectPixels(printer->resolution()).width() - 20;
    int pageHeight = printer->pageLayout().paintRectPixels(printer->resolution()).height() - 20;
    QRect rect (20, 20, pageWidth, 0.1 * pageHeight);

    PaintStockHeader(painter);

    QPen pen = painter.pen();
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);

    QSqlQuery query("SELECT cat.name, cat.description, st.code, st.number, st.price "
                    "FROM stock AS st "
                    "JOIN catalog_sup AS cat ON cat.code = st.code "
                    "WHERE st.number > 0 order by cat.name");

    int top = 0.1 * pageHeight;
    QString prevType;

    while (query.next()) {
        if(prevType != query.value(0).toString())
            top += 0.02 * pageHeight;
        prevType = query.value(0).toString();

        top += 0.04 * pageHeight;

        if(top > pageHeight ){
            printer->newPage();
            PaintStockHeader(painter);
            top = 0.16 * pageHeight;
        }

        rect.setTop(top);
        rect.setHeight(0.04 * pageHeight);
        rect.setLeft(20);
        rect.setWidth(150);
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, query.value(0).toString());

        rect.setLeft(170);
        rect.setWidth(350);
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, query.value(1).toString());

        rect.setLeft(520);
        rect.setWidth(80);
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, query.value(2).toString());

        rect.setLeft(600);
        rect.setWidth(50);
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, query.value(3).toString());

        rect.setLeft(650);
        rect.setWidth(90);
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, query.value(4).toString());
    }
}

void DirectorMainWindow::PaintStockHeader(QPainter& painter) {
    int pageWidth = 727 - 20;
    int pageHeight = 1071 - 20;
    QRect rect (20, 20, pageWidth, 0.1 * pageHeight);

    QFont font = painter.font();
    font.setPixelSize(22);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, "Список товаров на складе - " + QDate::currentDate().toString("dd.MM.yyyy"));

    font.setPixelSize(15);
    font.setBold(true);
    painter.setFont(font);

    rect.setTop(0.12 * pageHeight);
    rect.setHeight(0.04 * pageHeight);

    rect.setLeft(20);
    rect.setWidth(150);
    painter.drawText(rect, Qt::AlignCenter, "Категория");

    rect.setLeft(170);
    rect.setWidth(350);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, "Наименование");

    rect.setLeft(520);
    rect.setWidth(80);
    painter.drawText(rect, Qt::AlignCenter, "Код");

    rect.setLeft(600);
    rect.setWidth(50);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, "Кол-во");

    rect.setLeft(650);
    rect.setWidth(90);
    painter.drawText(rect, Qt::AlignCenter, "Цена");

    font.setBold(false);
    painter.setFont(font);
}

void DirectorMainWindow::UpdateStock() {
    _sqlStockModel.setQuery(_sqlStockModel.query().lastQuery());
    ui->stockGoodCategory->setCurrentIndex(0);
}

DirectorMainWindow::~DirectorMainWindow() {
    for (auto& connection : _danglingConnections)
        disconnect(connection);

    delete ui;
}

void DirectorMainWindow::on_saveStoreInfo_clicked() {
    for (int i = 1; i < _sqlStoreTransposeModel.rowCount(); i++) { // С 1-го, потому что не нужен код магазина
        if (_sqlStoreTransposeModel.index(i, 0).data().toString().isEmpty()) {
            ui->storeTable->selectRow(i);
            QMessageBox::critical(nullptr, "Ошибка сохранения информации о магазине", "Все поля информации о магазине должны быть заполнены");
            ui->storeTable->setFocus();
            return;
        }

        if (i == 3 || i == 4) {
            if (!PhoneLineEdit::IsValid(_sqlStoreTransposeModel.index(i, 0).data().toString())) {
                ui->storeTable->selectRow(i);
                QMessageBox::critical(nullptr, "Ошибка сохранения информации о магазине", "Неверно введён номер телефона");
                ui->storeTable->setFocus();
                return;
            }
        }

        if (i == 8) {
            if (_sqlStoreTransposeModel.index(i, 0).data().toString().count() < 10) {
                ui->storeTable->selectRow(i);
                QMessageBox::critical(nullptr, "Ошибка сохранения информации о магазине", "ИНН должен содержать 10 цифр");
                ui->storeTable->setFocus();
                return;
            }
        }
    }

    _sqlStoreModel.submitAll();
}
