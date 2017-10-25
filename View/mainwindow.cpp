#include <vtkAutoInit.h>
VTK_MODULE_INIT (vtkRenderingFreeType)
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

#include "ui_mainwindow.h"
#include <vtkLegendScaleActor.h>
#include "View/mainwindow.h"
#include "View/serialportconfigdialog.h"
#include <QDebug>
#include <QString>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QCloseEvent>
#include <QtConcurrent>
#include <QMetaEnum>
#include "Model/gulmaygeneratorconnector.h"
#include "Model/teledynedetectorconnector.h"
#include "Model/deltab2motordriverconnector.h"

#include "Controller/gulmaygeneratorcontroller.h"
#include "Controller/teledynedetectorcontroller.h"
#include "Controller/deltab2motordrivercontroller.h"

#include "View/gulmaygeneratorpanelview.h"
#include "View/teledynedetectorpanelview.h"
#include "View/deltab2motordriverpanelview.h"
#include "View/serialportconfigdialog.h"

#include <QMap>
#include "utils.h"
#include <vtkTIFFReader.h>
#include <vtkImageViewer2.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkInteractorStyleImage.h>
#include <vtkTextRendererStringToImage.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>

#include <vtkLegendScaleActor.h>
#include <vtkAxisActor2D.h>
#include <vtkCommand.h>



namespace SimpleScanner {

    // A class not derived from vtkObjectBase
    class MyClass
    {
      public:
        void KeypressCallbackFunction(vtkObject*,
                                      long unsigned int vtkNotUsed(eventId),
                                      void* vtkNotUsed(callData))
      {
        qDebug() << "Caught event in MyClass";
      }

    };


    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        _settingsPath(QApplication::applicationDirPath() + "/settings.ini"),
        _generatorPanelView(*new GulmayGeneratorPanelView(nullptr)),
        _genConnector(*new GulmayGeneratorConnector()),
        _generator(new GulmayGeneratorController(this,&_generatorPanelView,&_genConnector)),
        _detectorPanelView(*new TeledyneDetectorPanelView(nullptr)),
        _detConnector(*new TeledyneDetectorConnector(this)),
        _detector(new TeledyneDetectorController(this,&_detectorPanelView,&_detConnector)),
        _machinePanelView(*new DeltaB2MotorDriverPanelView(nullptr)),
        _machineConnector(*new DeltaB2MotorDriverConnector(nullptr)),
        _machine(new DeltaB2MotorDriverController(this,&_machinePanelView,&_machineConnector)),
        renderWindowInteractor(vtkSmartPointer<vtkRenderWindowInteractor>::New()),
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

        if(!Utils::FileExists(_settingsPath))
        {
            QMessageBox::warning(this, tr("Setting file is missing"),
                                 tr("Setting file did not found.\n "
                                    "Please configure the connection parameters for machinary and generator."),
                                 QMessageBox::Ok);
        }

        QSettings* _settings = new QSettings(_settingsPath, QSettings::IniFormat);
        QMap<QString,QString>* _settingsMap= new QMap<QString,QString>();
        _settings->beginGroup("ACT");
        QStringList childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();
        _machineConnector.UpdateConnectionSettings(_settingsMap);
        _settingsMap= new QMap<QString,QString>();
        _settings->beginGroup("GEN");
        childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();
        _genConnector.UpdateConnectionSettings(_settingsMap);


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
            renderWindowInteractor->TerminateApp();
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
        generatorSettingsDialog =new SerialPortConfigDialog(_settingsPath,"GEN",this);
        generatorSettingsDialog->show();
    }

    void MainWindow::on_action_Actuators_triggered()
    {
        generatorSettingsDialog =new SerialPortConfigDialog(_settingsPath,"ACT",this);
        generatorSettingsDialog->show();
    }

    // MUST NOT BE CALLED MULTIPLE TIMES,
    // O.W. WILL NOT FINISHED AT ALL.
    void MainWindow::on_startBtn_clicked()
    {
        vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
        //vtkSmartPointer<vtkDICOMImageReader> reader = vtkSmartPointer<vtkDICOMImageReader>::New();

        //        if (reader.CanReadFile(@"D:\Trash\Test1.tif") == 0)
        //        {
        //            MessageBox.Show("Cannot read file " + @"D:\Trash\Test1.tif", "Error", MessageBoxButtons.OK);
        //            return;
        //        }

        reader->SetFileName("D:/Trash/test.tiff");
        reader->Update();

        vtkSmartPointer<vtkImageViewer2> imageViewer =
                vtkSmartPointer<vtkImageViewer2>::New();

        imageViewer->SetColorLevel(28816);
        imageViewer->SetColorWindow(66438);
        imageViewer->SetInputConnection(reader->GetOutputPort());

        vtkSmartPointer<vtkLegendScaleActor> legendScaleActor =
          vtkSmartPointer<vtkLegendScaleActor>::New();
        legendScaleActor->TopAxisVisibilityOff();
        legendScaleActor->RightAxisVisibilityOff();
        legendScaleActor->SetLegendVisibility(0);

//        vtkSmartPointer<vtkDistanceWidget> distanceWidget =
//          vtkSmartPointer<vtkDistanceWidget>::New();
//        distanceWidget->SetInteractor(renderWindowInteractor);
//        distanceWidget->CreateDefaultRepresentation();
//        static_cast<vtkDistanceRepresentation *>(distanceWidget->GetRepresentation())
//          ->SetLabelFormat("%-#6.3g pix");

        vtkSmartPointer<vtkInteractorStyleImage> style =
                vtkSmartPointer<vtkInteractorStyleImage>::New();
//        SimpleScanner::MyClass myClass;
//        renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, &myClass, &MyClass::KeypressCallbackFunction);

        renderWindowInteractor->SetInteractorStyle(style);
        imageViewer->SetupInteractor(renderWindowInteractor);

        imageViewer->GetRenderer()->ResetCamera();
        imageViewer->SetRenderWindow(ui->displayWidget->GetRenderWindow());

        vtkRenderer* renderer = vtkRenderer::New();
        renderer->SetBackground(0,0,0);
        renderer->ResetCameraClippingRange();

//        renderer->AddActor(axisActor2D);
        renderer->AddActor(legendScaleActor);


        imageViewer->SetRenderer(renderer);
        imageViewer->Render();
        renderWindowInteractor->SetRenderWindow(ui->displayWidget->GetRenderWindow());
        renderWindowInteractor->Initialize();
//          distanceWidget->On();
        renderWindowInteractor->Start();

    }




}


