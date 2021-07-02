#ifndef SUPPLIERFILTERPROXYMODEL_H
#define SUPPLIERFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class SupplierFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit SupplierFilterProxyModel(QObject* parent = nullptr);
    void SetCategoryFilter(QString category);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    QString _category;
};

#endif // SUPPLIERFILTERPROXYMODEL_H
