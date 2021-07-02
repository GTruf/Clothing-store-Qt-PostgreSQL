#include "addorderwindow.h"
#include "ui_addorderwindow.h"
#include <QMessageBox>
#include <QSqlQuery>

using namespace std;

AddOrderWindow::AddOrderWindow(int orderNumber, QMdiArea* mdiArea, QWidget* parent) : QDialog(parent), ui(new Ui::AddOrderWindow),
    _mdiArea(mdiArea), _quantityValidator(QRegularExpression("0|[1-9]\\d{2}")), _numExpression("(^(0|[1-9][0-9]{0,9})(,[0-9]{0,2})?)?")
{
    ui->setupUi(this);
    Init();
    SetupUI();
    ui->orderNumber->setText(QString::number(orderNumber));
}

void AddOrderWindow::Init() {
    _comboSupplierModel.setQuery("SELECT DISTINCT ON (name) name, code FROM suppliers "
                                 "WHERE EXISTS (SELECT num FROM contracts WHERE completion_date >= NOW() "
                                 "AND contracts.supplier_code = suppliers.code)");
    ui->suppliers->setModel(&_comboSupplierModel);

    // При создании окна чтобы сразу прогрузились категории для первого в списке поставщика
    _supplierCode = "'" + _comboSupplierModel.index(0, 1).data().toString() + "'";
    _comboSupplyCategoriesModel.setQuery("SELECT DISTINCT name FROM catalog_sup WHERE supplier_code = " + _supplierCode);
    ui->supplyCategories->setModel(&_comboSupplyCategoriesModel);

    _sqlCatalogModel.setTable("catalog_sup");
    _sqlCatalogModel.setFilter("supplier_code = " + _supplierCode);
    _sqlCatalogModel.select();
    _sqlCatalogModel.setHeaderData(0, Qt::Horizontal, "Код товара");
    _sqlCatalogModel.setHeaderData(2, Qt::Horizontal, "Наименование");
    _sqlCatalogModel.setHeaderData(3, Qt::Horizontal, "Цена");

    _catalogFilterModel.setSourceModel(&_sqlCatalogModel);
    _catalogFilterModel.SetCategoryFilter(ui->supplyCategories->currentText());

    ui->supplyGoods->setModel(&_catalogFilterModel);
    ui->supplyGoods->hideColumn(1); // Категория
    ui->supplyGoods->hideColumn(4); // Код поставщика
    ui->supplyGoods->selectRow(0);
    QFont font = ui->supplyGoods->horizontalHeader()->font();
    font.setBold(true);
    ui->supplyGoods->horizontalHeader()->setFont(font);

    ui->goodsInOrder->setModel(&_goodsInOrderModel);
    ui->goodsInOrder->setItemDelegateForColumn(3, &_orderCostDelegate);

    InitConnects();
}

void AddOrderWindow::InitConnects() {
    connect(ui->suppliers, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (_isNeedToSupplierWarning) {
            auto result = QMessageBox::warning(nullptr, "Предупреждение при смене поставщика",
                                "Заказать товар можно только у одного поставщика. При смене поставщика корзина будет опустошена. Продолжить?",
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (result == QMessageBox::No) {
                _isNeedToSupplierWarning = false; // Отменяем выбор поставщика, заново предупреждение выводить не нужно
                ui->suppliers->setCurrentIndex(_lastSupplierIndex);
            }
            else {
                _goodsInOrderModel.removeRows(0, _goodsInOrderModel.rowCount());
                _totalCost = 0;
                ui->totalCost->setText("Общая стоимость: 0 рублей");
                _isNeedToSupplierWarning = true;
                _lastSupplierIndex = index;

                _supplierCode = "'" + _comboSupplierModel.index(index, 1).data().toString() + "'";
                _sqlCatalogModel.setFilter("supplier_code = " + _supplierCode);
                _catalogFilterModel.SetCategoryFilter(ui->supplyCategories->currentText());
                _comboSupplyCategoriesModel.setQuery("SELECT DISTINCT name FROM catalog_sup WHERE supplier_code = " + _supplierCode);
                ui->removeFromOrder->setEnabled(false);
            }
        }
        else {
            _isNeedToSupplierWarning = true;
        }
    });

    connect(ui->supplyCategories, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int) {
        _catalogFilterModel.SetCategoryFilter(ui->supplyCategories->currentText());
        ui->supplyGoods->selectRow(0);
    });
}

void AddOrderWindow::SetupUI() {
    ui->quantity->setValidator(&_quantityValidator);

    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->orderNumberGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->addToOrder, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->totalCost, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->getInvoice, Qt::AlignCenter);

    SetupTables();

    ui->supplyGoods->horizontalHeader()->resizeSection(2, 210);
    ui->supplyGoods->horizontalHeader()->resizeSection(0, 70);

    ui->goodsInOrder->horizontalHeader()->resizeSection(0, 150);
}

void AddOrderWindow::SetupTables() {
    QVector<QTableView*> tables = {ui->supplyGoods, ui->goodsInOrder};
    for (const auto& table : tables) {
        table->setWordWrap(true);
        table->resizeColumnsToContents();
        connect(table->horizontalHeader(), &QHeaderView::sectionResized, table, &QTableView::resizeRowsToContents);
        if (table->horizontalHeader()->isHidden())
            table->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки :/
    }
}

void AddOrderWindow::on_addToOrder_clicked() {
    if (ui->quantity->text().isEmpty() || ui->quantity->text() == "0") {
        QMessageBox::critical(nullptr, "Ошибка добавления товара в заказ", "Количество товара должно быть не меньше одного");
        ui->quantity->setFocus();
        ui->quantity->selectAll();
        return;
    }

    const auto supplyGoodIndex = ui->supplyGoods->currentIndex();

    const auto quantity = ui->quantity->text().toInt();
    const auto goodCode = _catalogFilterModel.index(supplyGoodIndex.row(), 0).data().toString();
    const auto supplyGoodDescription = _catalogFilterModel.index(supplyGoodIndex.row(), 2).data().toString();
    const auto goodPriceStr = _catalogFilterModel.index(supplyGoodIndex.row(), 3).data().toString().simplified().remove(' ');
    const auto totalGoodPrice = _numExpression.match(goodPriceStr).captured().replace(',', '.').toDouble() * quantity;

    _totalCost += totalGoodPrice;
    ui->totalCost->setText("Общая стоимость: " + QString::number(_totalCost, 'f', 2).append(" рублей"));

    auto row = _goodsInOrderModel.FindAndUpdateGoodInfo(goodCode, quantity, totalGoodPrice);
    if (row >= 0) { // Если товар уже добавлен, к нему просто прибавится количество и цена
        ui->goodsInOrder->update(_goodsInOrderModel.index(row, 2));
        ui->goodsInOrder->update(_goodsInOrderModel.index(row, 3));
        ui->goodsInOrder->selectRow(row);
    }
    else {  // Добавление товара в заказ
        row = _goodsInOrderModel.rowCount();
        _goodsInOrderModel.insertRows(row, 1);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 0), supplyGoodDescription);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 1), goodCode);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 2), quantity);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 3), totalGoodPrice);
        ui->goodsInOrder->selectRow(_goodsInOrderModel.rowCount() - 1);
    }

    ui->removeFromOrder->setEnabled(true);
}

bool AddOrderWindow::ErrorCheck() const {
    if (_goodsInOrderModel.rowCount() == 0) {
        QMessageBox::critical(nullptr, "Ошибка создания заказа", "Корзина заказа не может быть пустой.\nДобавьте товар(ы) в заказ");
        return false;
    }

    return true;
}

void AddOrderWindow::on_getInvoice_clicked() {
    if (!ErrorCheck())
        return;

    parentWidget()->hide(); // Скрываем данное окно, чтоб остался только счёт

    QRegularExpression numExpression("\\d+.\\d{1,2}");
    auto totalCost = numExpression.match(ui->totalCost->text()).captured().toDouble();

    InvoiceInfo invoiceInfo = {ui->orderNumber->text().toInt(), ui->suppliers->currentText(), &_goodsInOrderModel, totalCost};

    InvoiceWindow* iw = new InvoiceWindow(invoiceInfo);
    _mdiArea->addSubWindow(iw);
    iw->open();
    connect(iw, &QDialog::finished, this, [=](int) {
        QSqlQuery contractNumber("SELECT num FROM contracts WHERE supplier_code = " + _supplierCode + " and completion_date > now()");
        contractNumber.next();

        QSqlQuery addOrder;
        addOrder.prepare("INSERT INTO orders_sup (contract_num, paid) "
                         "VALUES (:contract_num, :paid) RETURNING num");
        addOrder.bindValue(":contract_num", contractNumber.value(0));
        addOrder.bindValue(":paid", iw->IsPaid());
        addOrder.exec();
        addOrder.next();

        QSqlQuery addOrderedSupGoods;
        addOrderedSupGoods.prepare("INSERT INTO ordered_sup_goods (order_sup_num, good_code, number, cost) "
                                   "VALUES (:order_sup_num, :good_code, :number, :cost)");
        addOrderedSupGoods.bindValue(":order_sup_num", addOrder.value(0));

        for (int i = 0; i < _goodsInOrderModel.rowCount(); i++) {
            addOrderedSupGoods.bindValue(":good_code", _goodsInOrderModel.index(i, 1).data());
            addOrderedSupGoods.bindValue(":number", _goodsInOrderModel.index(i, 2).data());
            addOrderedSupGoods.bindValue(":cost", _goodsInOrderModel.index(i, 3).data().toDouble());
            addOrderedSupGoods.exec();
        }

        QSqlQuery addSupReceipt;
        addSupReceipt.prepare("INSERT INTO receipts_sup (price, price_nds, date, order_sup_num) "
                              "VALUES (:price, :price_nds, :time, :order_sup_num)");
        addSupReceipt.bindValue(":price", iw->GetCost());
        addSupReceipt.bindValue(":price_nds", iw->GetCostVat());
        addSupReceipt.bindValue(":time", iw->IsPaid() ? "NOW()" : nullptr);
        addSupReceipt.bindValue(":order_sup_num", addOrder.value(0));
        addSupReceipt.exec();

        this->parentWidget()->close();
        this->setResult(QDialog::Accepted);
    });
}

void AddOrderWindow::on_removeFromOrder_clicked() {
    auto row = ui->goodsInOrder->currentIndex().row();
    _totalCost -= _goodsInOrderModel.index(row, 3).data().toDouble();
    _goodsInOrderModel.removeRows(row, 1);
    ui->goodsInOrder->selectRow(row);
    ui->totalCost->setText("Общая стоимость: " + QString::number(_totalCost).append(" рублей"));

    if (_goodsInOrderModel.rowCount() == 0)
        ui->removeFromOrder->setEnabled(false);
}

AddOrderWindow::~AddOrderWindow() {
    done(result());
    delete ui;
}
