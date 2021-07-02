#include "viewcontractwindow.h"
#include "ui_viewcontractwindow.h"
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QSqlQuery>
#include <QDate>

ViewContractWindow::ViewContractWindow(const QString& contractNumber, QWidget* parent) : QDialog(parent), ui(new Ui::ViewContractWindow) {
    ui->setupUi(this);
    SetupUI();
    DisplayContractInfo(contractNumber);
}

void ViewContractWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->contractNumberGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->supplierGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->print, Qt::AlignCenter);
    ui->description->setTabStopDistance(ui->description->fontMetrics().horizontalAdvance(' ') * 4);
}

void ViewContractWindow::DisplayContractInfo(const QString& contractNumber) const {
    QSqlQuery contractInfo;
    contractInfo.prepare("SELECT substring(c.num, '\\d+')::int, c.conclusion_date, c.completion_date, s.name, c.text, s.code "
                         "FROM contracts AS c "
                         "JOIN suppliers AS s ON c.num = :contractNum AND c.supplier_code = s.code");
    contractInfo.bindValue(":contractNum", contractNumber);
    contractInfo.exec();
    contractInfo.next();

    ui->contractNumber->setText(contractInfo.value(0).toString());
    ui->conclusionDate->setText(QDate::fromString(contractInfo.value(1).toString(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
    ui->completionDate->setText(QDate::fromString(contractInfo.value(2).toString(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
    ui->supplier->setText(contractInfo.value(3).toString());
    ui->description->setText(contractInfo.value(4).toString());
    ui->description->moveCursor(QTextCursor::Start); // Почему-то при открытии окна скролл не в самом верху
    _supplierCode = contractInfo.value(5).toString();
}

void ViewContractWindow::on_print_clicked() {
    QPrinter printer;
    QPrintPreviewDialog printWindow(&printer, nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    printWindow.setWindowTitle("Просмотр печати договора");
    printWindow.resize(1000, 600);
    connect(&printWindow, &QPrintPreviewDialog::paintRequested, this, &ViewContractWindow::PaintContractPages);
    printWindow.exec();
}

void ViewContractWindow::PaintContractPages(QPrinter* printer) const {
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
    _supplierCode + "'");
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

ViewContractWindow::~ViewContractWindow() {
    delete ui;
}

