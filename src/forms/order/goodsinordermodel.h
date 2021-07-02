#ifndef GOODSINORDERMODEL_H
#define GOODSINORDERMODEL_H

#include <QAbstractTableModel>

// Поскольку все QSql модели в Qt при удалении не прячут строку (нужно вызывать явно hideRow), проще написать свою несложную модель
class GoodsInOrderModel  : public QAbstractTableModel {
public:
    explicit GoodsInOrderModel(QObject* parent = nullptr);
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int nSection, Qt::Orientation orientation, int role) const override;

    int FindGood(const QString& goodCode) const;
    int FindAndUpdateGoodInfo(const QString& goodCode, int quantity, double cost);
    void UpdateGoodInfo(int row, int quantity, double cost);

private:
    struct Good {
        QString description, code, quantity, totalCost;
    };

    QList<Good> _goodsInOrder;
};

#endif // GOODSINORDERMODEL_H

