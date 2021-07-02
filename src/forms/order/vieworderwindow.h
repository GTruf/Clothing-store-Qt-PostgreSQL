#ifndef VIEWORDERWINDOW_H
#define VIEWORDERWINDOW_H

#include "goodsinordermodel.h"
#include <QDialog>
#include <QSqlQueryModel>

namespace Ui {
class ViewOrderWindow;
}

struct ViewOrderInfo {
    QString supplierName, contractNum, registrationDate, paid, sent, supplierCode;
};

class ViewOrderWindow : public QDialog {
    Q_OBJECT

public:
    explicit ViewOrderWindow(QString orderNumber, ViewOrderInfo orderInfo, QWidget* parent = nullptr);
    ~ViewOrderWindow();

private slots:
    void on_ok_clicked();
    void on_printInvoice_clicked();
    void on_printWaybill_clicked();

private:
    Ui::ViewOrderWindow *ui;
    QString _orderNumber;
    ViewOrderInfo _orderInfo;
    QSqlQueryModel _goodsInOrderModel;

    void SetupUI();
    void SetupTables();
    void DisplayOrderInfo();
};

#endif // VIEWORDERWINDOW_H
