#ifndef VIEWCONTRACTWINDOW_H
#define VIEWCONTRACTWINDOW_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QPrinter>

namespace Ui {
class ViewContractWindow;
}

class ViewContractWindow : public QDialog {
    Q_OBJECT

public:
    explicit ViewContractWindow(const QString& contractNumber, QWidget* parent = nullptr);
    ~ViewContractWindow();

private slots:
    void on_print_clicked();

private:
    Ui::ViewContractWindow *ui;
    QSqlQueryModel _contractModel;
    mutable QString _supplierCode;

    void SetupUI();
    void DisplayContractInfo(const QString& contractNumber) const;
    void PaintContractPages(QPrinter* printer) const;
};

#endif // VIEWCONTRACTWINDOW_H
