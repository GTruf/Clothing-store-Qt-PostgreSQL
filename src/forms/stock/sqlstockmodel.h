#ifndef SQLSTOCKMODEL_H
#define SQLSTOCKMODEL_H

#include <QSqlQueryModel>

class SqlStockModel : public QSqlQueryModel {
public:
    explicit SqlStockModel(QObject* parent = nullptr);

protected:
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    void Refresh();
};

#endif // SQLSTOCKMODEL_H
