QT       += core gui printsupport sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/directormainwindow.cpp \
    src/cashiermainwindow.cpp \
    src/forms/authorization/authorizationwindow.cpp \
    src/forms/authorization/passwordlineedit.cpp \
    src/forms/contract/contractfilterproxymodel.cpp \
    src/forms/contract/addcontractwindow.cpp \
    src/forms/contract/viewcontractwindow.cpp \
    src/forms/supplier/supplierfilterproxymodel.cpp \
    src/forms/supplier/supplierwindow.cpp \
    src/forms/supplier/goodfilterproxymodel.cpp \
    src/forms/supplier/sqlsuppliercatalogcostdelegate.cpp \
    src/forms/supplier/sqlsupplierinndelegate.cpp \
    src/forms/supplier/sqlsupplierphonedelegate.cpp \
    src/forms/order/goodsinordermodel.cpp \
    src/forms/order/ordercostdelegate.cpp \
    src/forms/order/orderfilterproxymodel.cpp \
    src/forms/order/sqlorderbooleanformatdelegate.cpp \
    src/forms/order/addorderwindow.cpp \
    src/forms/order/vieworderwindow.cpp \
    src/forms/order/invoicewindow.cpp \
    src/forms/stock/sqlstockmodel.cpp \
    src/forms/stock/stockfilterproxymodel.cpp \
    src/forms/report/reportwindow.cpp \
    src/forms/client/phonelineedit.cpp \
    src/forms/client/clientdiscountdelegate.cpp \
    src/forms/client/clientdiscountwindow.cpp

HEADERS += \
    src/directormainwindow.h \
    src/cashiermainwindow.h \
	src/director_utils.h \
    src/forms/authorization/authorizationwindow.h \
    src/forms/authorization/passwordlineedit.h \
    src/forms/contract/contractfilterproxymodel.h \
    src/forms/contract/addcontractwindow.h \
    src/forms/contract/viewcontractwindow.h \
    src/forms/supplier/sqlsuppliercatalogcostdelegate.h \
    src/forms/supplier/sqlsupplierinndelegate.h \
    src/forms/supplier/sqlsupplierphonedelegate.h \
    src/forms/supplier/supplierfilterproxymodel.h \
    src/forms/supplier/supplierwindow.h \
    src/forms/supplier/goodfilterproxymodel.h \
    src/forms/order/goodsinordermodel.h \
    src/forms/order/ordercostdelegate.h \
    src/forms/order/orderfilterproxymodel.h \
    src/forms/order/addorderwindow.h \
    src/forms/order/vieworderwindow.h \
    src/forms/order/invoicewindow.h \
    src/forms/order/sqlorderbooleanformatdelegate.h \
    src/forms/stock/sqlstockmodel.h \
    src/forms/stock/stockfilterproxymodel.h \
    src/forms/report/reportwindow.h \
    src/forms/client/phonelineedit.h \
    src/forms/client/clientdiscountdelegate.h \
    src/forms/client/clientdiscountwindow.h

FORMS += \
    ui/cashiermainwindow.ui \
    ui/directormainwindow.ui \
    ui/forms/authorizationwindow.ui \
    ui/forms/addcontractwindow.ui \
    ui/forms/viewcontractwindow.ui \
    ui/forms/supplierwindow.ui \
    ui/forms/addorderwindow.ui \
    ui/forms/vieworderwindow.ui \
    ui/forms/invoicewindow.ui \
    ui/forms/reportwindow.ui \
    ui/forms/clientdiscountwindow.ui

INCLUDEPATH += \
    src/forms/authorization \
    src/forms/client
	
RESOURCES += \
    resource/resource.qrc

RC_ICONS = resource/icons/window-logo.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
