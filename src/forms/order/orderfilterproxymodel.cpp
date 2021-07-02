#include "orderfilterproxymodel.h"

OrderFilterProxyModel::OrderFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
}

bool OrderFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    auto paidIndex = sourceModel()->index(sourceRow, 4);  // "Оплачен"
    auto deliveredIndex = sourceModel()->index(sourceRow, 5);  // "Доставлен"
    auto isPaid = sourceModel()->data(paidIndex) == "✔";
    auto isDelivered = sourceModel()->data(deliveredIndex) == "✔";

    if (_isUsePaidFilter && _isUseDeliveredFilter)
        return isPaid && isDelivered;
    else if (!_isUsePaidFilter && _isUseDeliveredFilter)
        return !isPaid && isDelivered;
    else if (_isUsePaidFilter && !_isUseDeliveredFilter)
        return isPaid && !isDelivered;
    else
        return !isPaid && !isDelivered;
}

void OrderFilterProxyModel::SetUsePaidFilter(bool on) {
    _isUsePaidFilter = on;
    invalidate();
}

void OrderFilterProxyModel::SetUseDeliveredFilter(bool on) {
    _isUseDeliveredFilter = on;
    invalidate();
}
