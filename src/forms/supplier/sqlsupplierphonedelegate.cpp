#include "sqlsupplierphonedelegate.h"

SqlSupplierPhoneDelegate::SqlSupplierPhoneDelegate(QObject* parent) : QStyledItemDelegate(parent) {
}

QWidget* SqlSupplierPhoneDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const {
    PhoneLineEdit* editor = new PhoneLineEdit(parent);
    return editor;
}

void SqlSupplierPhoneDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    PhoneLineEdit* lineEdit = qobject_cast<PhoneLineEdit*>(editor);

    // При клике по полю в таблице, когда там уже есть данные, маска пустая, поэтому её нужно установить явно
    lineEdit->setInputMask("+7(D00)‒000‒00‒00;_");
    lineEdit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void SqlSupplierPhoneDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const {
    editor->setGeometry(option.rect);
}
