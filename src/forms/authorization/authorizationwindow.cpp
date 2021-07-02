#include "authorizationwindow.h"
#include "ui_authorizationwindow.h"
#include <QMessageBox>
#include <QKeyEvent>

AuthorizationWindow::AuthorizationWindow(QWidget* parent) : QDialog(parent), ui(new Ui::Authorization) {
    ui->setupUi(this);
    SetupUI();
}

void AuthorizationWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return)
        on_signIN_clicked();

    QDialog::keyPressEvent(event);
}

UserType AuthorizationWindow::GetUserType() const {
    return _userType;
}

void AuthorizationWindow::ClearPassword() {
    ui->password->clear();
    ui->password->setFocus();
}

void AuthorizationWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->password, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->signIN, Qt::AlignCenter);
}

void AuthorizationWindow::on_signIN_clicked() {
    const auto password = ui->password->text();

    if (password == "директор") {
        _userType = UserType::Manager;
    }
    else if (password == "кассир") {
        _userType = UserType::Cashier;
    }
    else {
        QMessageBox::critical(this, "Ошибка авторизации", "Введен неверный пароль. Повторите попытку");
        ui->password->setFocus();
        return;
    }

    this->close();
    this->setResult(QDialog::Accepted);
}

AuthorizationWindow::~AuthorizationWindow() {
    delete ui;
}
