#include "goodfilterproxymodel.h"

GoodFilterProxyModel::GoodFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {

}

bool GoodFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    auto categoryIndex = sourceModel()->index(sourceRow, 1);  // Столбец с категориями
    auto data = sourceModel()->data(categoryIndex);
    return _category == data.toString();
}

void GoodFilterProxyModel::SetCategoryFilter(QString category) {
    _category = category;
    invalidate();
}

