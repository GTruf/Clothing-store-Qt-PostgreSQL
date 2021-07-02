#ifndef CONTRACTFILTERPROXYMODEL_H
#define CONTRACTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class ContractFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit ContractFilterProxyModel(QObject* parent = nullptr);
    void SetUseExpiringFilter(bool on);
    void SetUseActingFilter(bool on);
    void SetUseArchivalFilter(bool on);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    bool _isUseExpiringFilter = false, _isUseActingFilter = true, _isUseArchivalFilter = false;
};

#endif // CONTRACTFILTERPROXYMODEL_H
