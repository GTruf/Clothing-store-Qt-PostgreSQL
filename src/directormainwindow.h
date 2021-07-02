#ifndef DIRECTORMAINWINDOW_H
#define DIRECTORMAINWINDOW_H

#include "director_utils.h"
#include <QMainWindow>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class DirectorMainWindow; }
QT_END_NAMESPACE

class DirectorMainWindow : public QMainWindow {
    Q_OBJECT

public:
    DirectorMainWindow(QWidget* parent = nullptr);
    ~DirectorMainWindow();
    void UpdateStock(); // Склад может меняться как у менеджера, так и у кассира, поэтому при запуске окна нужно обновлять его

private slots:
    void on_addContract_clicked();
    void OnContractsDoubleClicked(const QModelIndex& index);
    void OnSuppliersDoubleClicked(const QModelIndex& index);
    void OnOrdersDoubleClicked(const QModelIndex& index);
    void on_addSupplier_clicked();
    void on_addOrder_clicked();
    void on_analysisOk_clicked();
    void on_stockPrint_clicked();
    void on_saveStoreInfo_clicked();

private:
    Ui::DirectorMainWindow *ui;
    uint16_t _lastTab = 0;
    QSqlQueryModel _sqlContractModel;
    QSqlQueryModel _sqlSupplierModel;
    QSqlQueryModel _sqlOrderModel;
    SqlStockModel _sqlStockModel;
    QSqlTableModel _sqlStoreModel;
    ContractFilterProxyModel _contractFilterModel;
    SupplierFilterProxyModel _supplierFilterModel;
    OrderFilterProxyModel _orderFilterModel;
    StockFilterProxyModel _stockFilterModel;
    QTransposeProxyModel _sqlStoreTransposeModel;
    SqlOrderBooleanFormatDelegate _sqlOrderBooleanDelegate;
    SqlSupplierCatalogCostDelegate _stockCostDelegate;
    SqlSupplierInnDelegate _storeInnDelegate;
    SqlSupplierPhoneDelegate _storePhoneDelegate;

    QTimer _checkExistConnectionsTimer;
    QList<QMetaObject::Connection> _danglingConnections;

    void Init();
    void InitConnects();
    void SetupUI();
    void SetupTables();
    void SetupContracts();
    void SetupSuppliers();
    void SetupOrders();
    void SetupStock();
    void SetupStore();
    void PaintStockPages(QPrinter* printer);
    void PaintStockHeader(QPainter& painter);
};
#endif // DIRECTORMAINWINDOW_H
