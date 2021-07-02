#ifndef CLIENTDISCOUNTWINDOW_H
#define CLIENTDISCOUNTWINDOW_H

#include <QDialog>
#include <QRegularExpressionValidator>

namespace Ui {
class ClientDiscountWindow;
}

class ClientDiscountWindow : public QDialog {
    Q_OBJECT

public:
    explicit ClientDiscountWindow(QString totalCostWithoutDiscount, QWidget *parent = nullptr);
    ~ClientDiscountWindow();
    QString GetTotalCostWithDiscount() const;
    QString GetFullName() const;
    QString GetClientCode() const;

protected:
    void accept() override;

private slots:
    void on_apply_clicked();

private:
    Ui::ClientDiscountWindow *ui;
    QRegularExpressionValidator _cardValidator;
    QString _totalCostWithoutDiscount, _totalCostWithDiscount, _clientCode;

    void SetupUI();
    void InitConnects();
    void HandleDiscount(QString discount, QString fullName);
    bool ErrorCheck() const;
    QPair<QString, QString> DiscountByPhone();
    QPair<QString, QString> DiscountByCard();
};

#endif // CLIENTDISCOUNTWINDOW_H
