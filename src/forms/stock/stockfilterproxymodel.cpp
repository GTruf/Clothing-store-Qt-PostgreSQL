#include "stockfilterproxymodel.h"

StockFilterProxyModel::StockFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
}

bool StockFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    auto index = sourceModel()->index(sourceRow, 0);  // Столбец с категориями
    auto data = sourceModel()->data(index);
    return _category == "Все" ? true : data.toString().contains(_category);
}

void StockFilterProxyModel::SetCategoryFilter(QString category) {
    _category = std::move(category);
    invalidate();
}

