#ifndef ADDORDERWINDOW_H
#define ADDORDERWINDOW_H

#include "../supplier/goodfilterproxymodel.h"
#include "goodsinordermodel.h"
#include "ordercostdelegate.h"
#include "invoicewindow.h"
#include <QRegularExpressionValidator>
#include <QDialog>
#include <QSqlTableModel>
#include <QMdiArea>

namespace Ui {
class AddOrderWindow;
}

class AddOrderWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddOrderWindow(int orderNumber, QMdiArea* mdiArea, QWidget* parent = nullptr);
    ~AddOrderWindow();

private slots:
    void on_getInvoice_clicked();
    void on_addToOrder_clicked();
    void on_removeFromOrder_clicked();

private:
    Ui::AddOrderWindow *ui;

    QMdiArea* _mdiArea;
    QSqlQueryModel _comboSupplierModel;
    QSqlQueryModel _comboSupplyCategoriesModel;
    QSqlTableModel _sqlCatalogModel;
    GoodsInOrderModel _goodsInOrderModel;
    GoodFilterProxyModel _catalogFilterModel;
    OrderCostDelegate _orderCostDelegate;

    QRegularExpressionValidator _quantityValidator;
    QRegularExpression _numExpression;
    QString _supplierCode;
    int _lastSupplierIndex = 0;
    bool _isNeedToSupplierWarning = true;
    double _totalCost = 0;

    void Init();
    void InitConnects();
    void SetupUI();
    void SetupTables();
    bool ErrorCheck() const;
};

#endif // ADDORDERWINDOW_H
