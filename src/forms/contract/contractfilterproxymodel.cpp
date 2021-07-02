#include "contractfilterproxymodel.h"
#include <QDate>

ContractFilterProxyModel::ContractFilterProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
}

bool ContractFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex&) const {
    auto completionDate = sourceModel()->index(sourceRow, 3).data().toString();  // "Дата окончания"

    // Количество оставшихся дней у договора
    auto daysRemain = QDate::currentDate().daysTo(QDate::fromString(completionDate, "yyyy-MM-dd")); // Такой формат возвращает модель

    return ((_isUseExpiringFilter && daysRemain >= 0 && daysRemain < 3) || (_isUseActingFilter && daysRemain >= 0) ||
            (_isUseArchivalFilter && daysRemain < 0));
}

void ContractFilterProxyModel::SetUseExpiringFilter(bool on) {
    _isUseExpiringFilter = on;
    invalidate();  // Сброс фильтров и вызов filterAcceptsRow
}

void ContractFilterProxyModel::SetUseActingFilter(bool on) {
    _isUseActingFilter = on;
    invalidate();
}

void ContractFilterProxyModel::SetUseArchivalFilter(bool on) {
    _isUseArchivalFilter = on;
    invalidate();
}
