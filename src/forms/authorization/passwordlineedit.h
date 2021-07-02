#ifndef PASSWORDLINEEDIT_H
#define PASSWORDLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QAction>

class PasswordLineEdit : public QLineEdit {
public:
    explicit PasswordLineEdit(QWidget* parent = nullptr);

private:
    QToolButton* _revealHideButton;
    QIcon _eyeOn, _eyeOff;
};

#endif // PASSWORDLINEEDIT_H
