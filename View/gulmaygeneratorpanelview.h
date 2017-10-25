#ifndef GULMAYGENERATORCONTROLPANELVIEW_H
#define GULMAYGENERATORCONTROLPANELVIEW_H

#include <QWidget>

namespace Ui {
    class GulmayGeneratorPanelView;
}

namespace SimpleScanner
{

    class GulmayGeneratorPanelView : public QWidget
    {
        Q_OBJECT

    public:
        explicit GulmayGeneratorPanelView(QWidget *parent = 0);
        void SetViewAccessable(bool value);
        ~GulmayGeneratorPanelView();

        void SetKvpBoundries(int min,int max);
        void SetCurrentBoundries(int min,int max);
        void SetTimeBoundries(int min, int max);

    signals:
        void NotifyUpdateParameters(const int& kvp, const int& amper, const int& duration) const;
        void NotifyStartExposure() const;
        void NotifyStopExposure() const;
        void NotifyStartLongTermWarmup() const;
        void NotifyStartShortTermWarmup() const;
        void NotifyEmergencyStop() const;
        void NotifyPowerOn() const;
        void NotifyCheckWarmup() const;

        void NotifySmallFocalSpotTriggered();
        void NotifyLargFocalSpotTriggered();

    public slots:

        void DisableControlPanel();
        void EnableControlPanel();

        void DisableReadingExposureParameters();
        void EnableReadingExposureParameters();

        void DisableShooting();
        void EnableShooting();

        void DisableFocalSpotChange();
        void EnableFocalSpotChange();

        void DisableWarmup();
        void EnableWarmup();

        void DisablePowerOnButton();
        void UpdateKvpValue(const int& kvp);
        void UpdateCurrentValue(const int& current);
        void UpdateShootTime(const int& time);
        void UpdateEllapsedExposureTime(const int& time);

        void SetFocalSpotLarg();
        void SetFocalSpotSmall();

        void EnableCheckWarmup();
        void DisableCheckWarmup();

        void EnableShortWarmup();
        void DisableShortWarmup();

        void EnableLongWarmup();
        void DisableLongWarmup();




    private slots:


        void on_applyBtn_clicked();
        void on_stopExposureBtn_clicked();
        void on_startExposureBtn_clicked();
        void on_shortWamupBtn_clicked();
        void on_longWarmupBtn_clicked();
        void on_emergencyStopBtn_clicked();
        void on_powerBtn_clicked();
        void on_checkWarmupProgramBtn_clicked();

        void on_toggleFocalSpotBtn_toggled(bool checked);

    private:
        Ui::GulmayGeneratorPanelView *ui;
    };

}
#endif // GULMAYGENERATORCONTROLPANELVIEW_H
