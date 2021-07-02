#ifndef CLIENTDISCOUNTDELEGATE_H
#define CLIENTDISCOUNTDELEGATE_H

#include <QStyledItemDelegate>

class ClientDiscountDelegate : public QStyledItemDelegate {
public:
    ClientDiscountDelegate(QObject* parent = nullptr);

protected:
    QString displayText(const QVariant& value, const QLocale& locale) const override;
};


#endif // CLIENTDISCOUNTDELEGATE_H
