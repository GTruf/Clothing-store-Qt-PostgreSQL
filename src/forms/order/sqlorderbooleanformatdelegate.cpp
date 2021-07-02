#include "sqlorderbooleanformatdelegate.h"
#include <QApplication>
#include <QPainter>

SqlOrderBooleanFormatDelegate::SqlOrderBooleanFormatDelegate(QObject* parent) : QItemDelegate(parent) {
}

void SqlOrderBooleanFormatDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    const QString text = index.data().toString() == "true" ? "✔" : "✖";
    painter->save();
    painter->setPen(QColor(text == "✔" ? Qt::darkGreen : Qt::darkRed));
    QStyle* style = QApplication::style();

    auto newOption = option;
    newOption.palette.setColor(QPalette::Window, "#e9f3fb");

    if (newOption.state & QStyle::State_Selected) {
        painter->setBrush(QBrush("#e9f3fb"));
    }


    QRect textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &newOption);
    painter->drawText(textRect, Qt::AlignCenter, text);
    painter->restore();
}
