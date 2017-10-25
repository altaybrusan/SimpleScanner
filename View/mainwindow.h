#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>


namespace Ui {
    class MainWindow;
}



namespace SimpleScanner {




    class SerialPortConfigDialog;
    class ActuatorDialog;

    class GulmayGeneratorConnector;
    class GulmayGeneratorPanelView;
    class GulmayGeneratorController;

    class TeledyneDetectorPanelView;
    class TeledyneDetectorConnector;
    class TeledyneDetectorController;

    class DeltaB2MotorDriverPanelView;
    class DeltaB2MotorDriverConnector;
    class DeltaB2MotorDriverController;



    class MainWindow : public QMainWindow
    {
        Q_OBJECT



    public:

        explicit MainWindow(QWidget *parent = 0);
        void closeEvent (QCloseEvent *event) override;      
        ~MainWindow();


    private slots:

        void OnProgress(float percent);
        void WriteMessage(const QString& errorMsg);
        void Wireup();
        void on_action_Generator_triggered();
        void on_action_Actuators_triggered();
        void on_startBtn_clicked();

    private:
        Ui::MainWindow *ui;
        SerialPortConfigDialog *generatorSettingsDialog;

        GulmayGeneratorConnector& _genConnector;
        GulmayGeneratorPanelView& _generatorPanelView;
        GulmayGeneratorController* _generator;

        TeledyneDetectorConnector& _detConnector;
        TeledyneDetectorPanelView& _detectorPanelView;
        TeledyneDetectorController* _detector;

        DeltaB2MotorDriverConnector& _machineConnector;
        DeltaB2MotorDriverPanelView& _machinePanelView;
        DeltaB2MotorDriverController* _machine;
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;


        QString _settingsPath;


    };
}


#endif // MAINWINDOW_H
