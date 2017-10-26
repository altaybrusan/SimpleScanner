// this initialization is required for my own VTK
// project compilation.
// I have chosen the backend OpenGl2 (in CMake)
// and this cause some sort of problems that,
// this initializations can solve.
#include <vtkAutoInit.h>
VTK_MODULE_INIT (vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

#include "ui_mainwindow.h"


#include <QDebug>
#include <QString>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QtConcurrent>
#include <QMetaEnum>

#include "Model/gulmaygeneratorconnector.h"
#include "Model/teledynedetectorconnector.h"
#include "Model/deltab2motordriverconnector.h"
#include "Model/settingsmanager.h"

#include "Controller/gulmaygeneratorcontroller.h"
#include "Controller/teledynedetectorcontroller.h"
#include "Controller/deltab2motordrivercontroller.h"
#include "Controller/displaycontroller.h"

#include "View/mainwindow.h"
#include "View/serialportconfigdialog.h"
#include "View/gulmaygeneratorpanelview.h"
#include "View/teledynedetectorpanelview.h"
#include "View/deltab2motordriverpanelview.h"
#include "View/serialportconfigdialog.h"

#include<vtkRenderWindow.h>

namespace SimpleScanner {


    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),        
        _generatorPanelView(*new GulmayGeneratorPanelView(nullptr)),
        _genConnector(*new GulmayGeneratorConnector()),
        _generator(new GulmayGeneratorController(this,&_generatorPanelView,&_genConnector)),
        _detectorPanelView(*new TeledyneDetectorPanelView(nullptr)),
        _detConnector(*new TeledyneDetectorConnector(this)),
        _detector(new TeledyneDetectorController(this,&_detectorPanelView,&_detConnector)),
        _machinePanelView(*new DeltaB2MotorDriverPanelView(nullptr)),
        _machineConnector(*new DeltaB2MotorDriverConnector(nullptr)),
        _machine(new DeltaB2MotorDriverController(this,&_machinePanelView,&_machineConnector)),
        ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

        ui->splitter->setStretchFactor(0,1);
        _generatorPanelView.setParent(this);
        _detectorPanelView.setParent(this);
        _machinePanelView.setParent(this);

        ui->generatorWidgetScrollArea->setWidget(&_generatorPanelView);
        ui->detectorWidgetScrollArea->setWidget(&_detectorPanelView);
        ui->actuatorWidgetscrollArea->setWidget(&_machinePanelView);

        if(!SettingsManager::IsSettingsValid())
        {
            QMessageBox::warning(this, tr("Setting file is missing"),
                                 tr("Setting file is not available.\n "
                                    "Please configure the connection parameters for machinary and generator."
                                    "then restart the program."),
                                 QMessageBox::Ok);
        }
        else
        {
            _machineConnector.UpdateConnectionSettings(SettingsManager::GetActuatorSettings());
            _genConnector.UpdateConnectionSettings(SettingsManager::GetGeneratorSettings());
        }

        _displayController = new DisplayController(this->ui->displayWidget->GetRenderWindow()),


        Wireup();
    }


    MainWindow::~MainWindow()
    {
        delete ui;
        delete _detector;
        delete _machine;
        delete _generator;
    }

    void MainWindow::OnProgress(float percent)
    {
        ui->textBrowser->append("Progress: [ " + QString::number(percent)+" ]");
    }

    void MainWindow::Wireup()
    {
        connect(_generator,&GulmayGeneratorController::NotifyConnectionError,this,&MainWindow::WriteMessage);
        connect(_generator,&GulmayGeneratorController::NotifyGeneratorError,this,&MainWindow::WriteMessage);
        connect(_generator,&GulmayGeneratorController::NotifyGeneratorMessage,this,&MainWindow::WriteMessage);
        connect(_detector,&TeledyneDetectorController::NotifyDetectorMessage,this,&MainWindow::WriteMessage);
        connect(_detector,&TeledyneDetectorController::NotifyDetectorError,this,&MainWindow::WriteMessage);
        connect(_detector,&TeledyneDetectorController::NotifyProgress,this,&MainWindow::OnProgress);
        connect(_machine,&DeltaB2MotorDriverController::NotifyActuatorError,this,&MainWindow::WriteMessage);
        connect(_machine,&DeltaB2MotorDriverController::NotifyActuatorMessage,this,&MainWindow::WriteMessage);
    }

    void MainWindow::closeEvent (QCloseEvent *event)
    {

        QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Simple Scanner",
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes)
        {
            event->ignore();
        } else
        {
            _machine->PrepareForShutdown();
            event->accept();
            ui->displayWidget->GetRenderWindow()->Finalize();
            _displayController->Terminate();

        }

    }

    void MainWindow::WriteMessage(const QString &errorMsg)
    {
        ui->textBrowser->append(errorMsg);
    }

    void KeypressCallbackFunction(vtkObject*,
                                  long unsigned int vtkNotUsed(eventId),
                                  void* vtkNotUsed(callData))
  {
    qDebug() << "Caught event in MyClass";
  }

    void MainWindow::on_action_Generator_triggered()
    {
        generatorSettingsDialog =new SerialPortConfigDialog(SettingsManager::GetSettingFilePath(),"GEN",this);
        generatorSettingsDialog->show();
    }

    void MainWindow::on_action_Actuators_triggered()
    {
        generatorSettingsDialog =new SerialPortConfigDialog(SettingsManager::GetSettingFilePath(),"ACT",this);
        generatorSettingsDialog->show();
    }

    // MUST NOT BE CALLED MULTIPLE TIMES,
    // O.W. WILL NOT FINISHED AT ALL.
    void MainWindow::on_startBtn_clicked()
    {
        if(_displayController->IsValidTiffFile("D:/Trash/test.tiff"))
        {
             _displayController->DisplayImage("D:/Trash/test.tiff");
        }
    }
}


