#ifndef ORDERFILTERPROXYMODEL_H
#define ORDERFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class OrderFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit OrderFilterProxyModel(QObject* parent = nullptr);
    void SetUsePaidFilter(bool on);
    void SetUseDeliveredFilter(bool on);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    bool _isUsePaidFilter = true, _isUseDeliveredFilter = false;
};

#endif // ORDERFILTERPROXYMODEL_H
