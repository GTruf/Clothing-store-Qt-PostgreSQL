#ifndef SQLSUPPLIERCATALOGCOSTDELEGATE_H
#define SQLSUPPLIERCATALOGCOSTDELEGATE_H

#include <QStyledItemDelegate>

class SqlSupplierCatalogCostDelegate : public QStyledItemDelegate {
public:
    explicit SqlSupplierCatalogCostDelegate(QObject* parent = nullptr);

protected:
    QString displayText(const QVariant& value, const QLocale& locale) const override;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QRegularExpression _numExpression;
    QRegularExpressionValidator _validator;
};

#endif // SQLSUPPLIERCATALOGCOSTDELEGATE_H
