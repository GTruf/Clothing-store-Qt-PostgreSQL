#ifndef INVOICEWINDOW_H
#define INVOICEWINDOW_H

#include "goodsinordermodel.h"
#include "ordercostdelegate.h"
#include <QDialog>

namespace Ui {
class InvoiceWindow;
}

struct InvoiceInfo {
    int invoiceNumber;
    QString supplierName;
    GoodsInOrderModel* goodsInOrder;
    double totalCost;
};

class InvoiceWindow : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceWindow(const InvoiceInfo& invoiceInfo, QWidget* parent = nullptr);
    bool IsPaid() const;
    double GetCost() const;
    double GetCostVat() const;
    ~InvoiceWindow();

private slots:
    void on_ok_clicked();

private:
    Ui::InvoiceWindow *ui;
    OrderCostDelegate _orderCostDelegate;

    void SetupUI();
    void SetupTables();
    void DisplayInvoiceInfo(const InvoiceInfo& invoiceInfo) const;
};

#endif // INVOICEWINDOW_H
