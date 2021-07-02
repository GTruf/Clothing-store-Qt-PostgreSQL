#ifndef AUTHORIZATIONWINDOW_H
#define AUTHORIZATIONWINDOW_H

#include <QDialog>

namespace Ui {
class Authorization;
}

enum class UserType {
    Manager,
    Cashier
};

class AuthorizationWindow : public QDialog {
    Q_OBJECT

public:
    explicit AuthorizationWindow(QWidget* parent = nullptr);
    ~AuthorizationWindow();
    void ClearPassword();
    UserType GetUserType() const;

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void on_signIN_clicked();

private:
    Ui::Authorization *ui;
    UserType _userType;

    void SetupUI();
};

#endif // AUTHORIZATIONWINDOW_H
