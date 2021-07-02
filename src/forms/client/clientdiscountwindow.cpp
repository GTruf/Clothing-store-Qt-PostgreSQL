#include "clientdiscountwindow.h"
#include "ui_clientdiscountwindow.h"
#include <QPushButton>
#include <QMessageBox>
#include <QSqlQuery>
#include <QException>

ClientDiscountWindow::ClientDiscountWindow(QString totalCostWithoutDiscount, QWidget *parent) : QDialog(parent), ui(new Ui::ClientDiscountWindow),
    _cardValidator(QRegularExpression("[1-9]\\d{9}")), _totalCostWithoutDiscount(std::move(totalCostWithoutDiscount))
{
    ui->setupUi(this);
    SetupUI();
    InitConnects();
}

void ClientDiscountWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Ок");
    layout()->setAlignment(ui->apply, Qt::AlignCenter);
    ui->card->setValidator(&_cardValidator);
    ui->totalCostWithoutDiscount->setText(_totalCostWithoutDiscount + QLocale::system().currencySymbol().prepend(" "));
}

void ClientDiscountWindow::InitConnects() {
    connect(ui->phone, &QLineEdit::textChanged, this, [=]() {
        ui->card->clear();
    });

    connect(ui->card, &QLineEdit::textEdited, this, [=](const QString& text) {
        ui->phone->Clear();
        ui->card->setText(text); // Почему-то без setText первый символ пропускается
    });
}

QString ClientDiscountWindow::GetTotalCostWithDiscount() const {
    return _totalCostWithDiscount;
}

QString ClientDiscountWindow::GetFullName() const {
    return ui->fullName->text();
}

QString ClientDiscountWindow::GetClientCode() const {
    return _clientCode;
}

void ClientDiscountWindow::accept() {
    // Проверка _totalCostWithDiscount.isEmpty() означает, что окно закрыли без применения карты какого-либо клиента
    _totalCostWithDiscount.isEmpty() ? QDialog::reject() : QDialog::accept();
}

void ClientDiscountWindow::on_apply_clicked() {
    if (!ErrorCheck())
        return;

    try {
        auto [discount, fullName] = !ui->phone->text().isEmpty() ? DiscountByPhone() : DiscountByCard();
        HandleDiscount(std::move(discount), std::move(fullName));
        setFocus(); // После нажатия на "Применить" теряется фокус с окна и перестаёт работать Enter, пока не вернётся фокус
    }
    catch (const std::runtime_error& message) {
        QMessageBox::critical(nullptr, "Ошибка применения скидки", message.what());
        return;
    }
}

void ClientDiscountWindow::HandleDiscount(QString discount, QString fullName) {
    _totalCostWithDiscount = QString::number(
                _totalCostWithoutDiscount.toDouble() - (_totalCostWithoutDiscount.toDouble() * discount.toDouble() / 100), 'f', 2);
    ui->totalCostWithDiscount->setText(_totalCostWithDiscount + QLocale::system().currencySymbol().prepend(" "));
    ui->fullName->setText(fullName);
    ui->discount->setText(discount.replace(".", ",").append("%"));
}

bool ClientDiscountWindow::ErrorCheck() const {
    if (ui->phone->text().isEmpty() && ui->card->text().isEmpty()) {
        QMessageBox::critical(nullptr, "Ошибка применения скидки", "Введите номер телефона или карту клиента");
        ui->phone->setFocus();
        return false;
    }

    if (!ui->phone->text().isEmpty() && !ui->phone->IsValid()) {
        QMessageBox::critical(nullptr, "Ошибка применения скидки", "Неверно введён номер телефона");
        ui->phone->setFocus();
        return false;
    }

    if (!ui->card->text().isEmpty() && ui->card->text().count() < 10) {
        QMessageBox::critical(nullptr, "Ошибка применения скидки", "Карта клиента должна иметь 10 цифр");
        ui->card->setFocus();
        return false;
    }

    return true;
}

QPair<QString, QString> ClientDiscountWindow::DiscountByPhone() {
    QSqlQuery discountByPhone("SELECT discount, name, code FROM clients WHERE phone = '" + ui->phone->text() + "'");
    if (!discountByPhone.next())
        throw std::runtime_error(QString("Клиент с телефоном \"" + ui->phone->text() + "\" не найден").toStdString());
    _clientCode = discountByPhone.value(2).toString();
    return {discountByPhone.value(0).toString(), discountByPhone.value(1).toString()};
}

QPair<QString, QString> ClientDiscountWindow::DiscountByCard() {
    QSqlQuery discountByCard("SELECT discount, name, code FROM clients WHERE card = '" + ui->card->text() + "'");
    if (!discountByCard.next())
        throw std::runtime_error(QString("Клиент с картой \"" + ui->card->text() + "\" не найден").toStdString());
    _clientCode = discountByCard.value(2).toString();
    return {discountByCard.value(0).toString(), discountByCard.value(1).toString()};
}

ClientDiscountWindow::~ClientDiscountWindow() {
    delete ui;
}
