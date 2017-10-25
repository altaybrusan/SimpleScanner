#ifndef GULMAYGENERATOR_H
#define GULMAYGENERATOR_H
#include <QWidget>
#include <QtSerialPort/QSerialPort>
#include <generatordialog.h>
#include <QEventLoop>

namespace SimpleScanner {
    class GulmayGenerator:public QObject
    {
        Q_OBJECT
    public:
        explicit GulmayGenerator(QObject *parent);

        void StartMachine();
        void StopMachine();
        void StopExposureFast();

        void UpdateKvp(const QString& newValue);
        void UpdateAmp(const QString& newValue);
        void UpdateShootTime(const QString& newValue);

        void StartExposure();
        void StopExposure();

        //        public slots:
        //        void OnStartExposure();
        //        void OnStopExposure();
        //        void OnWarmup();
        //        void GetError(const QString& errorMsg);
        //        void GetMode(const QString& modeMsg);

    public slots:
        void WarmupRemainingTime();

    signals:
        // void NotifyConnectionIsOpned(const QString& errorMsg) const;
        // void NotifyConnectionIsClosed(const QString& errorMsg) const;

        void NotifyErrorMessage(const QString& errorMsg) const;
        void NotifyMessage(const QString& message) const;

        void NotifyKvpValue(const int& value) const;
        void NotifyAmpValue(const int& value) const;
        void NotifyShootTime(const int& value) const;
        void NotifyWarmupFinished() const;
        void NotifyWarmupFailed(const QString& message) const;


        //        void NotifyWarmupResult(const int& errorMsg)const;

        //        void NotifyStartExposure() const;
        //        void NotifyStopExposure() const;
        //        void NotifyWarmupStarted() const;

        //        void NotifyTime() const;


    private:
        QSerialPort *connection;
        QString  _SettingsFile;
        QByteArray charBuffer;
        QString _Errors;
        int _KVP;
        int _mA;
        int _Time;
        QTimer *timer;
        QEventLoop _warmuploop;

        void Wireup();
        void Unwire();
        //void ReadEventDataHandler();
        void InitConnection();
        void ResetMod();
        void GetExposureParameters();
        void SetGeneratorInUserMode();
        void SuppressErrorsVoltageAndAmpError();
        int StringToInteger(const QString& string,int index);
        QString ReadChannel();
        QString SafeWriteToChannel(const QString& cmd);

        int GetWarmupProgram();
        bool IsAnyError();
        bool ControlAndStartWarmup();







    };
}

#endif // GULMAYGENERATOR_H


