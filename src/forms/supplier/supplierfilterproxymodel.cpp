#include "supplierfilterproxymodel.h"

SupplierFilterProxyModel::SupplierFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent), _category("Все") {
}

bool SupplierFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    auto index = sourceModel()->index(sourceRow, 2);  // Столбец с категориями
    auto data = sourceModel()->data(index);
    return _category == "Все" ? true : data.toString().contains(_category);
}

void SupplierFilterProxyModel::SetCategoryFilter(QString category) {
    _category = std::move(category);
    invalidate();
}
