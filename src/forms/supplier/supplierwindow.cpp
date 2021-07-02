#include "supplierwindow.h"
#include "ui_supplierwindow.h"
#include <QMessageBox>
#include <QSqlQuery>

using namespace std;

SupplierWindow::SupplierWindow(QString supplierCode, QWidget* parent) : QDialog(parent), ui(new Ui::SupplierWindow),
    _supplierCode(std::move(supplierCode))
{
    ui->setupUi(this);
    Init();
    SetupUI();
}

void SupplierWindow::Init() {
//////////////////////////////////////  ПОСТАВЩИК  ////////////////////////////////////////
    _sqlSupplierModel.setTable("suppliers");
    _sqlSupplierModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    _sqlSupplierModel.select();
    _sqlSupplierModel.setFilter("code = '" + _supplierCode + "'");

    _sqlSupplierModel.setHeaderData(0, Qt::Horizontal, "Название");
    _sqlSupplierModel.setHeaderData(1, Qt::Horizontal, "Адрес");
    _sqlSupplierModel.setHeaderData(2, Qt::Horizontal, "Телефон");
    _sqlSupplierModel.setHeaderData(3, Qt::Horizontal, "Факс");
    _sqlSupplierModel.setHeaderData(4, Qt::Horizontal, "Директор");
    _sqlSupplierModel.setHeaderData(5, Qt::Horizontal, "Бухгалтер");
    _sqlSupplierModel.setHeaderData(6, Qt::Horizontal, "Расчетный счет");
    _sqlSupplierModel.setHeaderData(7, Qt::Horizontal, "ИНН");

    if (_supplierCode.isEmpty())
         _sqlSupplierModel.insertRow(0);

    _sqlSupplierTransposeModel.setSourceModel(&_sqlSupplierModel);

    ui->supplierInfo->setItemDelegateForRow(2, &_supplierPhoneDelegate);  // Телефон
    ui->supplierInfo->setItemDelegateForRow(3, &_supplierPhoneDelegate);  // Факс
    ui->supplierInfo->setItemDelegateForRow(7, &_supplierInnDelegate);    // ИНН
    ui->supplierInfo->setModel(&_sqlSupplierTransposeModel);
    ui->supplierInfo->hideRow(_sqlSupplierTransposeModel.rowCount() - 1); // Код поставщика
    ui->supplierInfo->selectRow(0);
    QFont font = ui->supplierInfo->verticalHeader()->font();
    font.setBold(true);
    ui->supplierInfo->verticalHeader()->setFont(font);

    _defaultNameOnSupplierEditing = _sqlSupplierTransposeModel.index(0, 0).data().toString();

/////////////////////////////////////  КАТАЛОГ  ////////////////////////////////////////
    _sqlCatalogModel.setTable("catalog_sup");
    _sqlCatalogModel.setFilter("supplier_code = '" + _supplierCode + "'");
    _sqlCatalogModel.setEditStrategy(QSqlTableModel::OnManualSubmit);
    _sqlCatalogModel.select();
    _sqlCatalogModel.setHeaderData(2, Qt::Horizontal, "Наименование");
    _sqlCatalogModel.setHeaderData(3, Qt::Horizontal, "Цена");

    _goodFilterModel.SetCategoryFilter(ui->clothingCategory->currentText());
    _goodFilterModel.setSourceModel(&_sqlCatalogModel);

    ui->clothingInfo->setItemDelegateForColumn(3, &_catalogCostDelegate);
    ui->clothingInfo->setModel(&_goodFilterModel);
    ui->clothingInfo->hideColumn(0);
    ui->clothingInfo->hideColumn(1);
    ui->clothingInfo->hideColumn(4);

    if (_sqlCatalogModel.rowCount() > 0)
        ui->clothingInfo->selectRow(_goodFilterModel.rowCount() - 1);

    font = ui->clothingInfo->horizontalHeader()->font();
    font.setBold(true);
    ui->clothingInfo->horizontalHeader()->setFont(font);
}

void SupplierWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    if (ui->clothingInfo->horizontalHeader()->isHidden())
        ui->clothingInfo->horizontalHeader()->show();

    ui->clothingInfo->setColumnWidth(2, 370);
    ui->clothingInfo->setColumnWidth(3, 70);

    connect(ui->clothingInfo->horizontalHeader(), &QHeaderView::sectionResized, ui->clothingInfo, &QTableView::resizeRowsToContents);

    ui->supplierVerticalLayout->setAlignment(ui->clothingCategoryGroup, Qt::AlignCenter);
    ui->supplierVerticalLayout->setAlignment(ui->save, Qt::AlignCenter);
}

bool SupplierWindow::ErrorCheck() const {
    for (int i = 0; i < _sqlSupplierTransposeModel.rowCount() - 1; i++) { // -1, потому что не нужен код поставщика
        if (_sqlSupplierTransposeModel.index(i, 0).data().toString().isEmpty()) {
            ui->supplierInfo->selectRow(i);
            QMessageBox::critical(nullptr, "Ошибка сохранения информации о поставщике", "Все поля информации о поставщике должны быть заполнены");
            ui->supplierInfo->setFocus();
            return false;
        }

        if (i == 2 || i == 3) {
            if (!PhoneLineEdit::IsValid(_sqlSupplierTransposeModel.index(i, 0).data().toString())) {
                ui->supplierInfo->selectRow(i);
                QMessageBox::critical(nullptr, "Ошибка сохранения информации о поставщике", "Неверно введён номер телефона");
                ui->supplierInfo->setFocus();
                return false;
            }
        }

        if (i == 7) {
            if (_sqlSupplierTransposeModel.index(i, 0).data().toString().count() < 10) {
                ui->supplierInfo->selectRow(i);
                QMessageBox::critical(nullptr, "Ошибка создания поставщика", "ИНН должен содержать 10 цифр");
                ui->supplierInfo->setFocus();
                return false;
            }
        }
    }

    if (_sqlCatalogModel.rowCount() == 0) {
        QMessageBox::critical(nullptr, "Ошибка сохранения каталога поставщика", "Каталог поставщика не может быть пустым");
        return false;
    }

    for (int i = 0; i < _sqlCatalogModel.rowCount(); i++) {
        auto category = _sqlCatalogModel.index(i, 1).data().toString();
        auto name = _sqlCatalogModel.index(i, 2).data().toString();
        auto price = _sqlCatalogModel.index(i, 3).data().toString();

        if (name.isEmpty() || price.isEmpty()) {
            ui->clothingCategory->setCurrentText(category);
            auto filterRow = _goodFilterModel.mapFromSource(_sqlCatalogModel.index(i, 0)).row();
            ui->clothingInfo->selectRow(filterRow);
            QMessageBox::critical(nullptr, "Ошибка", "Все поля товаров должны быть заполнены");
            ui->clothingInfo->setFocus();
            return false;
        }
    }

    auto currentSupplierName = _sqlSupplierTransposeModel.index(0, 0).data().toString();
    QSqlQuery existSupplier;
    existSupplier.prepare("SELECT 1 FROM suppliers where name = '" + currentSupplierName + "'");
    existSupplier.exec();

    // _defaultNameOnSupplierEditing != currentSupplierName <--- означает, что при редактировании введён другой поставщик, а не тот,
    // который уже был при открытии
    if (_defaultNameOnSupplierEditing != currentSupplierName && existSupplier.next()) {
        ui->supplierInfo->selectRow(0);
        QMessageBox::critical(nullptr, "Ошибка сохранения информации о поставщике",
                              "Поставщик \"" + currentSupplierName + "\" уже существует");
        ui->supplierInfo->setFocus();
        return false;
    }

    return true;
}

void SupplierWindow::on_save_clicked() {
    if (!ErrorCheck())
        return;

    if (_supplierCode.isEmpty()) { // Если добавляем поставщика
        QSqlQuery addSupplier;
        addSupplier.prepare("INSERT INTO suppliers (name, address, phone, fax, director, accountant, bank, inn) "
            "VALUES (:name, :address, :phone, :fax, :director, :accountant, :bank, :inn) RETURNING code");
        addSupplier.bindValue(":name", _sqlSupplierTransposeModel.index(0, 0).data());
        addSupplier.bindValue(":address", _sqlSupplierTransposeModel.index(1, 0).data());
        addSupplier.bindValue(":phone", _sqlSupplierTransposeModel.index(2, 0).data());
        addSupplier.bindValue(":fax", _sqlSupplierTransposeModel.index(3, 0).data());
        addSupplier.bindValue(":director", _sqlSupplierTransposeModel.index(4, 0).data());
        addSupplier.bindValue(":accountant", _sqlSupplierTransposeModel.index(5, 0).data());
        addSupplier.bindValue(":bank", _sqlSupplierTransposeModel.index(6, 0).data());
        addSupplier.bindValue(":inn", _sqlSupplierTransposeModel.index(7, 0).data());
        addSupplier.exec();
        addSupplier.next();

        for (int i = 0; i < _sqlCatalogModel.rowCount(); i++) {
            auto record = _sqlCatalogModel.record(i);
            record.setValue("supplier_code", addSupplier.value("code"));
            _sqlCatalogModel.setRecord(i, record);
        }
    }
    else {
        _sqlSupplierModel.submitAll();
    }

    _sqlCatalogModel.submitAll();
    this->parentWidget()->close();
    this->setResult(QDialog::Accepted);
}

void SupplierWindow::on_addClothing_clicked() {
    QSqlRecord record = _sqlCatalogModel.record();
    record.setValue(1, ui->clothingCategory->currentText()); // Задание категории
    record.setValue(4, _supplierCode); // Задание кода поставщика
    record.remove(0); // Удаление столбца с первичным ключом, чтоб не было NULL (сгенерирует значение уже сама БД)
    _sqlCatalogModel.insertRecord(-1, record);

    auto index = _goodFilterModel.index(_goodFilterModel.rowCount() - 1, 0);
    ui->clothingInfo->selectRow(index.row());
    ui->clothingInfo->setFocus();
}

void SupplierWindow::on_clothingCategory_currentIndexChanged(int) {
    _goodFilterModel.SetCategoryFilter(ui->clothingCategory->currentText());
    ui->clothingInfo->selectRow(_goodFilterModel.rowCount() - 1);
}

void SupplierWindow::on_clothingCategory_currentTextChanged(const QString& arg1) {
    _goodFilterModel.SetCategoryFilter(arg1);
    ui->clothingInfo->selectRow(_goodFilterModel.rowCount() - 1);
}

SupplierWindow::~SupplierWindow() {
    done(result());
    delete ui;
}
