#include "phonelineedit.h"
#include <QMouseEvent>
#include <QFocusEvent>

PhoneLineEdit::PhoneLineEdit(QWidget* parent) : QLineEdit(parent), _phoneMask("+7(D00)‒000‒00‒00;_"), _emptyPhone("+7()‒‒‒") {
}

void PhoneLineEdit::Clear() {
    setInputMask("");
}

bool PhoneLineEdit::IsValid() const {
    return text().length() == maxLength();
}

bool PhoneLineEdit::IsValid(const QString& phone) {
    static PhoneLineEdit phoneChecker;
    if (phoneChecker.inputMask().isEmpty())
        phoneChecker.setInputMask("+7(D00)‒000‒00‒00;_");
    phoneChecker.setText(phone);
    return phoneChecker.IsValid();
}

void PhoneLineEdit::mousePressEvent(QMouseEvent* event) {
    if (text().isEmpty()) {
        setInputMask(_phoneMask);
    }

    QLineEdit::mousePressEvent(event);

    if (text() == _emptyPhone) { // Тут не получится избавиться от подобной проверки, потому что если установить позицию курсора в верхнем
        setCursorPosition(3);    // блоке "if", то это не сработает из-за последующего вызова QLineEdit::mousePressEvent
    }
}

void PhoneLineEdit::focusOutEvent(QFocusEvent* event) {
    if (text() == _emptyPhone && event->reason() != Qt::FocusReason::ActiveWindowFocusReason &&
            event->reason() != Qt::FocusReason::PopupFocusReason) // Если свернули окно или нажали на пкм (всплывающее меню), текст не сотрётся
    {
        Clear();
    }

    QLineEdit::focusOutEvent(event);
}

void PhoneLineEdit::focusInEvent(QFocusEvent* event) {
    if (text().isEmpty()) {
        setInputMask(_phoneMask);
    }

    QLineEdit::focusInEvent(event);

    if (text() == _emptyPhone) { // Тут не получится избавиться от подобной проверки, потому что если установить позицию курсора в верхнем
        setCursorPosition(3);    // блоке "if", то это не сработает из-за последующего вызова QLineEdit::mousePressEvent
    }
}

void PhoneLineEdit::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_0 && cursorPosition() <= 3) // Чтобы нельзя было ввести +7(0**)... (первый символ 0 в круглых скобках)
        return;                 // Маска тоже не позволит ввести, но она перенесёт тогда курсор на следующий символ и вставит 0 туда

    QLineEdit::keyPressEvent(event);
}
