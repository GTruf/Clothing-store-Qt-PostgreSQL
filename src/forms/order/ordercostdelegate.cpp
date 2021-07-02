#include "ordercostdelegate.h"

OrderCostDelegate::OrderCostDelegate(QObject* parent)  : QStyledItemDelegate(parent) {
}

QString OrderCostDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    return locale.toString(value.toDouble(), 'f', 2) + " " + locale.currencySymbol();
}
