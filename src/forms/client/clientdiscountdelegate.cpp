#include "clientdiscountdelegate.h"

ClientDiscountDelegate::ClientDiscountDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QString ClientDiscountDelegate::displayText(const QVariant& value, const QLocale&) const {
    return value.toString().replace(".", ",").append("%");
}
