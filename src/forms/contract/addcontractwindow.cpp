#include "addcontractwindow.h"
#include "ui_addcontractwindow.h"
#include <QMessageBox>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QSqlQuery>
#include <QScrollBar>

AddContractWindow::AddContractWindow(int contractNumber, QWidget* parent) : QDialog(parent), ui(new Ui::AddContractWindow) {
    ui->setupUi(this);
    Init();
    SetupUI();
    ui->contractNumber->setText(QString::number(contractNumber));
}

void AddContractWindow::Init() {
    _comboSupplierModel.setQuery("SELECT DISTINCT ON (name) name, code FROM suppliers "
                                 "WHERE NOT EXISTS (SELECT num FROM contracts WHERE completion_date >= NOW() "
                                 "AND contracts.supplier_code = suppliers.code)");
    ui->suppliers->setModel(&_comboSupplierModel);
}

void AddContractWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->contractNumberGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->supplierGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->printSaveGroup, Qt::AlignRight);

    ui->conclusionDate->setDate(QDate::currentDate());
    ui->completionDate->setDate(QDate::currentDate().addYears(1));

    ui->description->setTabStopDistance(ui->description->fontMetrics().horizontalAdvance(' ') * 4);
    ui->description->setFocus();
}

bool AddContractWindow::ErrorCheck() const {
    if (ui->conclusionDate->date() >= ui->completionDate->date()) {
        QMessageBox::critical(nullptr, "Ошибка создания договора",
                              "Дата окончания срока действия договора не может быть раньше или такой же, как дата заключения договора");
        return false;
    }

    return true;
}

void AddContractWindow::on_save_clicked() {
    if (!ErrorCheck())
        return;

    QSqlQuery addContract;
    addContract.prepare("INSERT INTO contracts (text, conclusion_date, completion_date, supplier_code, store_code) "
        "VALUES (:text, :conclusion_date, :completion_date, :supplier_code, :store_code)");
    addContract.bindValue(":text", ui->description->toPlainText());
    addContract.bindValue(":conclusion_date", ui->conclusionDate->text());
    addContract.bindValue(":completion_date", ui->completionDate->text());
    addContract.bindValue(":supplier_code", _comboSupplierModel.index(ui->suppliers->currentIndex(), 1).data());
    addContract.bindValue(":store_code", "МАГ-1");
    addContract.exec();

    this->parentWidget()->close();
    this->setResult(QDialog::Accepted);
}

void AddContractWindow::on_print_clicked() {
    QPrinter printer;
    QPrintPreviewDialog printWindow(&printer, nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    printWindow.setWindowTitle("Просмотр печати договора");
    printWindow.resize(1000, 600);
    connect(&printWindow, &QPrintPreviewDialog::paintRequested, this, &AddContractWindow::PaintContractPages);
    printWindow.exec();
}

void AddContractWindow::PaintContractPages(QPrinter* printer) const {
    QPainter painter;
    painter.begin(printer);

    int pageWidth = printer->pageLayout().paintRectPixels(printer->resolution()).width() - 20;
    int pageHeight = printer->pageLayout().paintRectPixels(printer->resolution()).height() - 20;
    QRect rect (20, 20, pageWidth, 0.1 * pageHeight);

    QFont font = painter.font();
    font.setPixelSize(25);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, "Договор о поставке № " + ui->contractNumber->text());

    rect.setTop(0.1 * pageHeight);
    rect.setHeight(0.05 * pageHeight);
    font.setPixelSize(17);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignRight | Qt::AlignVCenter, "Дата заключения " + ui->conclusionDate->text());

    rect.setTop(0.15 * pageHeight);
    rect.setHeight(0.05 * pageHeight);
    painter.drawText(rect, Qt::AlignRight | Qt::AlignTop, "Дата окончания срока действия " + ui->completionDate->text());

    rect.setTop(0.2 * pageHeight);
    rect.setHeight(0.5 * pageHeight);
    rect.setLeft(0.05 * pageWidth);
    rect.setWidth(0.9 * pageWidth);
    painter.drawText(rect, Qt::AlignJustify | Qt::AlignVCenter | Qt::TextWordWrap, ui->description->toPlainText());

    rect.setTop(0.7 * pageHeight);
    rect.setHeight(0.03 * pageHeight);
    rect.setLeft(20);
    rect.setWidth(pageWidth);
    painter.drawText(rect, Qt::AlignHCenter | Qt::AlignTop, "Юридические адреса и реквизиты сторон");

    QSqlQuery sup ("SELECT * FROM suppliers WHERE code = '" +
    _comboSupplierModel.index(ui->suppliers->currentIndex(), 1).data().toString() + "'");
    sup.next();

    rect.setTop(0.73 * pageHeight);
    rect.setHeight(0.24 * pageHeight);
    rect.setLeft(20);
    rect.setWidth(pageWidth / 2 - 10);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                        "ПРОДАВЕЦ - \"" + sup.value(0).toString() + "\"\n"
                        "Юридический адрес:\n " + sup.value(1).toString() + "\n"
                        "Расчетный счет:\n " + sup.value(6).toString() + "\n"
                        "ИНН: " + sup.value(7).toString() + "\n"
                        "Директор: " + sup.value(4).toString() + "\n\n"
                        " ______________");

    QSqlQuery store ("SELECT * FROM stores");
    store.next();
    rect.setLeft(30 + pageWidth / 2);
    rect.setWidth(pageWidth / 2);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                        "ПОКУПАТЕЛЬ - \"" + store.value(1).toString() + "\"\n"
                        "Юридический адрес:\n " + store.value(2).toString() + "\n"
                        "Расчетный счет:\n " + store.value(7).toString() + "\n"
                        "ИНН: " + store.value(8).toString() + "\n"
                        "Директор: " + store.value(5).toString() + "\n\n"
                        " ______________");
    rect.setTop(0.95 * pageHeight);
    rect.setHeight(0.05 * pageHeight);
    rect.setLeft(20);
    rect.setWidth(pageWidth);
    painter.drawText(rect, Qt::AlignLeft | Qt::AlignTop, QDate::currentDate().toString("dd.MM.yyyy"));
}

AddContractWindow::~AddContractWindow() {
    done(result());
    delete ui;
}
