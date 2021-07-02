#ifndef SQLORDERBOOLEANFORMATDELEGATE_H
#define SQLORDERBOOLEANFORMATDELEGATE_H

#include <QItemDelegate>

class SqlOrderBooleanFormatDelegate : public QItemDelegate {
public:
    explicit SqlOrderBooleanFormatDelegate(QObject* parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // SQLORDERBOOLEANFORMATDELEGATE_H
