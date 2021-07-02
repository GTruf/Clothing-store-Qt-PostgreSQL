#ifndef ORDERCOSTDELEGATE_H
#define ORDERCOSTDELEGATE_H

#include <QStyledItemDelegate>

class OrderCostDelegate : public QStyledItemDelegate {
public:
    OrderCostDelegate(QObject* parent = nullptr);

protected:
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};

#endif // ORDERCOSTDELEGATE_H
