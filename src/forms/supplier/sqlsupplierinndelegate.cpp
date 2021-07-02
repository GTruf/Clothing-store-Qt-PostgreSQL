#include "sqlsupplierinndelegate.h"
#include <QLineEdit>

SqlSupplierInnDelegate::SqlSupplierInnDelegate(QObject* parent) : QStyledItemDelegate(parent),
    _validator(QRegularExpression("\\d{0,10}"))
{
}

QString SqlSupplierInnDelegate::displayText(const QVariant& value, const QLocale& locale) const {
    switch(static_cast<QMetaType::Type>(value.type())){
        case QMetaType::Float:
            return locale.toString(value.toFloat(), 'f', 0);
        case QMetaType::Double:
            return locale.toString(value.toDouble(), 'f', 0);
        default:
            return QStyledItemDelegate::displayText(value, locale);
    }
}

QWidget* SqlSupplierInnDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    QLineEdit* editor = new QLineEdit(parent);
    editor->setValidator(&_validator);
    return editor;
}

void SqlSupplierInnDelegate::setEditorData(QWidget* editor, const QModelIndex &index) const {
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void SqlSupplierInnDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void SqlSupplierInnDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const {
    editor->setGeometry(option.rect);
}

