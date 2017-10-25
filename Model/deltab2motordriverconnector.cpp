#include "deltab2motordriverconnector.h"
#include <QDebug>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>
#include <QSettings>
#include <QMetaEnum>
#include <QMap>

namespace SimpleScanner
{
    enum MOVE_STAT {
        FORWARD,
        STOP,
        BACKWARD
    } _moveStat ;
    const static int SLAVE_ADDRESS = 127;

    /*!
     * The actuators are connected to the workstation machin via DELTA-B2
     * motor driver. The cmmunication protocol is
     * MOD-BUS RTU on serial line.
     * This class is designed to handel the communication
     * to this driver JUST FOR THIS PROJECT. Its is specific to
     * handel this problem and with no way it should not be concidered
     * as a general solution.
     *
     * A critical problem: When I close the main window I beg the devie to
     * stop the mechanics. but I found that the device did not stop. The reason
     * of this is that I beg the destructor let amount of time for the
     * DeltaB2MotorDriverConnector object to write into the registers.
     * Howerver, The destructor immadiately close up and do not let the
     * object to wait for responses.
     * At the moment I switch this issue into main window. I handel
     * the close event. When a close event is received it first inform the
     * controller to finishup then shuttdown the program.
     * */

    DeltaB2MotorDriverConnector::DeltaB2MotorDriverConnector(QObject *parent) :
        QObject(parent),
        _settings(nullptr)
    {
        ResetConnectionLink();

        if (device != nullptr)
        {
            emit NotifyMotorDriverMessage( QString("Motor state: %1").arg(QString::number(device->state())));
        }
        else
        {
            QString _message = device->state();
            emit NotifyMotorDriverError( _message );
        }
        _moveStat= MOVE_STAT::STOP;
    }

    void DeltaB2MotorDriverConnector::StopMachine()
    {
        if (!device)
            return;
        if (device->state() != QModbusDevice::ConnectedState)
        {
            Connect();
        }
        Stop();
        _moveStat=MOVE_STAT::STOP;
        //emit NotifyMachineIsReady();
    }
    void DeltaB2MotorDriverConnector::UnloadMotor()
    {
        if (!device)
            return;
        if (device->state() != QModbusDevice::ConnectedState)
        {
            Connect();
        }
        Stop();
        ServoOff();
    }
    void DeltaB2MotorDriverConnector::MoveForward()
    {
        if (!device)
            return;

        if (device->state() != QModbusDevice::ConnectedState)
        {
            Connect();
        }
        ServoOn();
        Move(4999);
        _moveStat=MOVE_STAT::FORWARD;
        emit NotifyMachineIsMoving();
    }
    void DeltaB2MotorDriverConnector::MoveBackWard()
    {
        if (!device)
            return;

        if (device->state() != QModbusDevice::ConnectedState)
        {
            Connect();
        }
        ServoOn();
        Move(4998);
        _moveStat=MOVE_STAT::BACKWARD;
        emit NotifyMachineIsMoving();
    }

    void DeltaB2MotorDriverConnector::UpdateConnectionSettings(QMap<QString,QString> *settings)
    {
        _settings= settings;
    }
    void DeltaB2MotorDriverConnector::UpdateSpeedBoundaries(const int& min,const  int& max)
    {
        MIN_SPEED=min;
        MAX_SPEED=max;
    }
    void DeltaB2MotorDriverConnector::UpdateInitialSpeed(const int &speed)
    {
        INIT_SPEED= speed;
    }

    DeltaB2MotorDriverConnector::~DeltaB2MotorDriverConnector()
    {
        /*! super critical to check if
         * connectionsettings is not null
         * Test: open and close program without
         * pressing any button */
        // !device: is device pointer null

        if (!(!device)) //&& !connectionSettings))
        {
            UnloadMotor();
            //device->disconnectDevice();
        }
        //delete device;

    }

    void DeltaB2MotorDriverConnector::UpdateSpeed(int value)
    {

        if (!device)
            return;
        if(value>MAX_SPEED || value<MIN_SPEED)
            return;

        if (device->state() != QModbusDevice::ConnectedState)
        {
            Connect();
        }

        WriteRegister(1034,value);
        ReadRegister(1034);
        switch (_moveStat) {
        case MOVE_STAT::FORWARD:
            MoveForward();
            break;
        case MOVE_STAT::BACKWARD:
            MoveBackWard();
            break;
        case MOVE_STAT::STOP:
            StopMachine();
            break;
        default:
            break;
        }
        //emit NotifyMachineIsMoving();

    }
    void DeltaB2MotorDriverConnector::ReadRegister(int registerAddress)
    {
        QVector<quint16> readBuffer(1);
        QModbusDataUnit adu(QModbusDataUnit::HoldingRegisters,registerAddress, readBuffer);

        if (auto *reply = device->sendReadRequest(adu, SLAVE_ADDRESS))
        {

            if (!reply->isFinished())
            {
                connect(reply, &QModbusReply::finished,this, &DeltaB2MotorDriverConnector::readReady);
            }
            else
                delete reply; // broadcast replies return immediately
        }
        else
        {
            QString _errorMsg=device->errorString();
            emit NotifyMotorDriverError(_errorMsg);
        }

    }
    void DeltaB2MotorDriverConnector::WriteRegister(int registerAddress, quint16 value)
    {
        QVector<quint16> writeBuffer(1);
        writeBuffer[0] = value;
        QModbusDataUnit adu(QModbusDataUnit::HoldingRegisters, registerAddress, writeBuffer);


        if (auto *reply = device->sendWriteRequest(adu, SLAVE_ADDRESS))
        {

            if (!reply->isFinished())
            {
                connect(reply, &QModbusReply::finished, this,&DeltaB2MotorDriverConnector::writeReady);
            }
            else
            {
                delete reply; // broadcast replies return immediately
            }

        }
        else
        {
            QString _errorMsg=device->errorString();
            emit NotifyMotorDriverError(_errorMsg);
        }

    }
    void DeltaB2MotorDriverConnector::ServoOn()
    {
        if (!device)
            return;
        // write servo-on register on P2-30
        WriteRegister(572,1);
        emit NotifyMachineIsReady();

    }
    void DeltaB2MotorDriverConnector::ServoOff()
    {
        if (!device)
            return;
        // write servo-on register on P2-30
        WriteRegister(572,0);
        emit NotifyMotorIsUnloaded();

    }
    void DeltaB2MotorDriverConnector::readReady()
    {
        auto reply = qobject_cast<QModbusReply *>(sender());
        if (!reply)
            return;

        if (reply->error() == QModbusDevice::NoError) {
            const QModbusDataUnit unit = reply->result();
            QString _address=unit.startAddress();

            for (uint i = 0; i < unit.valueCount(); i++)
            {
                const QString _address= QString::number(unit.startAddress());
                const QString _value=QString::number(unit.value(i));
                emit NotifyRegisterValue(_address,_value);
            }

        }
        else if (reply->error() == QModbusDevice::ProtocolError)
        {
            qDebug()<<(tr("Read response error: %1 (Mobus exception: 0x%2)").
                       arg(reply->errorString()).
                       arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
        }
        else
        {
            qDebug()<<"Read response error: "+ reply->errorString() + "(code:)" + reply->error();

        }

        reply->deleteLater();

    }
    void DeltaB2MotorDriverConnector::writeReady()
    {
        auto reply = qobject_cast<QModbusReply *>(sender());
        if (!reply)
            return;

        if (reply->error() == QModbusDevice::ProtocolError) {
            QString _errorMsg = "Write response error: " + reply->errorString()
                    + " (Mobus exception: " + reply->rawResult().exceptionCode()+")";
            ResetConnectionLink();
            emit NotifyMotorDriverError(_errorMsg);

        }
        else if (reply->error() != QModbusDevice::NoError)
        {

            QString _errorMsg = QString("Write response error: %1 ( code: %2 )").arg( reply->errorString()).arg(QString::number(reply->error()));
            ResetConnectionLink();
            emit NotifyMotorDriverError(_errorMsg);
        }
        reply->deleteLater();
    }


    void DeltaB2MotorDriverConnector::Move(quint16 direction)
    {

        if (!device)
            return;
        if(direction == 4999 || direction== 4998)
        {
            // Write on register P4-05 with address 1034
            WriteRegister(1034,direction);

        }


    }
    void DeltaB2MotorDriverConnector::Stop()
    {
        if (!device)
            return;

        // Write on register P4-05 with address 1034
        WriteRegister(1034,0);

    }
    void DeltaB2MotorDriverConnector::Connect()
    {
        if (!device)
            return;

        // Set connection parameters.
        if (device->state() != QModbusDevice::ConnectedState)
        {
             device->setConnectionParameter(QModbusDevice::SerialPortNameParameter, _settings->value("Name",""));

            QMetaEnum metaEnum = QMetaEnum::fromType<QSerialPort::DataBits>();
            auto _val =_settings->value("DataBits","").toInt();
            QSerialPort::DataBits _dataBits = static_cast<QSerialPort::DataBits>(_val);
            device->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,_dataBits);

            metaEnum = QMetaEnum::fromType<QSerialPort::BaudRate>();
            _val =_settings->value("BaudRate","").toInt();
            QSerialPort::BaudRate _baudRate = static_cast<QSerialPort::BaudRate>(_val);
            device->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,_baudRate);


            metaEnum = QMetaEnum::fromType<QSerialPort::Parity>();
            _val = _settings->value("Parity","").toInt();
            QSerialPort::Parity _parity = static_cast<QSerialPort::Parity>(_val);
            device->setConnectionParameter(QModbusDevice::SerialParityParameter,_parity);

            metaEnum = QMetaEnum::fromType<QSerialPort::StopBits>();
            _val =_settings->value("StopBits","").toInt();
            QSerialPort::StopBits _stopBit = static_cast<QSerialPort::StopBits>(_val);
            device->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,_stopBit);
        }

        device->setTimeout(25);
        device->setNumberOfRetries(3);

        // now connect to the device
        if(device->connectDevice())
        {
            //QString _mechineError=device->state();
            WriteRegister(1034,INIT_SPEED);
            emit NotifyMachineIsConnected();
        }


    }

    // If error in connection happens (e.g. cable disconnects)
    // recreate device object to restart connection if
    // cable repluged
    void DeltaB2MotorDriverConnector::ResetConnectionLink()
    {
        if(!device)
        {
            device->disconnectDevice();
            delete device;
        }
        device = new QModbusRtuSerialMaster();
    }



}
