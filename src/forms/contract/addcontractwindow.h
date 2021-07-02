#ifndef ADDCONTRACTWINDOW_H
#define ADDCONTRACTWINDOW_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QPrinter>

namespace Ui {
class AddContractWindow;
}

class AddContractWindow : public QDialog {
    Q_OBJECT

public:
    explicit AddContractWindow(int contractNumber, QWidget* parent = nullptr);
    ~AddContractWindow();

private slots:
    void on_save_clicked();
    void on_print_clicked();

private:
    Ui::AddContractWindow *ui;
    QSqlQueryModel _comboSupplierModel;

    void Init();
    void SetupUI();
    void PaintContractPages(QPrinter* printer) const;
    bool ErrorCheck() const;
};

#endif // ADDCONTRACTWINDOW_H
