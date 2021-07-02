#include "sqlsuppliercatalogcostdelegate.h"
#include <QLineEdit>

SqlSupplierCatalogCostDelegate::SqlSupplierCatalogCostDelegate(QObject* parent) : QStyledItemDelegate(parent),
    _numExpression("(^(0|[1-9][0-9]{0,9})(,[0-9]{0,2})?)?"), _validator(_numExpression)
{
}

QString SqlSupplierCatalogCostDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    auto text = QStyledItemDelegate::displayText(value, locale);

    if (!text.isEmpty()) {
        if (!text.contains(',')) // Если ввели, например, просто "5"
            text.append(",00");
        else if (text[text.size() - 1] == ',') // Если ввели, например, "5,"
            text.append("00");
        else if (text[text.size() - 2] == ',') // Если ввели, например, "5,5"
            text.append("0");

        if (!text.contains(locale.currencySymbol())) // При первом запуске знак рубля будет, но при редактировании будет пропадать
            text.append(" ").append(locale.currencySymbol());

        if (text == ("0,00 " + locale.currencySymbol())) // Если цена нулевая
            text = "1,00 " + locale.currencySymbol();
    }

    return text;
}

QWidget* SqlSupplierCatalogCostDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    QLineEdit* editor = new QLineEdit(parent);
    editor->setValidator(&_validator);
    return editor;
}

void SqlSupplierCatalogCostDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    auto strPrice = index.model()->data(index, Qt::EditRole).toString().simplified().remove(' ');
    auto price = _numExpression.match(strPrice).captured().replace(',', '.').toDouble();
    lineEdit->setText(QString::number(price, 'f', 2).replace('.', ','));
}

void SqlSupplierCatalogCostDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void SqlSupplierCatalogCostDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const {
    editor->setGeometry(option.rect);
}
