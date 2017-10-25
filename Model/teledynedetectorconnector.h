#ifndef TELEDYNEDETECTOR_H
#define TELEDYNEDETECTOR_H
#include <QObject>
#include <QDialog>
#include "ArgusAPI.h"


class QFileSystemWatcher;
class QFile;

namespace SimpleScanner {


    /*	standard telnet port number	 */
#define TELNET_PORT					23
    /*	Camera response maximum length, must be greater than RXBUF_LEN longest camera
     * response burst is FACT>h = 54?? bytes */
#define CAM_RESP_LEN				(256 * 25)
    /*	string length limit of the camera command */
#define CAM_TX_BUF_LEN				30
    /*	Length limit for user entry to dll
    /*	Demo Code Constant Alias	*/
#define INIT_STR_LEN				20
    /*	Camera resonse, MUST be greater or equal to dll buffer size	CAM_RESP_LEN */
#define DLL_MSG_LEN					CAM_RESP_LEN
    /*	Command string length	*/
#define CMD_LEN						CAM_TX_BUF_LEN


    class TeledyneDetectorConnector:public QObject
    {
        Q_OBJECT

    public:
        explicit TeledyneDetectorConnector(QObject *parent );
        void SetImageRepositoryPath(QString& path);

    public slots:
        bool OnBinningUpdated(const int binning);
        bool OnLineSpeedUpdated(const int speed);
        bool OnImageHeightUpdated(const int height);
        void SetIntoInternalTriggerAreaMode();
        void SetIntoInternalTriggerTDIMode();
        bool CaptureImage();
        bool Connect(bool silent=false);
        bool Disconnect(bool silent=false);


    signals:
        void NotifyDetectorMessage(const QString& message);
        void NotifyDetectorError(const QString& message) const;
        void NotifyDetectorConnectionError() const;
        void NotifyDetectorConnectionOpened() const;
        void NotifyDetectorConnectionClosed() const;
//        void NotifyDetectorIsInInternalTriggerAreaMode()const;
//        void NotifyDetectorIsInInternalTriggerTDIMode() const;


    private:

        QString _logFileName;
        QString _imageDepositPath;
        int _imageHeight;
        int _speed;
        int _binning;
        char Cmd[CMD_LEN]={0};
        char TargetFileName[50] = {0};
        char TargetLogName[50] = {0};
        QFile* _logFile;
        QString GetFileName();
        void ChangeMode(int mode);


    };
}
#endif // TELEDYNEDETECTOR_H


