#ifndef REPORTWINDOW_H
#define REPORTWINDOW_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QPrinter>

namespace Ui {
class ReportWindow;
}

struct ReportInfo {
    QString _reportCategory, _periodFrom, _periodTo;
};

class ReportWindow : public QDialog {
    Q_OBJECT

public:
    explicit ReportWindow(const ReportInfo& info, QWidget *parent = nullptr);
    ~ReportWindow();

private slots:
    void on_print_clicked();

private:
    Ui::ReportWindow *ui;
    QSqlQueryModel _sqlReportModel;

    void SetupUI();
    void SetupTables();
    void DisplayReport(const ReportInfo& info);
    void PrintTableHeader(QPainter& painter);
};

#endif // REPORTWINDOW_H
