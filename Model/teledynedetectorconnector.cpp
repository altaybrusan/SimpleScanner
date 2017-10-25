#include "teledynedetectorconnector.h"
#include "ArgusAPI.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QFile>


#define DEBUGING

namespace SimpleScanner {

    //internal data structure used for connecting
    //to the teledyne xray detector
    typedef struct
    {
        char Login[INIT_STR_LEN];
        char Password[INIT_STR_LEN];
        char CameraIP[INIT_STR_LEN];
        char CameraResp[DLL_MSG_LEN];
        int WSAStatus;
        int TARGET_PORT;
    } _CAM_INFO;

    static _CAM_INFO _cam_info= _CAM_INFO();
    static int CmdTimeout = 20000;			/*	Must be greater or equal to CAM_MIN_WAIT in millisecond	*/
    static int CamConnectTimout = 3000;	/*	Timeout value for _camera_conect()	*/

    /*!
     * This class is a wrapper around Teledyne-DALSA TDI X-Ray detector.
     * For our project we have used ARGUS CEPHAL.
     * \link http://www.teledynedalsa.com/imaging/products/x-ray/scanning/argus/ARGUS-CEPH/
     * In our hardware design we have no external ACTIVE or TRIGGER signals.
     * So, this class make a connection with Internal Trigger (Software Generated Trigger).
     * For further details refer to technical documentations and user manual of the device
     */

    TeledyneDetectorConnector::TeledyneDetectorConnector(QObject *parent):
        QObject(parent),
        _logFileName("image.log"),
        _logFile(new QFile)
    {

        // this is the where the images are going to be saved in
        // I put here to make sure the path is not empty.
        // the client must update it to an appropriate directory
        _imageDepositPath = QApplication::applicationDirPath();
    }

    /*!
     * Initilize connection parameters and connects
     */
    bool TeledyneDetectorConnector::Connect(bool silent)
    {
        strcpy_s( _cam_info.CameraIP, "192.168.5.100");
        strcpy_s( _cam_info.Login, "root" );
        strcpy_s( _cam_info.Password, "root" );

        _cam_info.TARGET_PORT = TELNET_PORT;
        _cam_info.WSAStatus = 0;

        QString _cmd;
        QByteArray _array;
        char* _buffer;
        int _result;

        _result = _camera_connect( _cam_info.CameraIP, _cam_info.TARGET_PORT,
                                   &_cam_info.WSAStatus, _cam_info.CameraResp,
                                   _cam_info.Login, _cam_info.Password,
                                   CamConnectTimout );

        if(_result !=0)
        {
            emit NotifyDetectorConnectionError();
            return false;
        }
        else if(!silent)
        {
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
        }


        //	Display camera settings.
        if(!silent)
        {
            _cmd= "gcp";
            _array = _cmd.toLocal8Bit();
            _buffer = _array.data();
            _result = _camera_cmd( _buffer, _cam_info.CameraResp, CmdTimeout );
            if(_result != 0)
            {
                emit NotifyDetectorError("Failed to get detector parameters (gcp). Returned value: "
                                         + QString(_result));
                return false;
            }
#ifdef DEBUGING
            qDebug()<<"Camera response:  ...\n"<< QString::fromLocal8Bit(_cam_info.CameraResp);
#endif
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp));
        }
        emit NotifyDetectorConnectionOpened();

        return true;
    }
    bool TeledyneDetectorConnector::Disconnect(bool silent)
    {
        QThread::msleep(3);
        int _result = _camera_disconnect( &_cam_info.WSAStatus, _cam_info.CameraResp, CmdTimeout );
        if(_result != 0)
        {
#ifdef DEBUGING
            qDebug()<<"_camera_disconnect failed: "
                      "Make sure wget arguments are correct\n";
#endif
            return false;
        }
        if(!silent)
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
        emit NotifyDetectorConnectionClosed();

        return true;
    }

    bool TeledyneDetectorConnector::OnBinningUpdated(const int binning)
    {
        QString _cmd= "sbm "+ QString::number(binning);
        QByteArray _array = _cmd.toLocal8Bit();
        char* _buffer = _array.data();
        int _result = _camera_cmd( _buffer, _cam_info.CameraResp, CmdTimeout );
        if(_result !=0)
        {
            emit NotifyDetectorError("Failed to set binning mode (sbm). Returned value: "
                                     + QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
            return false;
        }
        else
        {
            _binning = binning;
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
        }
        return true;
    }

    bool TeledyneDetectorConnector::OnLineSpeedUpdated(const int speed)
    {
        QString _cmd= "slr "+ QString::number(speed);
        QByteArray _array = _cmd.toLocal8Bit();
        char* _buffer = _array.data();
        int _result = _camera_cmd( _buffer, _cam_info.CameraResp, CmdTimeout );

        if(_result !=0)
        {
            emit NotifyDetectorError("Failed to change line speed. Returned value: "
                                     + QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
            return false;
        }
        else
        {
            _speed = speed;
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
        }
        return true;
    }

    bool TeledyneDetectorConnector::OnImageHeightUpdated(const int height)
    {
        QString _cmd= "sih "+ QString::number(height);
        QByteArray _array = _cmd.toLocal8Bit();
        char* _buffer = _array.data();
        int _result = _camera_cmd( _buffer, _cam_info.CameraResp, CmdTimeout );

        if(_result !=0)
        {
            emit NotifyDetectorError("Failed to set image height (sih). Returned value: "
                                     + QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
            return false;
        }
        else
        {
            _imageHeight=height;
            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
        }
        return true;

    }

    void TeledyneDetectorConnector::SetIntoInternalTriggerAreaMode()
    {
        ChangeMode(2);
    }
    void TeledyneDetectorConnector::SetIntoInternalTriggerTDIMode()
    {

        ChangeMode(1);

    }

    bool TeledyneDetectorConnector::CaptureImage()
    {
        QString _bin= "acq";
        QByteArray array = _bin.toLocal8Bit();
        char* buf = array.data();
        int _result = _camera_cmd( buf, _cam_info.CameraResp, CmdTimeout*10 );
        if(_result != 0)
        {
            emit NotifyDetectorError(QString("Failed to acquire image (acq. Returned value: %1 )").arg(QString::number(_result)));
#ifdef DEBUGING
            qDebug()<<"_camera_get_image failed; "
                      "Make sure wget arguments are correct\n";
#endif
            return false;
        }

        emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());


        /*
         * This section is from SDK documentation. Use it as is
         * argv[] gets permuted in wget (library SDK) automatically
         * url is moved to the last position always	(?!)
         */




        QString _str6 = _imageDepositPath + "/"+GetFileName()+".tiff";
        QByteArray ba6 = _str6.toLatin1();
        char *arg6 = ba6.data();

        QString _str8 = _imageDepositPath+"/"+_logFileName;
        QByteArray ba8 = _str8.toLatin1();
        char *arg8 = ba8.data();



        char  arg0[] = "_camera_get_image";
        char  arg1[] = "-w";
        char  arg2[] = "2";
        char  arg3[] = "-T";
        char  arg4[] = "15";
        char  arg5[] = "-O";
        char  arg7[] = "-o";
        char  arg9[] = "http://192.168.5.100/images/tp.tiff";
        char* argv[] = {&arg0[0], &arg1[0], &arg2[0], &arg3[0],&arg4[0], &arg5[0],
                        &arg6[0], &arg7[0], &arg8[0], &arg9[0],NULL };
        int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

#ifdef DEBUGING
        qDebug()<<argc;
        qDebug()<<argv[0];
        qDebug()<<argv[1];
        qDebug()<<argv[2];
        qDebug()<<argv[3];
        qDebug()<<argv[4];
        qDebug()<<argv[5];
        qDebug()<<argv[6];
        qDebug()<<argv[7];
        qDebug()<<argv[8];
        qDebug()<<argv[9];
        qDebug()<<"Downloading image from Camera ... \n";
#endif
        _result = _camera_get_image( argc, argv, _cam_info.CameraResp );

#ifdef DEBUGING
        qDebug()<<"returned value is "<<_result;
        qDebug()<<_cam_info.CameraResp;
#endif
        return true;
    }
    QString TeledyneDetectorConnector::GetFileName()
    {
        QString _fileName = QString::number(QTime::currentTime().hour())+
                QString::number(QTime::currentTime().minute())+
                QString::number(QTime::currentTime().second())+
                QString::number(QTime::currentTime().msec());

        return _fileName;
    }

    /*!
     * mode 1: InInternalTriggerTDIMode
     * mode 2: InInternalTriggerAreaMode
     */
    void TeledyneDetectorConnector::ChangeMode(int mode)
    {
        if (mode ==1 || mode == 2)
        {
            //	Set the  Internal Trigger (Software Generated Trigger)
            QString _cmd = "sim "+ QString::number(mode);
            QByteArray _array = _cmd.toLocal8Bit();
            char* _buffer = _array.data();
            int _result = _camera_cmd( _buffer, _cam_info.CameraResp, CmdTimeout );
            if(_result != 0)
            {
                emit NotifyDetectorError("Failed to Set into internal trigger mode (sim). "
                                         "Returned value: " + QString(_result));
                return;
            }

            emit NotifyDetectorMessage(QString::fromLocal8Bit(_cam_info.CameraResp).simplified());
            //            if(mode == 1)
            //            {
            //                emit NotifyDetectorIsInInternalTriggerTDIMode();
            //            }
            //            else
            //            {
            //                emit NotifyDetectorIsInInternalTriggerAreaMode();
            //            }
        }
    }
    void TeledyneDetectorConnector::SetImageRepositoryPath(QString &path)
    {
        _imageDepositPath = path;
    }

}
