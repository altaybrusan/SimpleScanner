#ifndef GULMAYGENERATORCONNECTOR_H
#define GULMAYGENERATORCONNECTOR_H

#include <QObject>
#include <QQueue>
#include <QThread>
#include <QMap>


class QMutex;
class QWaitCondition;
class QSettings;
class QSerialPort;

namespace SimpleScanner
{
    class GulmayGeneratorConnector : public QThread
    {
        Q_OBJECT
    public:
        explicit GulmayGeneratorConnector(QObject *parent = nullptr,
                                          QMap<QString,QString>* settings = nullptr);
        ~GulmayGeneratorConnector();

        void run() Q_DECL_OVERRIDE;

        void StartGenerator();
        void DisconnectFromMachine();

        void UpdateKvp(int newValue);
        void UpdateAmp(int newValue);
        void UpdateShootTime(int newValue);
        void UpdateConnectionSettings(QMap<QString,QString> *parameters);

        void CheckAnyError();


    signals:

        void NotifyShortWarmupIsRequired();
        void NotifyLongWarmupIsRequired();
        void NotifyGeneratorError(const QString& errorMsg) const;
        void NotifyGeneratorMessage(const QString& message) const;

        void NotifySuccessfullConnectionEstablished() const;
        void NotifyConnectionError(const QString& errorMsg) const;
        void NotifyConnectionClosed();// emited when the connectionn is gracefully closed

        void NotifyKvpValue(const int& value) const;
        void NotifyCurrentValue(const int& value) const;
        void NotifyShootTime(const int& value) const;
        void NotifyEllapsedExposureTime(const int& value) const;
        void NotifyGeneratorMode(const int& value) const;

        void NotifyExposureStoped() const;
        void NotifyExposureStarted() const;
        void NotifyWarmupFinished() const;

        void NotifyLargFocalSpotSizeTriggered() const;
        void NotifySmallFocalSpotSizeTriggered() const;



    public slots:

        void StartExposure();
        void StopExposure();
        void SetFocalSpotSizeSmall();
        void SetFocalSpotSizeLarg();
        void EmergencyStop();
        void CloseConnection();
        void QueryExposureParameters();
        void QueryGeneratorMode();
        void QueryWarmupProgram();
        void QueryFocalSpotSize();
        void QueryEllapsedExposureTime();
        void StartShortTermWarmup();
        void StartLongTermWarmup();



    private slots:
        //void OnControlGenerator();


    private:

        QMutex& mutex;
        QWaitCondition& cond;
        // The controller class is responsible for checking the connection
        // periodically. Move it to the controller class.
        //QTimer *timer;
        QQueue<QString> commandQueue;// The producer-consumer buffer
        // A flag to stop the thread. Internal use only
        bool quit;
        QMap<QString,QString>* _settings;

        // isWamrupCompleted is generator-state-variables
        // It should be handeled by controller class not the model.
        //bool isWamrupCompleted;

        // internal use only
        bool emergencyStop;

        // instead of path to settings, process the settings and then
        // pass the settings itself.
        //QString* connectionSettingFilePath;

        void transaction(const QString &request);
        void InitConnection(QSerialPort *connection, QMap<QString, QString> *settings);
        void ProcessGeneratorResponse(QString &response);

        void SetGeneratorInUserMode();
        void SuppressErrorsVoltageAndAmpError();


        int StringToInteger(const QString& string,int index);
        void SendMessage(QString currentRequest, QSerialPort *connection);
        QString IntToThreeDigitString(int &param);

    };


}
#endif // GULMAYGENERATORCONNECTOR_H
