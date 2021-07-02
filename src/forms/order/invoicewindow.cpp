#include "invoicewindow.h"
#include "ui_invoicewindow.h"

InvoiceWindow::InvoiceWindow(const InvoiceInfo& invoiceInfo, QWidget* parent) : QDialog(parent), ui(new Ui::InvoiceWindow) {
    ui->setupUi(this);
    DisplayInvoiceInfo(invoiceInfo);
    SetupUI();
}

bool InvoiceWindow::IsPaid() const {
    return ui->payment->isChecked();
}

double InvoiceWindow::GetCost() const {
    return ui->costInfo->item(0, 1)->text().toDouble();
}

double InvoiceWindow::GetCostVat() const {
    return ui->costInfo->item(0, 3)->text().toDouble();
}

void InvoiceWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->invoiceNumberGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->supplierGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->paymentGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->ok, Qt::AlignCenter);
    SetupTables();
}

void InvoiceWindow::SetupTables() {
    ui->goodsInOrder->setWordWrap(true);
    ui->goodsInOrder->resizeColumnsToContents();
    ui->goodsInOrder->horizontalHeader()->resizeSection(0, 200);
    ui->goodsInOrder->setItemDelegateForColumn(3, &_orderCostDelegate);
    connect(ui->goodsInOrder->horizontalHeader(), &QHeaderView::sectionResized, ui->goodsInOrder, &QTableView::resizeRowsToContents);
    if (ui->goodsInOrder->horizontalHeader()->isHidden())
        ui->goodsInOrder->horizontalHeader()->show();

    ui->costInfo->horizontalHeader()->setDefaultSectionSize(135);
}

void InvoiceWindow::DisplayInvoiceInfo(const InvoiceInfo& invoiceInfo) const {
    ui->goodsInOrder->setModel(invoiceInfo.goodsInOrder);

    ui->invoiceNumber->setText(QString::number(invoiceInfo.invoiceNumber));
    ui->supplier->setText(invoiceInfo.supplierName);

    ui->costInfo->item(0, 1)->setText(QString::number(invoiceInfo.totalCost, 'f', 2));
    ui->costInfo->item(0, 3)->setText(QString::number(invoiceInfo.totalCost * 1.2, 'f', 2));
}

void InvoiceWindow::on_ok_clicked() {
    this->parentWidget()->close();
    this->setResult(QDialog::Accepted);
}

InvoiceWindow::~InvoiceWindow() {
    done(result());
    delete ui;
}
