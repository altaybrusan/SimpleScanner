#ifndef MACHINARY_H
#define MACHINARY_H

#include <QObject>
#include <QMap>

class QSerialPort;
class QModbusRtuSerialMaster;
class QSettings;


namespace SimpleScanner
{
    class DeltaB2MotorDriverConnector : public QObject
    {
        Q_OBJECT

    public:
        explicit DeltaB2MotorDriverConnector(QObject *parent = nullptr);
        void UpdateConnectionSettings(QMap<QString, QString> *settings);
        void UpdateSpeedBoundaries(const int &min, const int &max);
        void UpdateInitialSpeed(const int &speed);

        ~DeltaB2MotorDriverConnector();

    signals:
        void NotifyMachineIsConnected();
        void NotifyMachineIsReady();
        void NotifyMotorIsUnloaded();
        void NotifyMachineIsMoving();
        void NotifyMotorDriverError(const QString& message);
        void NotifyMotorDriverMessage(const QString& message);
        void NotifyRegisterValue(const QString& registerAddress,const QString& registerValue);

    public slots:
        void UpdateSpeed(int value);
        void StopMachine();
        void UnloadMotor();
        void MoveForward();
        void MoveBackWard();

    private:
        QModbusRtuSerialMaster *device;
        QMap<QString,QString> *_settings;
        int MAX_SPEED;
        int MIN_SPEED;
        int INIT_SPEED;
        void ReadRegister(int registerAddress);
        void WriteRegister(int registerAddress, quint16 value);
        void Move(quint16 direction);
        void ServoOn();
        void ServoOff();
        void readReady();
        void writeReady();
        void Stop();
        void Connect();
        void ResetConnectionLink();
    };


}


#endif // MACHINARY_H
