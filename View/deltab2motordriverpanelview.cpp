#include "deltab2motordriverpanelview.h"
#include "ui_deltab2motordriverpanelview.h"
#include <QtMath>

namespace SimpleScanner
{
    DeltaB2MotorDriverPanelView::DeltaB2MotorDriverPanelView(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::DeltaB2MotorDriverPanelView)
    {
        ui->setupUi(this);
    }

    DeltaB2MotorDriverPanelView::~DeltaB2MotorDriverPanelView()
    {
        delete ui;
    }
    void DeltaB2MotorDriverPanelView::UpdateSpeedBoundaries(const int min, const int max)
    {
        ui->speedSlider->setMinimum(min);
        ui->speedSlider->setMaximum(max);
    }
    void DeltaB2MotorDriverPanelView::UpdateSpeed(int speed)
    {
        _currentSpeed = speed;
        ui->speedSlider->setValue(speed);
        ui->speedLcd->display(speed);
    }

    // if you drag the slider OR click the mous then you are here
    // ignore the drag event but issue the click event
    void DeltaB2MotorDriverPanelView::on_speedSlider_valueChanged(int value)
    {
        ui->speedLcd->display(QString::number(value));

        // how to detect the click event
        if(qFabs(_currentSpeed-value) == ui->speedSlider->singleStep())
        {
            // clicked on the Slider
            _currentSpeed = value;
            emit NotifyUpdateSpeed(value);
        }

    }

    void DeltaB2MotorDriverPanelView::on_moveForwardBtn_clicked()
    {
        emit NotifyMoveForward();
    }

    void DeltaB2MotorDriverPanelView::on_stopBtn_clicked()
    {
        emit NotifyStop();
    }

    void DeltaB2MotorDriverPanelView::on_moveBackwardBtn_clicked()
    {
        emit NotifyMoveBackward();
    }
    void DeltaB2MotorDriverPanelView::on_speedSlider_sliderReleased()
    {
        _currentSpeed = ui->speedSlider->value();
        emit NotifyUpdateSpeed(ui->speedSlider->value());
    }


}






