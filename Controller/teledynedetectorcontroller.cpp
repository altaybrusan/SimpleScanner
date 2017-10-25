#include <QDebug>
#include <QMetaObject>
#include "teledynedetectorcontroller.h"
#include "View/teledynedetectorpanelview.h"
#include "Model/teledynedetectorconnector.h"
#include <qtconcurrentrun.h>
#include <QTime>
#include <QTimer>
#include <QtMath>
#include <QRegExp>

namespace SimpleScanner
{
    enum DETECTOR_STATE
    {
        NOTCONNECTED,
        CONNECTED,
        ERROR
    } _state= NOTCONNECTED;

    static int PROGRESS_PRECENT=0;

    /*!
     * The TeledyneDetectorController is responsible for:
     * >holding the relation between binning
     * and the maximum value of the image height.
     * \ref Argus detector manual Table 7
     * [ Binning Modes, Image Height and Scanning Distance ]
     *
     * Binning mode | Max Number Line | Scanning Distance(mm)
     * -----------------------------------------------------
     *  2x2         |    5555         |  299.97
     *  3x3         |    8332         |  674.892
     *  4x4         |    11100        |  1198.8
     *  5x5         |    13887        |  1874.745
     *  6x6         |    16665        |  2699.73
     *
     * if the user change the binning mode, it automatically updates the
     * maximum number of lines the user can change.
     *
     * The other tasks of this class is to estimate image capture progress:
     *    Ttotal = TIAT + TICT + TETT
     *    TIAT = Image Acquisition Time = (number of lines) / (line rate)
     *    TICT = Image Reconstruction Time = (Image size in bytes) / (4.5 MB/ sec)
     *    TETT = Ethernet Transfer Time = (Image size in bytes) / (4.5 MB/ sec)
     *    ImageSize = 8160/ (binning factor)*2*(number of lines) [bytes]
     *
     * Point: Binning, Linespeed, ImageHeight Are BASED ON CONNECTION.
     * By closing a connection all of them are reset.
     * Because of this I gnored get parameters function.
     *
     * The view passes the settings for the detector.
     * It is important to note that the view DOES NOT PROVIDE THE BINNING VALUE.
     * It just provide the current SELECTED INDEX ON BINNING COMBOBOX.
     * The controller should process this raw value.
     * For example if 0 is received, this means that binning is 2.
     * I think it is the controller job to handel this type of details
     * Because the the controller may change the order of the items in
     * the combobox in the future and view object has no idea about how
     * to process map the selected index into actual binning
     * value
     * For current UI design
     *
     *   INDEX | BINNING
     *  -----------------
     *    0    |  2
     *    1    |  3
     *    2    |  4
     *    3    |  5
     *    4    |  6
     *
     * Point: In area mode image height is fixed (128 pixel).
     * TeledyneDetectorController handels this too.
     */

    TeledyneDetectorController::TeledyneDetectorController(QObject *parent,
                                                           TeledyneDetectorPanelView* view,
                                                           TeledyneDetectorConnector* connector):
        QObject(parent),
        _view(*view),
        _timer(new QTimer()),
        _connector(*connector)
    {
        _timer->setInterval(1000);
        Wireup();
    }

    TeledyneDetectorController::~TeledyneDetectorController()
    {
        if(_timer)
            _timer->stop();
        delete _timer;
    }

    void TeledyneDetectorController::OnConnectorMessageReceived(const QString &message)
    {
        emit NotifyDetectorMessage(message);
    }

    void TeledyneDetectorController::OnConnectorError(const QString &error) const
    {
        if(_timer->isActive())
            _timer->stop();

        emit NotifyDetectorError(error);
    }

    void TeledyneDetectorController::OnGrabImage(int &binningIndex, int &speed, int &height, QString &mode)
    {

        _timer->start(); // Run on UI thread.

        int _binning = ConvertBinningIndexToBinningMode(binningIndex);

        emit NotifyDetectorError("Trying to connect to detecctor.");

        QFuture<void> future = QtConcurrent::run([&](
                                                 int _binning,
                                                 int _speed,
                                                 int _height,
                                                 QString _mode){

            if(_connector.Connect(true))
            {
                if(_mode.contains("Area"))
                    _height=128;   //This is fixed height for Area mode

                float _imageSize = ((float)((8160/(_binning))*2*_height))/1000000; // in MB
                float _imageReconstructionTime = _imageSize/4.5;
                float _ethernetTransferTime = _imageSize/4.5;
                float _imageAcquisitionTime = ((float)_height)/((float)_speed);

                // this last is for the connecting, updating and disconnecting
                // overhead
                _totalAcqTimeEstimate = _imageReconstructionTime +
                        _ethernetTransferTime +
                        _imageAcquisitionTime +
                        1;

                emit NotifyDetectorMessage("Estimated grab time: "+ QString::number(_totalAcqTimeEstimate) + " seconds.");

                if(_connector.OnBinningUpdated(_binning))
                {
                    if(_connector.OnLineSpeedUpdated(_speed))
                    {
                        if(_connector.OnImageHeightUpdated(_height))
                        {
                            if(_mode.contains("Area"))
                                _connector.SetIntoInternalTriggerAreaMode();

                            if(_mode.contains("TDI"))
                                _connector.SetIntoInternalTriggerTDIMode();

                            QTime _progressbartimer;
                            _progressbartimer.start();

                            if(_connector.CaptureImage())
                            {
                                int nMilliseconds = _progressbartimer.elapsed()/1000;
                                emit NotifyDetectorMessage("Ellapsed time (sec): " + QString::number(nMilliseconds));

                                _connector.Disconnect(true);
                            }
                        }
                    }
                }
            }
        },_binning,speed,height,mode);
    }

    void TeledyneDetectorController::OnCommandFinished()
    {
        _view.EnableCommands();
        PROGRESS_PRECENT=0;
        if(_timer->isActive())
            _timer->stop();
    }

    void TeledyneDetectorController::OnDetectorConnectionError()
    {
        // there may be connection lost during  executing a command
        // first do command finshed house keeping.
        OnCommandFinished();

        emit NotifyDetectorError("Detector connection is lost or "
                                 "there is no connection.");
    }


    void TeledyneDetectorController::Wireup()
    {
        connect(&_view,SIGNAL(NotifyGrabImage(int&,int&,int&,QString&)),
                this, SLOT(OnGrabImage(int&,int&,int&,QString&)));
        connect(&_connector,SIGNAL(NotifyDetectorError(QString)),
                this,SLOT(OnConnectorError(QString)));
        connect(&_connector,SIGNAL(NotifyDetectorConnectionError()),
                this,SLOT(OnDetectorConnectionError()));
        connect(&_connector,SIGNAL(NotifyDetectorMessage(QString)),
                this,SLOT(OnConnectorMessageReceived(QString)));
        connect(&_connector,SIGNAL(NotifyDetectorConnectionOpened()),&_view, SLOT(DisableCommands()));
        connect(&_connector,SIGNAL(NotifyDetectorConnectionClosed()),this, SLOT(OnCommandFinished()));


        /*!
         * this handels the Image height enable/disable with
         * respect to mode (Area or TDI).
         * There is a glich with this approach.
         * I assume the first element in the combo box
         * is "TDI" but if is were not the case,
         * then this SIGNAL is not triggered for
         * the first time when the form is loaded
         *
         */
        connect(&_view,&TeledyneDetectorPanelView::NotifyDeviceModeIsChanged,this,
                [this](const QString& value)
        {
            if(value.contains("Area"))
                static_cast<TeledyneDetectorPanelView*>(sender())->DisableImageHeight();
            if(value.contains("TDI"))
                static_cast<TeledyneDetectorPanelView*>(sender())->EnableImageHeight();
        });
        connect(_timer,&QTimer::timeout,this,[this]()
        {
            PROGRESS_PRECENT++;
            float _percent = qFloor(((float)PROGRESS_PRECENT/_totalAcqTimeEstimate)*100);

            // The timer first starts, then a connection is made.
            // if the device is not connected then it will take a time
            // to issue the connection error. In order to avoid this problem
            // I use a flag. This flag is not thread safe. If I see any problem
            // in the future I may add the thread safty gaurds.

            // the timer may not stop on time.
            if(_percent<100)
                emit NotifyProgress(_percent);
            //            if(_isConnected)
            //            {
            //            }

        });

    }

    int TeledyneDetectorController::ConvertBinningIndexToBinningMode(int &index)
    {
        return index+2;
    }

}

