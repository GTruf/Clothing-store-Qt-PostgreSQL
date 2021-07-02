#include "passwordlineedit.h"

PasswordLineEdit::PasswordLineEdit(QWidget* parent) : QLineEdit(parent), _eyeOn(":icons/icons/eyeOn.svg"), _eyeOff(":icons/icons/eyeOff.svg") {
    setEchoMode(QLineEdit::Password);
    addAction(QIcon(":icons/icons/password.svg"), QLineEdit::LeadingPosition);

    QAction* revealHideAction = addAction(_eyeOff, QLineEdit::TrailingPosition);
    _revealHideButton = qobject_cast<QToolButton*>(revealHideAction->associatedWidgets().last());
    _revealHideButton->setCursor(QCursor(Qt::PointingHandCursor));

    connect(_revealHideButton, &QToolButton::clicked, this, [=]() {
        if (echoMode() == QLineEdit::Password) {
            _revealHideButton->setIcon(_eyeOn);
            setEchoMode(QLineEdit::Normal);
        }
        else {
            _revealHideButton->setIcon(_eyeOff);
            setEchoMode(QLineEdit::Password);
        }
    });
}
