#ifndef GOODFILTERPROXYMODEL_H
#define GOODFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class GoodFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit GoodFilterProxyModel(QObject* parent = nullptr);
    void SetCategoryFilter(QString category);
    void SetCategoryColumn(int column);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;

private:
    QString _category;
};

#endif // GOODFILTERPROXYMODEL_H
