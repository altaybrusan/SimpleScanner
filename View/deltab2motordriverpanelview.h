#ifndef DELTAB2MOTORDRIVERPANELVIEW_H
#define DELTAB2MOTORDRIVERPANELVIEW_H

#include <QWidget>

namespace Ui {
    class DeltaB2MotorDriverPanelView;
}
namespace SimpleScanner
{
    class DeltaB2MotorDriverPanelView : public QWidget
    {
        Q_OBJECT

    public:
        explicit DeltaB2MotorDriverPanelView(QWidget *parent = 0);
        ~DeltaB2MotorDriverPanelView();
        void UpdateSpeedBoundaries(const int min=100, const int max=2000);
        void UpdateSpeed(int speed);

    signals:
        void NotifyMoveForward();
        void NotifyMoveBackward();
        void NotifyStop();
        void NotifyUpdateSpeed(int speed);


    private slots:
        void on_speedSlider_valueChanged(int value);
        void on_moveForwardBtn_clicked();
        void on_stopBtn_clicked();
        void on_moveBackwardBtn_clicked();
        void on_speedSlider_sliderReleased();

    private:
        int _currentSpeed;// used for detecting if clicked or not
        Ui::DeltaB2MotorDriverPanelView *ui;
    };
}



#endif // DELTAB2MOTORDRIVERPANELVIEW_H
