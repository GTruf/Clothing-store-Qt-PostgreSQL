#ifndef CASHIERMAINWINDOW_H
#define CASHIERMAINWINDOW_H

#include "src/forms/client/clientdiscountdelegate.h"
#include "src/forms/order/goodsinordermodel.h"
#include "src/forms/order/ordercostdelegate.h"
#include "src/forms/stock/sqlstockmodel.h"
#include "src/forms/stock/stockfilterproxymodel.h"
#include <QMainWindow>
#include <QRegularExpressionValidator>
#include <QSqlQueryModel>
#include <QSqlTableModel>

namespace Ui {
class CashierMainWindow;
}

class CashierMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit CashierMainWindow(QWidget *parent = nullptr);
    ~CashierMainWindow();
    void UpdateStock(); // Склад может меняться как у менеджера, так и у кассира, поэтому при запуске окна нужно обновлять его

private slots:
    void on_addToOrder_clicked();
    void on_payment_clicked();
    void on_cancel_clicked();
    void on_addClient_clicked();
    void on_useClientCard_clicked();
    void on_removeFromOrder_clicked();

private:
    Ui::CashierMainWindow *ui;
    QRegularExpressionValidator _quantityValidator;
    double _totalCost = 0, _totalCostWithDiscount = 0;
    QString _clientCode;

    SqlStockModel _sqlStockModel;
    QSqlQueryModel _comboStockCategoriesModel;
    QSqlTableModel _sqlClientsModel;
    GoodsInOrderModel _goodsInOrderModel;
    StockFilterProxyModel _stockFilterModel;
    ClientDiscountDelegate _clientDiscountDelegate;
    OrderCostDelegate _orderCostDelegate;

    void Init();
    void SetupUI();
    void SetupTables();
    void PrintInvoice(const QString& orderNumber);
    void ClearReceiptInfo();
};

#endif // CASHIERMAINWINDOW_H
