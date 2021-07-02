#ifndef SUPPLIERWINDOW_H
#define SUPPLIERWINDOW_H

#include "goodfilterproxymodel.h"
#include "sqlsupplierinndelegate.h"
#include "sqlsuppliercatalogcostdelegate.h"
#include "sqlsupplierphonedelegate.h"
#include <QDialog>
#include <QSql>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QTransposeProxyModel>

namespace Ui {
class SupplierWindow;
}

class SupplierWindow : public QDialog {
    Q_OBJECT

public:
    explicit SupplierWindow(QString supplierCode = "", QWidget* parent = nullptr);
    ~SupplierWindow();

private slots:
    void on_save_clicked();
    void on_addClothing_clicked();
    void on_clothingCategory_currentIndexChanged(int);
    void on_clothingCategory_currentTextChanged(const QString& arg1);

private:
    Ui::SupplierWindow *ui;
    QSqlTableModel _sqlSupplierModel;
    QSqlTableModel _sqlCatalogModel;
    QTransposeProxyModel _sqlSupplierTransposeModel;
    SqlSupplierInnDelegate _supplierInnDelegate;
    SqlSupplierPhoneDelegate _supplierPhoneDelegate;
    SqlSupplierCatalogCostDelegate _catalogCostDelegate;
    GoodFilterProxyModel _goodFilterModel;
    QString _defaultNameOnSupplierEditing;
    QString _supplierCode;
    QSqlRecord _supplierRecord;
    //const QList<Supplier>& _suppliers;
    //Supplier _supplier;
    //GoodCategory _currentCategory = GoodCategory::Skirts;

    void Init();
    void SetupUI();
    bool ErrorCheck() const; // В случае найденной ошибки возвращает false
};

#endif // SUPPLIERWINDOW_H
