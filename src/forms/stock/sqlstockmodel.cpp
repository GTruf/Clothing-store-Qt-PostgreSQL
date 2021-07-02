#include "sqlstockmodel.h"
#include <QSqlQuery>
#include <QSqlRecord>

SqlStockModel::SqlStockModel(QObject* parent) : QSqlQueryModel(parent){
}

bool SqlStockModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        QSqlQuery updateStock;

        if (index.column() == 3) { // "Цена" - обновляется у менеджера
            updateStock.prepare("UPDATE stock SET price = :price WHERE code = '" + data(this->index(index.row(), 1)).toString() + "'");
            updateStock.bindValue(":price", value);
        }
        else if (index.column() == 4) { // "Кол-во" - обновляется у кассира
            updateStock.prepare("UPDATE stock SET number = :number WHERE code = '" + data(this->index(index.row(), 1)).toString() + "'");
            updateStock.bindValue(":number", value);
        }

        auto ok = updateStock.exec();
        Refresh();
        return ok;
    }

    return false;
}

void SqlStockModel::Refresh() {
    setQuery("SELECT cat.name, st.code, cat.description, st.price, st.number "
             "FROM stock AS st "
             "JOIN catalog_sup AS cat ON st.code = cat.code "
             "WHERE st.number > 0 "
             "ORDER BY st.number DESC");
//    setHeaderData(1, Qt::Horizontal, "Код товара");
//    setHeaderData(2, Qt::Horizontal, "Наименование");
//    setHeaderData(3, Qt::Horizontal, "Цена");
//    setHeaderData(4, Qt::Horizontal, "Количество");
}

Qt::ItemFlags SqlStockModel::flags(const QModelIndex& index) const {
    auto flags = QSqlQueryModel::flags(index);
    if (index.column() == 3)
        flags |= Qt::ItemIsEditable; // Работает только у менеджера, потому что для QTableView включён DoubleClicked флаг, у кассира нет
    return flags;

}

