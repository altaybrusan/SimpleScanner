#-------------------------------------------------
#
# Project created by QtCreator 2017-07-21T20:42:49
#
#-------------------------------------------------

QT       += core gui serialbus serialport concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SimpleScanner
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
#        actuator.cpp \
        Model/gulmaygeneratorconnector.cpp \
        Model/teledynedetectorconnector.cpp\
        Model/deltab2motordriverconnector.cpp\
        Controller/gulmaygeneratorcontroller.cpp \
        Controller/teledynedetectorcontroller.cpp \
        View/gulmaygeneratorpanelview.cpp \
        View/mainwindow.cpp \
        View/serialportconfigdialog.cpp \
        View/teledynedetectorpanelview.cpp\
        View/deltab2motordriverpanelview.cpp\
        utils.cpp \
    Controller/deltab2motordrivercontroller.cpp




HEADERS += \
#        actuator.h \
        Model/gulmaygeneratorconnector.h \
        Model/teledynedetectorconnector.h \
        Model/deltab2motordriverconnector.h\
        Controller/gulmaygeneratorcontroller.h \
        Controller/teledynedetectorcontroller.h \
        View/gulmaygeneratorpanelview.h \
        View/serialportconfigdialog.h \
        View/teledynedetectorpanelview.h\
        View/mainwindow.h\
        View/deltab2motordriverpanelview.h\
        utils.h \
    Controller/deltab2motordrivercontroller.h \
    Controller/dummyfile.h




FORMS += \
        View/mainwindow.ui \
        View/serialportconfigdialog.ui \
        View/gulmaygeneratorpanelview.ui \
        View/teledynedetectorpanelview.ui \
        View/deltab2motordriverpanelview.ui

RESOURCES += \
    resources.qrc


INCLUDEPATH += C:/VTK7/include/vtk-7.1
INCLUDEPATH += C:/Users/Altay/Documents/Qt/Projects/SimpleScanner/Teledyne/Include

#LIBS += C:/VTK7/lib/QVTKWidgetPlugin.lib
#LIBS += C:/VTK7/lib/vtkInteractionStyle-7.1.lib
#LIBS += C:/VTK7/lib/vtkCommonCore-7.1.lib
#LIBS += C:/VTK7/lib/vtkRenderingOpenGL2-7.1.lib
#LIBS += C:/VTK7/lib/vtkViewsCore-7.1.lib
#LIBS += C:/VTK7/lib/vtkGUISupportQt-7.1.lib
#LIBS += C:/VTK7/lib/vtkRenderingCore-7.1.lib
#LIBS += C:/VTK7/lib/vtkInteractionImage-7.1.lib
#LIBS += C:/VTK7/lib/vtkRenderingImage-7.1.lib



LIBS += C:/VTK7/lib/QVTKWidgetPlugin.lib
LIBS += C:/VTK7/lib/vtkInteractionStyle-7.1.lib
LIBS += C:/VTK7/lib/vtkCommonCore-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingOpenGL2-7.1.lib
LIBS += C:/VTK7/lib/vtkViewsCore-7.1.lib
LIBS += C:/VTK7/lib/vtkGUISupportQt-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingCore-7.1.lib
LIBS += C:/VTK7/lib/vtkInteractionImage-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingImage-7.1.lib
LIBS += C:/VTK7/lib/vtktiff-7.1.lib
LIBS += C:/VTK7/lib/vtkIOImage-7.1.lib
LIBS += C:/VTK7/lib/vtkInteractionImage-7.1.lib
LIBS += C:/VTK7/lib/vtkCommonExecutionModel-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingAnnotation-7.1.lib
LIBS += C:/VTK7/lib/vtkInteractionWidgets-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingCore-7.1.lib
LIBS += C:/VTK7/lib/vtkRenderingFreeType-7.1.lib
LIBS += "C:/Users/Altay/Documents/Qt/Projects/SimpleScanner/Teledyne/x64/td_telnet_x64.lib"
LIBS +="C:/Users/Altay/Documents/Qt/Projects/SimpleScanner/Teledyne/x64/td_wget_x64.lib"
