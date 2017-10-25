#ifndef TELEDYNEDETECTORPANELVIEW_H
#define TELEDYNEDETECTORPANELVIEW_H

#include <QWidget>


namespace Ui {
    class TeledyneDetectorPanelView;
}
namespace SimpleScanner
{
    class TeledyneDetectorPanelView : public QWidget
    {
        Q_OBJECT

    public:
        explicit TeledyneDetectorPanelView(QWidget *parent = 0);
        ~TeledyneDetectorPanelView();
        void UpdateImageHeightBoundries(int max);
        void DisableImageHeight();
        void EnableImageHeight();

    signals:
        void NotifyGrabImage(int& binningIndex,
                             int& speed,
                             int& height,
                             QString& modeText);

        void NotifyDeviceModeIsChanged(const QString& mode);

    private slots:
        void on_grabBtn_clicked();        
        void on_modeComboBox_currentIndexChanged(const QString &arg);



    public slots:
        void UpdateBinning(int& binningIndex);
        void UpdateSpeed(int& speed);
        void UpdateDetectorMode(QString mode);
        void UpdateImageHeight(int& height);
        void DisableCommands();
        void EnableCommands();



    private:
        Ui::TeledyneDetectorPanelView *ui;

    };

}

#endif // TELEDYNEDETECTORPANELVIEW_H
