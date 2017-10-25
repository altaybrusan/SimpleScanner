#include "gulmaygeneratorpanelview.h"
#include "ui_gulmaygeneratorpanelview.h"

namespace SimpleScanner
{
    GulmayGeneratorPanelView::GulmayGeneratorPanelView(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::GulmayGeneratorPanelView)
    {
        ui->setupUi(this);
    }

    void GulmayGeneratorPanelView::SetViewAccessable(bool value)
    {
        ui->mainFrame->setEnabled(value);
    }

    GulmayGeneratorPanelView::~GulmayGeneratorPanelView()
    {
        delete ui;
    }

    void GulmayGeneratorPanelView::SetKvpBoundries(int min, int max)
    {
        if(min>0)
            ui->KvpSpinBox->setMinimum(min);
        if(max>min)
            ui->KvpSpinBox->setMaximum(max);
    }

    void GulmayGeneratorPanelView::SetCurrentBoundries(int min, int max)
    {
        if(min>0)
            ui->currentSpinBox->setMinimum(min);
        if(max>min)
            ui->currentSpinBox->setMaximum(max);

    }

    void GulmayGeneratorPanelView::SetTimeBoundries(int min, int max)
    {
        if(min>0)
            ui->durationSpinBox->setMinimum(min);
        if(max>min)
            ui->durationSpinBox->setMaximum(max);

    }



    void GulmayGeneratorPanelView::DisableControlPanel()
    {
        ui->mainFrame->setEnabled(false);
    }

    void GulmayGeneratorPanelView::EnableControlPanel()
    {
        ui->mainFrame->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableReadingExposureParameters()
    {
        ui->applyBtn->setEnabled(false);
    }

    void GulmayGeneratorPanelView::EnableReadingExposureParameters()
    {
        ui->applyBtn->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableShooting()
    {
        //ui->startExposureBtn->setEnabled(false);
        ui->exposureBtnsFrame->setEnabled(false);

    }

    void GulmayGeneratorPanelView::EnableShooting()
    {
        //ui->startExposureBtn->setEnabled(true);
        ui->exposureBtnsFrame->setEnabled(true);

    }

    void GulmayGeneratorPanelView::DisableFocalSpotChange()
    {
        ui->toggleFocalSpotBtn->setEnabled(false);

    }

    void GulmayGeneratorPanelView::EnableFocalSpotChange()
    {
        ui->toggleFocalSpotBtn->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableWarmup()
    {
        ui->longWarmupBtn->setEnabled(false);
        ui->shortWamupBtn->setEnabled(false);
        //ui->checkWarmupProgramBtn->setEnabled(false);
        //ui->warmupBtnsFrame->setEnabled(false);
    }

    void GulmayGeneratorPanelView::EnableWarmup()
    {
        ui->longWarmupBtn->setEnabled(true);
        ui->shortWamupBtn->setEnabled(true);
//        ui->checkWarmupProgramBtn->setEnabled(true);
        //ui->warmupBtnsFrame->setEnabled(true);

    }

    void GulmayGeneratorPanelView::DisablePowerOnButton()
    {
        ui->powerBtn->setEnabled(false);

    }

    void GulmayGeneratorPanelView::UpdateKvpValue(const int &kvp)
    {
        ui->kvpLcd->display(kvp);
        ui->KvpSpinBox->setValue(kvp);
    }

    void GulmayGeneratorPanelView::UpdateCurrentValue(const int &current)
    {
        ui->currentLcd->display(current);
        ui->currentSpinBox->setValue(current);
    }

    void GulmayGeneratorPanelView::UpdateShootTime(const int &time)
    {
        ui->durationLcd->display(time);
        ui->durationSpinBox->setValue(time);
    }

    void GulmayGeneratorPanelView::UpdateEllapsedExposureTime(const int &time)
    {
        ui->passedTimeLcd->display(time);
    }

    void GulmayGeneratorPanelView::SetFocalSpotLarg()
    {
        ui->toggleFocalSpotBtn->setChecked(false);
        emit NotifyLargFocalSpotTriggered();
    }

    void GulmayGeneratorPanelView::SetFocalSpotSmall()
    {
        ui->toggleFocalSpotBtn->setChecked(true);
        emit NotifySmallFocalSpotTriggered();
    }

    void GulmayGeneratorPanelView::EnableCheckWarmup()
    {

        ui->checkWarmupProgramBtn->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableCheckWarmup()
    {
        ui->checkWarmupProgramBtn->setEnabled(false);
    }

    void GulmayGeneratorPanelView::EnableShortWarmup()
    {
        ui->shortWamupBtn->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableShortWarmup()
    {
        ui->shortWamupBtn->setEnabled(false);
    }

    void GulmayGeneratorPanelView::EnableLongWarmup()
    {
        ui->longWarmupBtn->setEnabled(true);
    }

    void GulmayGeneratorPanelView::DisableLongWarmup()
    {
        ui->longWarmupBtn->setEnabled(false);
    }

    void GulmayGeneratorPanelView::on_applyBtn_clicked()
    {
        int _kvp= ui->KvpSpinBox->value();
        int _amp= ui->currentSpinBox->value();
        int _time= ui->durationSpinBox->value();
        emit NotifyUpdateParameters(_kvp,_amp,_time);
    }

    void GulmayGeneratorPanelView::on_stopExposureBtn_clicked()
    {
        emit NotifyStopExposure();
    }

    void GulmayGeneratorPanelView::on_startExposureBtn_clicked()
    {
        emit NotifyStartExposure();
    }

    void GulmayGeneratorPanelView::on_shortWamupBtn_clicked()
    {
        emit NotifyStartShortTermWarmup();
    }

    void GulmayGeneratorPanelView::on_longWarmupBtn_clicked()
    {
        emit NotifyStartLongTermWarmup();
    }

    void GulmayGeneratorPanelView::on_emergencyStopBtn_clicked()
    {
        emit NotifyEmergencyStop();
    }

    void GulmayGeneratorPanelView::on_powerBtn_clicked()
    {
        emit NotifyPowerOn();
    }

    void GulmayGeneratorPanelView::on_checkWarmupProgramBtn_clicked()
    {
        emit NotifyCheckWarmup();
    }

    /*! Do you this it is good to let this function which signal should
     *  be issued by trigger signal. I guess On signal like FocalSpotToggle
     *  is good enough. The Controller class (i.e. GulmayGeneratorController)
     *  should keep track of the spot size and decide on the SIGNAL!!! My be changed
     *  in future versions.*/
    void GulmayGeneratorPanelView::on_toggleFocalSpotBtn_toggled(bool checked)
    {
        if(checked)
        {
           QIcon _icon= *new QIcon(":/images/image/smallspot.png");
           ui->toggleFocalSpotBtn->setIcon(_icon);
           ui->toggleFocalSpotBtn->setText("Small");
           ui->toggleFocalSpotBtn->setStyleSheet("background-color: rgb(255, 85, 127);");

           emit NotifySmallFocalSpotTriggered();
        }
        else
        {
            QIcon _icon= *new QIcon(":/images/image/largspot.png");
            ui->toggleFocalSpotBtn->setIcon(_icon);
            ui->toggleFocalSpotBtn->setText("Larg");
            ui->toggleFocalSpotBtn->setStyleSheet("background-color: rgb(85, 170, 127);");

            emit NotifyLargFocalSpotTriggered();
        }

    }
}












