#include "cashiermainwindow.h"
#include "ui_cashiermainwindow.h"
#include "src/forms/client/clientdiscountwindow.h"
#include <QMessageBox>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QDateTime>

using namespace std;

CashierMainWindow::CashierMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::CashierMainWindow),
    _quantityValidator(QRegularExpression("0|[1-9]\\d{2}"))
{
    ui->setupUi(this);
    Init();
    SetupUI();

    connect(ui->stockGoodCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
        _stockFilterModel.SetCategoryFilter(ui->stockGoodCategory->currentText());
        ui->stockTable->selectRow(0);
    });
}

void CashierMainWindow::SetupUI() {
    SetupTables();

    ui->quantity->setValidator(&_quantityValidator);

    ui->clientVerticalLayout->setAlignment(ui->registrationDateGroup, Qt::AlignCenter);
    ui->saleWidget->layout()->setAlignment(ui->receiptNumberGroup, Qt::AlignCenter);
    ui->saleWidget->layout()->setAlignment(ui->stockGoodCategoryGroup, Qt::AlignCenter);
    ui->saleWidget->layout()->setAlignment(ui->addToOrder, Qt::AlignCenter);
    ui->saleWidget->layout()->setAlignment(ui->costCardGroup, Qt::AlignCenter);

    ui->stockTable->resizeColumnsToContents();
    ui->stockTable->horizontalHeader()->resizeSection(2, 210);
    ui->stockTable->horizontalHeader()->resizeSection(3, 70);

    ui->clientsTable->horizontalHeader()->resizeSection(0, 220);
    ui->clientsTable->horizontalHeader()->resizeSection(1, 120);
    ui->clientsTable->horizontalHeader()->resizeSection(2, 20);

    ui->goodsInOrder->horizontalHeader()->resizeSection(0, 220);
    ui->goodsInOrder->horizontalHeader()->resizeSection(1, 80);
    ui->goodsInOrder->horizontalHeader()->resizeSection(2, 80);
    ui->goodsInOrder->horizontalHeader()->resizeSection(3, 80);

    ui->totalCost->setText(QString::number(_totalCost, 'f', 2) + QLocale::system().currencySymbol().prepend(" "));
}

void CashierMainWindow::SetupTables() {
    QVector<QTableView*> tables = {ui->stockTable, ui->clientsTable, ui->goodsInOrder};
    for (const auto& table : tables) {
        table->setWordWrap(true);
        table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
        table->horizontalHeader()->setFixedHeight(50);
        connect(table->horizontalHeader(), &QHeaderView::sectionResized, table, &QTableView::resizeRowsToContents);
        table->horizontalHeader()->show(); // Почему-то при запуске иногда прячутся заголовки
    }
}

void CashierMainWindow::Init() {
    QSqlQuery receiptNumber("SELECT substring(num, '\\d+')::int + 1 FROM receipts_client ORDER BY substring(num, '\\d+')::int DESC LIMIT 1");
    if (receiptNumber.next())
        ui->receiptNumber->setText(receiptNumber.value(0).toString());
    else
        ui->receiptNumber->setText("1"); // Если чеков ещё нет

    _comboStockCategoriesModel.setQuery("SELECT DISTINCT cat.name FROM catalog_sup as cat "
                                        "JOIN stock as st ON st.code = cat.code AND st.number > 0");
    ui->stockGoodCategory->setModel(&_comboStockCategoriesModel);

    _sqlStockModel.setQuery("SELECT cat.name, st.code, cat.description, st.price, st.number "
                            "FROM stock AS st "
                            "JOIN catalog_sup AS cat ON st.code = cat.code "
                            "WHERE st.number > 0 "
                            "ORDER BY st.number DESC");
    _sqlStockModel.setHeaderData(1, Qt::Horizontal, "Код товара");
    _sqlStockModel.setHeaderData(2, Qt::Horizontal, "Наименование");
    _sqlStockModel.setHeaderData(3, Qt::Horizontal, "Цена");
    _sqlStockModel.setHeaderData(4, Qt::Horizontal, "Кол-во");
    ui->stockTable->resizeColumnsToContents();

    _stockFilterModel.SetCategoryFilter(ui->stockGoodCategory->currentText());
    _stockFilterModel.setSourceModel(&_sqlStockModel);
    ui->stockTable->setModel(&_stockFilterModel);
    ui->stockTable->hideColumn(0); // Категория товара (используется в фильтрации склада)

    _sqlClientsModel.setTable("clients");
    _sqlClientsModel.select();
    _sqlClientsModel.setHeaderData(0, Qt::Horizontal, "Имя");
    _sqlClientsModel.setHeaderData(1, Qt::Horizontal, "Телефон");
    _sqlClientsModel.setHeaderData(3, Qt::Horizontal, "Скидка");
    _sqlClientsModel.setHeaderData(4, Qt::Horizontal, "Номер карты");

    ui->clientsTable->setModel(&_sqlClientsModel);
    ui->clientsTable->setItemDelegateForColumn(3, &_clientDiscountDelegate);
    ui->clientsTable->hideColumn(2); // Полная стоимость покупок
    ui->clientsTable->hideColumn(5); // Код клиента
    connect(&_sqlClientsModel, &QAbstractTableModel::dataChanged, ui->clientsTable, &QTableView::resizeRowsToContents);

    ui->goodsInOrder->setModel(&_goodsInOrderModel);
    ui->goodsInOrder->setItemDelegateForColumn(3, &_orderCostDelegate);
}


void CashierMainWindow::UpdateStock() {
    _comboStockCategoriesModel.setQuery(_comboStockCategoriesModel.query().lastQuery());
    _sqlStockModel.setQuery(_sqlStockModel.query().lastQuery());
    ui->stockGoodCategory->setCurrentIndex(0);
    ui->stockTable->selectRow(0);
}

void CashierMainWindow::on_addClient_clicked() {
    if (ui->fullName->text().isEmpty() || ui->phone->text().isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка при добавлении клиента", "Заполните все поля");
        return;
    }

    if (!ui->phone->IsValid()) {
        QMessageBox::critical(nullptr, "Ошибка при добавлении клиента", "Неверно введён номер телефона");
        ui->phone->setFocus();
        return;
    }

    QSqlRecord newClient = _sqlClientsModel.record();
    newClient.remove(2); // Чтобы purchase_amount по умолчанию стал 0 в БД
    newClient.remove(2); // Чтобы discount по умолчанию стал 0 в БД (2 из-за сдвига столбцов после удаления)
    newClient.remove(2); // Чтобы карта клиента сгенерировалась в БД
    newClient.remove(2); // Чтобы код клиента сгенерировался в БД
    newClient.setValue("name", ui->fullName->text());
    newClient.setValue("phone", ui->phone->text());

    _sqlClientsModel.insertRecord(-1, newClient); // -1 - в конец
    _sqlClientsModel.select();
    ui->clientsTable->selectRow(_sqlClientsModel.rowCount() - 1);

    ui->phone->Clear();
    ui->fullName->clear();
}

void CashierMainWindow::on_addToOrder_clicked() {
    if (ui->quantity->text().isEmpty() || ui->quantity->text() == "0") {
        QMessageBox::critical(nullptr, "Ошибка добавления товара в заказ", "Количество товара должно быть не меньше одного");
        ui->quantity->setFocus();
        ui->quantity->selectAll();
        return;
    }

    static const QRegularExpression numExpression("(^(0|[1-9][0-9]{0,9})(,[0-9]{0,2})?)?");
    const auto stockGoodIndex = ui->stockTable->currentIndex();
    const auto stockGoodQuantity = _stockFilterModel.index(stockGoodIndex.row(), 4).data().toInt();

    const auto quantity = ui->quantity->text().toInt();
    const auto goodCode = _stockFilterModel.index(stockGoodIndex.row(), 1).data().toString();
    const auto supplyGoodDescription = _stockFilterModel.index(stockGoodIndex.row(), 2).data().toString();
    const auto stockGoodPriceStr = _stockFilterModel.index(stockGoodIndex.row(), 3).data().toString().simplified().remove(' ');
    const auto totalStockGoodPrice = numExpression.match(stockGoodPriceStr).captured().replace(',', '.').toDouble() * quantity;

    auto row = _goodsInOrderModel.FindGood(goodCode);
    if (row >= 0) { // Если товар уже добавлен, к нему просто прибавится количество и цена
        if ((quantity + _goodsInOrderModel.index(row, 2).data().toInt()) > stockGoodQuantity) {
            QMessageBox::critical(nullptr, "Ошибка добавления товара в заказ", "Количество товара не должно превышать количество на складе");
            ui->quantity->setFocus();
            ui->quantity->selectAll();
            return;
        }

        _goodsInOrderModel.UpdateGoodInfo(row, quantity, totalStockGoodPrice);
        ui->goodsInOrder->update(_goodsInOrderModel.index(row, 2));
        ui->goodsInOrder->update(_goodsInOrderModel.index(row, 3));
        ui->goodsInOrder->selectRow(row);
    }
    else {  // Добавление товара в заказ
        if (quantity > stockGoodQuantity) {
            QMessageBox::critical(nullptr, "Ошибка добавления товара в заказ", "Количество товара не должно превышать количество на складе");
            ui->quantity->setFocus();
            ui->quantity->selectAll();
            return;
        }

        row = _goodsInOrderModel.rowCount();
        _goodsInOrderModel.insertRows(row, 1);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 0), supplyGoodDescription);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 1), goodCode);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 2), quantity);
        _goodsInOrderModel.setData(_goodsInOrderModel.index(row, 3), totalStockGoodPrice);
        ui->goodsInOrder->selectRow(_goodsInOrderModel.rowCount() - 1);
    }

    _totalCost += totalStockGoodPrice;
    ui->totalCost->setText(QString::number(_totalCost, 'f', 2) + QLocale::system().currencySymbol().prepend(" "));
    ui->removeFromOrder->setEnabled(true);
}


void CashierMainWindow::on_removeFromOrder_clicked() {
    auto row = ui->goodsInOrder->currentIndex().row();
    _totalCost -= _goodsInOrderModel.index(row, 3).data().toDouble();
    _goodsInOrderModel.removeRows(row, 1);
    ui->goodsInOrder->selectRow(row);
    ui->totalCost->setText(QString::number(_totalCost, 'f', 2) + QLocale::system().currencySymbol().prepend(" "));

    if (_goodsInOrderModel.rowCount() == 0)
        ui->removeFromOrder->setEnabled(false);
}

void CashierMainWindow::on_useClientCard_clicked() {
    if (_goodsInOrderModel.rowCount() == 0) {
        QMessageBox::critical(nullptr, "Ошибка использования карты клиента", "Добавьте в корзину хотя бы один товар");
        return;
    }

    ui->totalCost->setText(QString::number(_totalCost, 'f', 2) + QLocale::system().currencySymbol().prepend(" "));
    ui->clientCardFullName->clear();
    _totalCostWithDiscount = 0;

    ClientDiscountWindow cdw(QString::number(_totalCost, 'f', 2));
    if (cdw.exec() == QDialog::Accepted) {
        _clientCode = cdw.GetClientCode();
        ui->totalCost->setText(cdw.GetTotalCostWithDiscount() + QLocale::system().currencySymbol().prepend(" "));
        ui->clientCardFullName->setText(cdw.GetFullName());
        _totalCostWithDiscount = cdw.GetTotalCostWithDiscount().toDouble();
    }
}

void CashierMainWindow::on_cancel_clicked() {
    auto result = QMessageBox::warning(nullptr, "Предупреждение при отмене создания чека",
                "При отмене чека очистится корзина с товарами. Продолжить?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (result == QMessageBox::Yes)
        ClearReceiptInfo();
}

void CashierMainWindow::on_payment_clicked() {
    if (_goodsInOrderModel.rowCount() == 0) {
        QMessageBox::critical(nullptr, "Ошибка создания чека", "Добавьте в корзину хотя бы один товар");
        return;
    }

    QSqlQuery addOrderClient;
    addOrderClient.prepare("INSERT INTO orders_client (client_code) "
                           "VALUES (:client_code) RETURNING num");
    addOrderClient.bindValue(":client_code", _clientCode.isEmpty() ? QVariant() : _clientCode);
    addOrderClient.exec();
    addOrderClient.next();

    QSqlQuery addOrderedClientGoods;
    addOrderedClientGoods.prepare("INSERT INTO ordered_client_goods (order_client_num, good_code, number, cost) "
                                  "VALUES (:order_client_num, :good_code, :number, :cost)");
    addOrderedClientGoods.bindValue(":order_client_num", addOrderClient.value(0));

    for (int i = 0; i < _goodsInOrderModel.rowCount(); i++) {
        addOrderedClientGoods.bindValue(":good_code", _goodsInOrderModel.index(i, 1).data());
        addOrderedClientGoods.bindValue(":number", _goodsInOrderModel.index(i, 2).data());
        addOrderedClientGoods.bindValue(":cost", _goodsInOrderModel.index(i, 3).data().toDouble());
        addOrderedClientGoods.exec();
    }

    if (_clientCode.isEmpty()) { // Если не применяли карту клиента
        _totalCostWithDiscount = _totalCost; // Стоимость со скидкой такая же, как текущая общая стоимость
    }

    QSqlQuery addReceiptClient;
    addReceiptClient.prepare("INSERT INTO receipts_client (order_client_num, price, discount_price) "
                             "VALUES (:order_client_num, :price, :discount_price) RETURNING substring(num, '\\d+')::int + 1");
    addReceiptClient.bindValue(":order_client_num", addOrderClient.value(0));
    addReceiptClient.bindValue(":price", _totalCost);
    addReceiptClient.bindValue(":discount_price", _totalCostWithDiscount);
    addReceiptClient.exec();
    addReceiptClient.next();

    if (!_clientCode.isEmpty()) { // Если применили карту клиента
        _sqlClientsModel.select(); // У клиента может поменяться скидка, поэтому обновляем таблицу
    }

    _sqlStockModel.setQuery(_sqlStockModel.query().lastQuery());
    ui->stockTable->selectRow(0);
    PrintInvoice(addOrderClient.value(0).toString());
    ui->receiptNumber->setText(addReceiptClient.value(0).toString());
    ClearReceiptInfo();
}


void CashierMainWindow::ClearReceiptInfo() {
    _goodsInOrderModel.removeRows(0, _goodsInOrderModel.rowCount());
    ui->totalCost->setText("0 " + QLocale::system().currencySymbol());
    ui->clientCardFullName->clear();
    ui->removeFromOrder->setEnabled(false);
    _totalCost = _totalCostWithDiscount = 0;
    _clientCode.clear();
}

void CashierMainWindow::PrintInvoice(const QString& orderNumber) {
    QPrinter printer;
    QPrintPreviewDialog printWindow(&printer, nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    printWindow.setWindowTitle("Просмотр печати счет-фактуры");
    printWindow.resize(1000, 600);

    connect(&printWindow, &QPrintPreviewDialog::paintRequested, this, [&](QPrinter* printer) {

    QPainter painter(&printWindow);
    painter.begin(printer);

    int pageWidth = printer->pageLayout().paintRectPixels(printer->resolution()).width() * 0.65;
    int pageHeight = printer->pageLayout().paintRectPixels(printer->resolution()).height() * 0.65;
    QRect rect (pageWidth * 0.25, 20, pageWidth, pageHeight);

    QPen pen = painter.pen();
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    //painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));

    QFont font = painter.font();
    font.setPixelSize(20);
    painter.setFont(font);

    QSqlQuery storeQuery("SELECT name, address, inn FROM stores");
    storeQuery.next();

    QSqlQuery price("SELECT price, discount_price "
                    "FROM receipts_client "
                    "WHERE order_client_num = '" + orderNumber + "'");
    price.next();

    rect.setHeight(0.2 * pageHeight);
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, " " + storeQuery.value(0).toString() + "\n"
                        " КАССОВЫЙ ЧЕК        " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") + "\n"
                        " ПРИХОД\n"
                        " Продажа №" + ui->receiptNumber->text());

    rect.setTop(0.21 * pageHeight);
    rect.setHeight(0);
    painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));

    font.setPixelSize(15);
    painter.setFont(font);

    QSqlQuery query("SELECT cat.description, st.price, g.number, g.cost "
                    "FROM ordered_client_goods AS g "
                    "JOIN stock AS st ON st.code = g.good_code "
                    "JOIN catalog_sup AS cat ON cat.code = g.good_code "
                    "WHERE g.order_client_num = '" + orderNumber + "'");

    int top = 0.16 * pageHeight;
    while(query.next()) {
        top += 0.06 * pageHeight;

        if(top > pageHeight - 20 ){
            printer->newPage();
            top = 0.16 * pageHeight;
        }

        rect.setTop(top);
        rect.setHeight(0.06 * pageHeight);
        rect.setLeft(120);
        rect.setWidth(200);//наименование
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, query.value(0).toString());

        rect.setLeft(330);
        rect.setWidth(90);//цена за единицу
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, query.value(1).toString());

        rect.setLeft(420);
        rect.setWidth(80);//кол-во
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, "* " + query.value(2).toString());

        rect.setLeft(500);
        rect.setWidth(120);//сумма
        painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, "= " + query.value(3).toString());
    }

    static QRegularExpression numExpression("(^(0|[1-9][0-9]{0,9})(,[0-9]{0,2})?)?");
    auto strPriceWithoutDiscount = price.value(0).toString().simplified().remove(' ');
    auto priceWithoutDiscount = numExpression.match(strPriceWithoutDiscount).captured().replace(',', '.').toDouble();

    auto strPriceWithDiscount = price.value(1).toString().simplified().remove(' ');
    auto priceWithDiscount = numExpression.match(strPriceWithDiscount).captured().replace(',', '.').toDouble();

    rect.setTop(top + 0.06 * pageHeight);
    rect.setHeight(0.06 * pageHeight);
    rect.setLeft(120);
    rect.setWidth(200);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                     "Скидка " + QString::number((1 - priceWithDiscount / priceWithoutDiscount) * 100, 'f', 2).append("%"));
    rect.setLeft(500);
    rect.setWidth(120);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter,
                     "= " + QString::number(priceWithoutDiscount - priceWithDiscount, 'f', 2));


    rect.setTop(top + 0.12 * pageHeight);
    rect.setHeight(0.06 * pageHeight);
    rect.setLeft(120);
    rect.setWidth(200);

    font.setPixelSize(20);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap, "ИТОГ");
    rect.setLeft(500);
    rect.setWidth(130);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, "= " + price.value(1).toString());

    rect.setTop(top + 0.18 * pageHeight);
    rect.setHeight(0.2 * pageHeight);
    rect.setLeft(121);
    rect.setWidth(pageWidth);
    painter.drawText(rect, Qt::AlignVCenter | Qt::AlignLeft, " КАССИР - Петров А.В.\n"
                        " Место рассчетов:\n"
                        " " + storeQuery.value(1).toString() + "\n"
                        " ИНН: " + storeQuery.value(2).toString());
    rect.setHeight(0);
    painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
    rect.setTop(top + 0.38 * pageHeight);
    rect.setHeight(0.05 * pageHeight);
    painter.drawText(rect, Qt::AlignCenter, "Спасибо за покупку!");
    });

    printWindow.exec();
}

CashierMainWindow::~CashierMainWindow() {
    delete ui;
}
