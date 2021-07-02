#ifndef PHONELINEEDIT_H
#define PHONELINEEDIT_H

#include <QLineEdit>

class PhoneLineEdit : public QLineEdit {
    Q_OBJECT

public:
    explicit PhoneLineEdit(QWidget* parent = nullptr);
    void Clear();
    bool IsValid() const;
    static bool IsValid(const QString& phone);

protected:
    // Можно обойтись и перегрузкой focusInEvent, но тогда установка курсора setCursorPosition(3) не сработает в случае с "пустым" телефоном
    void mousePressEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QString _phoneMask, _emptyPhone;
};

#endif // PHONELINEEDIT_H
