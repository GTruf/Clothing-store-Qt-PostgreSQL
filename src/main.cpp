#include "forms/authorization/authorizationwindow.h"
#include "directormainwindow.h"
#include "cashiermainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSqlQuery>
#include <QMessageBox>

void TryCreateConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("clothing_store");
    db.setUserName("postgres");
    db.setPassword("postgres");
    if (!db.open()) {
        QMessageBox::critical(nullptr, "Ошибка подключения к базе данных",
                              "Не удалось подключиться к базе данных, обратитесь к системному администратору");
        exit(1);
    }

    QSqlQuery lcMonetary("SET lc_monetary to 'ru_RU.utf8'"); // Для отображения знака рубля в базе данных
    Q_UNUSED(lcMonetary);
}

// Вариант работы программы с "кэшем" (окна создаются один раз и, соответственно, первоначальная прогрузка базы данных тоже), но
// при этом оперативка забита двумя окнами и базой данных
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    TryCreateConnection();

    AuthorizationWindow aw;
    DirectorMainWindow managerMainWindow;
    CashierMainWindow cashierMainWindow;

    while (aw.exec() == QDialog::Accepted) {
        if (aw.GetUserType() == UserType::Manager) {
            managerMainWindow.UpdateStock();
            managerMainWindow.showMaximized();
        }
        else if (aw.GetUserType() == UserType::Cashier) {
            cashierMainWindow.UpdateStock();
            cashierMainWindow.showMaximized();
        }

        a.exec();
        aw.ClearPassword();
    }

    return 0;
}

/*
// Вариант работы программы без "кэша" (окна пересоздаются каждый раз при их открытии, и, соответственно, база данных прогружается тоже
// каждый раз при их открытии), но при этом оперативка не забивается двумя окнами и базой данных
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    TryCreateConnection();

    AuthorizationWindow aw;
    std::unique_ptr<QMainWindow> mainWindow;

    while (aw.exec() == QDialog::Accepted)
    {
        if (aw.GetUserType() == UserType::Manager) {
            mainWindow = std::make_unique<ManagerMainWindow>();
        }
        else if (aw.GetUserType() == UserType::Cashier) {
            mainWindow = std::make_unique<CashierMainWindow>();
        }

        mainWindow->showMaximized();
        a.exec();
        aw.ClearPassword();
    }

    return 0;
}
*/

