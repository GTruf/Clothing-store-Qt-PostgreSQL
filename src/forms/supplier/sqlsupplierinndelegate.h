#ifndef SQLSUPPLIERINNDELEGATE_H
#define SQLSUPPLIERINNDELEGATE_H

#include <QStyledItemDelegate>

class SqlSupplierInnDelegate : public QStyledItemDelegate {
public:
    explicit SqlSupplierInnDelegate(QObject* parent = nullptr);

protected:
    QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QRegularExpressionValidator _validator;
};

#endif // SQLSUPPLIERINNDELEGATE_H
