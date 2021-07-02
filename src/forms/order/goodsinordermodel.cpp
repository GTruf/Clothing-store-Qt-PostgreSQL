#include "goodsinordermodel.h"
#include <QFont>

GoodsInOrderModel::GoodsInOrderModel(QObject* parent) : QAbstractTableModel(parent) {
}

int GoodsInOrderModel::FindAndUpdateGoodInfo(const QString& goodCode, int quantity, double cost) {
    // Можно было бы вызвать FindGood и UpdateGoodInfo, но здесь мы уже имеем итератор, поэтому сразу можно по нему менять данные
    auto existGoodInOrder = std::find_if(_goodsInOrder.begin(), _goodsInOrder.end(), [=](const Good& good) {
        return good.code == goodCode;
    });

    if (existGoodInOrder != _goodsInOrder.end()) {
        auto currentQuantity = existGoodInOrder->quantity.toInt();
        auto currentCost = existGoodInOrder->totalCost.toDouble();
        existGoodInOrder->quantity = QString::number(currentQuantity + quantity);
        existGoodInOrder->totalCost = QString::number(currentCost + cost, 'f', 2);
        auto row = std::distance(_goodsInOrder.begin(), existGoodInOrder);
        return row;
    }

    return -1;
}


int GoodsInOrderModel::FindGood(const QString& goodCode) const {
    auto existGoodInOrder = std::find_if(_goodsInOrder.begin(), _goodsInOrder.end(), [=](const Good& good) {
        return good.code == goodCode;
    });

    if (existGoodInOrder != _goodsInOrder.end()) {
        auto row = std::distance(_goodsInOrder.begin(), existGoodInOrder);
        return row;
    }

    return -1;
}

void GoodsInOrderModel::UpdateGoodInfo(int row, int quantity, double cost) {
    auto& good = _goodsInOrder[row];
    auto currentQuantity = good.quantity.toInt();
    auto currentCost = good.totalCost.toDouble();
    good.quantity = QString::number(currentQuantity + quantity);
    good.totalCost = QString::number(currentCost + cost, 'f', 2);
}

int GoodsInOrderModel::rowCount(const QModelIndex&) const {
    return _goodsInOrder.size();
}

int GoodsInOrderModel::columnCount(const QModelIndex&) const {
    return 4;
}

QVariant GoodsInOrderModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 ||  index.row()  >= _goodsInOrder.size())
        return QVariant();

    const Good& goodInfo = _goodsInOrder.at(index.row());

    switch(role) {
        case Qt::DisplayRole:
            if (index.column() == 0)
                return goodInfo.description;
            else if (index.column() == 1)
                return goodInfo.code;
            else if (index.column() == 2)
                return goodInfo.quantity;
            else
                return goodInfo.totalCost;
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
       default:
            return QVariant();
    }
}

QVariant GoodsInOrderModel::headerData(int nSection, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Vertical || (role != Qt::DisplayRole && role != Qt::FontRole))
        return QVariant();

    if (role == Qt::FontRole) {
        QFont font;
        font.setBold(true);
        return font;
    }

    switch(nSection) {
        case 0: return "Наименование";
        case 1: return "Артикул";
        case 2: return "Количество";
        case 3: return "Стоимость";
        default:
            return QVariant();
    }
}

bool GoodsInOrderModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        switch(index.column()) {
            case 0: _goodsInOrder[index.row()].description = value.toString();
            break;
            case 1: _goodsInOrder[index.row()].code = value.toString();
            break;
            case 2: _goodsInOrder[index.row()].quantity = value.toString();
            break;
            case 3: _goodsInOrder[index.row()].totalCost = value.toString();
            break;
        }

        emit dataChanged(index, index);
        return true;
    }

    return false;
}

bool GoodsInOrderModel::insertRows(int row, int count, const QModelIndex& parent) {
   if (parent.isValid()){
        return false;
   }

   beginInsertRows(QModelIndex(), row,  row + count - 1);
   for (int i = 0;  i  < count; ++i) {
       _goodsInOrder.insert(row, Good());
   }
   endInsertRows();
   return true;
}

bool GoodsInOrderModel::removeRows(int row, int count, const QModelIndex& parent) {
   if (parent.isValid() || _goodsInOrder.isEmpty())
       return false;

   beginRemoveRows(parent, row, row + count - 1);
   for (int i = 0; i < count; ++i) {
       _goodsInOrder.removeAt(row);
   }
   endRemoveRows();
   return true;
}
