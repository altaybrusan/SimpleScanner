#include "teledynedetectorpanelview.h"
#include "ui_teledynedetectorpanelview.h"


namespace SimpleScanner
{
    TeledyneDetectorPanelView::TeledyneDetectorPanelView(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TeledyneDetectorPanelView)
    {
        ui->setupUi(this);        
    }

    TeledyneDetectorPanelView::~TeledyneDetectorPanelView()
    {
        delete ui;
    }

    void TeledyneDetectorPanelView::UpdateImageHeightBoundries(int max)
    {
        ui->imageHeightSpinBox->setMaximum(max);
    }

    void TeledyneDetectorPanelView::DisableImageHeight()
    {
        ui->imageHeightSpinBox->setEnabled(false);
    }

    void TeledyneDetectorPanelView::EnableImageHeight()
    {
        ui->imageHeightSpinBox->setEnabled(true);
    }

    void TeledyneDetectorPanelView::UpdateBinning(int &binningIndex)
    {
        ui->binningComboBox->setCurrentIndex(binningIndex);
    }
    void TeledyneDetectorPanelView::UpdateSpeed(int &speed)
    {
        ui->lineSpeedSpinBox->setValue(speed);
    }
    void TeledyneDetectorPanelView::UpdateDetectorMode(QString mode)
    {
        ui->modeComboBox->setCurrentText(mode);
    }
    void TeledyneDetectorPanelView::UpdateImageHeight(int &height)
    {
        ui->imageHeightSpinBox->setValue(height);
    }

    void TeledyneDetectorPanelView::DisableCommands()
    {
        ui->commandsframe->setEnabled(false);
    }
    void TeledyneDetectorPanelView::EnableCommands()
    {
        ui->commandsframe->setEnabled(true);
    }

    void TeledyneDetectorPanelView::on_modeComboBox_currentIndexChanged(const QString &arg)
    {
        emit NotifyDeviceModeIsChanged(arg);
    }

    // For more info about the parameters refer to
    // TeledyneDetectorController class.
    void TeledyneDetectorPanelView::on_grabBtn_clicked()
    {
        int _index =  ui->binningComboBox->currentIndex();
        int _speed = ui->lineSpeedSpinBox->value();
        int _height = ui->imageHeightSpinBox->value();
        QString _mod =ui->modeComboBox->currentText();

        emit NotifyGrabImage(_index,_speed,_height ,_mod);

    }

}












