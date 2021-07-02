#ifndef STOCKFILTERPROXYMODEL_H
#define STOCKFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class StockFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit StockFilterProxyModel(QObject* parent = nullptr);
    void SetCategoryFilter(QString category);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    QString _category;
};

#endif // STOCKFILTERPROXYMODEL_H
