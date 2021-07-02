#ifndef SQLSUPPLIERPHONEDELEGATE_H
#define SQLSUPPLIERPHONEDELEGATE_H

#include "phonelineedit.h"
#include <QStyledItemDelegate>

class SqlSupplierPhoneDelegate : public QStyledItemDelegate {
public:
    explicit SqlSupplierPhoneDelegate(QObject* parent = nullptr);

protected:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif // SQLSUPPLIERPHONEDELEGATE_H
