#include "reportwindow.h"
#include "ui_reportwindow.h"
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QSqlQuery>

using namespace std;

ReportWindow::ReportWindow(const ReportInfo& info, QWidget *parent) : QDialog(parent), ui(new Ui::ReportWindow) {
    ui->setupUi(this);
    DisplayReport(info);
    SetupUI();
}

void ReportWindow::SetupUI() {
    this->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);
    ui->verticalLayout->setAlignment(ui->reportCategoryGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->periodGroup, Qt::AlignCenter);
    ui->verticalLayout->setAlignment(ui->print, Qt::AlignCenter);

    SetupTables();
}

void ReportWindow::SetupTables() {
    QVector<QTableView*> tables = {ui->reportInfo};
    for (const auto& table : tables) {
        QFont font = table->horizontalHeader()->font();
        font.setBold(true);
        table->horizontalHeader()->setFont(font);
        table->setWordWrap(true);
        table->resizeColumnsToContents();
        connect(table->horizontalHeader(), &QHeaderView::sectionResized, table, &QTableView::resizeRowsToContents);
        table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter | static_cast<Qt::Alignment>(Qt::TextWordWrap));
        table->horizontalHeader()->setFixedHeight(50);
        connect(table->horizontalHeader(), &QHeaderView::sectionResized, table, &QTableView::resizeRowsToContents);
        table->horizontalHeader()->show();  // Почему-то при запуске иногда прячутся заголовки :/
        table->verticalHeader()->hide();    // Иногда появляются :/

        table->horizontalHeader()->resizeSection(0, 150); // "Наименование"
//        ui->reportInfo->horizontalHeader()->resizeSection(1, 200);
//        ui->reportInfo->horizontalHeader()->resizeSection(2, 20);
//        ui->reportInfo->horizontalHeader()->resizeSection(3, 20);
    }
}

void ReportWindow::DisplayReport(const ReportInfo& info) {
    ui->reportCategory->setText(info._reportCategory);
    ui->periodFrom->setText(info._periodFrom);
    ui->periodTo->setText(info._periodTo);

    QString reportQuery;

    if(ui->reportCategory->text() == "Продажи за период")
    {
        reportQuery = "SELECT  cat.description, cl_g.good_code, sup.name, "
                       "SUM(cl_g.number) AS n, SUM(cl_g.cost) AS sum "
                       "FROM  ordered_client_goods  AS  cl_g  "
                       "JOIN  catalog_sup  AS  cat  ON  good_code  =  cat.code "
                       "JOIN  stock  AS  st  ON  good_code  =  st.code "
                       "JOIN  suppliers  AS  sup  ON  cat.supplier_code  =  sup.code "
                       "JOIN  receipts_client  AS  rec  ON  rec.order_client_num  =  cl_g.order_client_num "
                       "WHERE rec.date BETWEEN '" + ui->periodFrom->text() + "' AND '" + ui->periodTo->text() + "' "
                       "GROUP BY  cl_g.good_code, cat.description,sup.name ORDER BY sum DESC;";
    } else
    {
        reportQuery = "SELECT  cat.description, ord_g.good_code, sup.name, SUM(ord_g.number) AS n, SUM(ord_g.cost) AS sum "
                       "FROM  ordered_sup_goods  AS  ord_g "
                       "JOIN  catalog_sup  AS  cat  ON  good_code  =  cat.code "
                       "JOIN  stock  AS  st  ON  good_code  =  st.code "
                       "JOIN  suppliers  AS  sup  ON  cat.supplier_code  =  sup.code "
                       "JOIN  receipts_sup  AS  rec  ON  rec.order_sup_num  =  ord_g.order_sup_num "
                       "WHERE rec.date BETWEEN '" + ui->periodFrom->text() + "' AND '" + ui->periodTo->text() + "' "
                       "GROUP BY  ord_g.good_code, cat.description, sup.name ORDER BY sum DESC;";
    }


    _sqlReportModel.setQuery(reportQuery);
    _sqlReportModel.setHeaderData(0, Qt::Horizontal, "Наименование");
    _sqlReportModel.setHeaderData(1, Qt::Horizontal, "Код товара");
    _sqlReportModel.setHeaderData(2, Qt::Horizontal, "Поставщик");
    _sqlReportModel.setHeaderData(3, Qt::Horizontal, "Кол-во");
    _sqlReportModel.setHeaderData(4, Qt::Horizontal, "Сумма");

    ui->reportInfo->setModel(&_sqlReportModel);
}

void ReportWindow::PrintTableHeader (QPainter& painter) {
    constexpr int pageWidth = 727 - 20;
    constexpr int pageHeight = 1071 - 20;
    QRect rect (20, 20, pageWidth, 0.1 * pageHeight);

    QFont font = painter.font();
    font.setPixelSize(22);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, "Отчет: " + ui->reportCategory->text() +
        "\nс " + ui->periodFrom->text() + " по " + ui->periodTo->text());

    font.setPixelSize(14);
    font.setBold(true);
    painter.setFont(font);

    rect.setTop(0.12 * pageHeight);
    rect.setHeight(0.04 * pageHeight);

    rect.setLeft(20);
    rect.setWidth(350);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, "Наименование");

    rect.setLeft(370);
    rect.setWidth(80);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, "Код товара");

    rect.setLeft(450);
    rect.setWidth(150);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, "Поставщик");

    rect.setLeft(600);
    rect.setWidth(50);
    painter.drawText(rect, Qt::AlignCenter, "Кол-во");

    rect.setLeft(650);
    rect.setWidth(90);
    painter.drawText(rect, Qt::AlignCenter, "Сумма");

    font.setBold(false);
    painter.setFont(font);
}

void ReportWindow::on_print_clicked() {
    QPrinter printer;
    QPrintPreviewDialog printWindow(&printer, nullptr, Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    printWindow.setWindowTitle("Просмотр печати отчета");
    printWindow.resize(1000, 600);

    connect(&printWindow, &QPrintPreviewDialog::paintRequested, this, [=](QPrinter* printer) {

    QPainter painter;
    painter.begin(printer);

    int pageWidth = printer->pageLayout().paintRectPixels(printer->resolution()).width() - 20;
    int pageHeight = printer->pageLayout().paintRectPixels(printer->resolution()).height() - 20;
    QRect rect (20, 20, pageWidth, 0.1 * pageHeight);

    PrintTableHeader(painter);

    QFont font = painter.font();
    font.setPixelSize(14);
    painter.setFont(font);

    QPen pen = painter.pen();
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);

    int top = 0.12 * pageHeight;
    int rows = _sqlReportModel.rowCount();

    for(int i = 0; i < rows; i++) {
        top += 0.04 * pageHeight;

        if(top > pageHeight - 20 ){
            printer->newPage();
            PrintTableHeader(painter);
            top = 0.16 * pageHeight;
        }

        rect.setTop(top);
        rect.setHeight(0.04 * pageHeight);
        rect.setLeft(20);
        rect.setWidth(350);//наименование
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, _sqlReportModel.index(i, 0).data().toString());

        rect.setLeft(370);
        rect.setWidth(80);//код
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, _sqlReportModel.index(i, 1).data().toString());

        rect.setLeft(450);
        rect.setWidth(150);//поставщик
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, _sqlReportModel.index(i, 2).data().toString());

        rect.setLeft(600);
        rect.setWidth(50);//кол-во
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, _sqlReportModel.index(i, 3).data().toString());

        rect.setLeft(650);
        rect.setWidth(90);//сумма
        painter.drawRect(rect.adjusted(0, 0, -pen.width(), -pen.width()));
        painter.drawText(rect, Qt::AlignCenter, _sqlReportModel.index(i, 4).data().toString());
    }});

    printWindow.exec();
}

ReportWindow::~ReportWindow() {
    emit finished(result());
    delete ui;
}
