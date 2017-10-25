#include "gulmaygenerator.h"
#include <QMetaEnum>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>
#include <QLineEdit>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QEventLoop>
#include <qtconcurrentrun.h>



namespace SimpleScanner {
    GulmayGenerator::GulmayGenerator(QObject *parent):
        QObject(parent)
    {
        connection = new QSerialPort(this);
    }

    int GulmayGenerator::StringToInteger(const QString &string, int index)
    {

        QString _temp="0";
        switch (index) {

        case 1:
            _temp = string.split(",")[0].mid(2,3);
            break;
        case 2:
            _temp = string.split(",")[1];
            break;
        case 3:
            _temp = string.split(",")[2];
            break;
        default:
            break;
        }


        return _temp.toInt();

    }

    void GulmayGenerator::InitConnection()
    {
        _SettingsFile = QApplication::applicationDirPath() + "/genconnectionsettings.ini";

        QSettings settings(_SettingsFile, QSettings::IniFormat);

        auto _intVal0 = settings.value("Name","").toString();
        qDebug()<< "Name: "<<settings.value("Name","").toString();
        connection->setPortName(_intVal0);

        qDebug()<< "BaudRate: "<<settings.value("BaudRate","").toInt();
        connection->setBaudRate(settings.value("BaudRate","").toInt());

        QMetaEnum metaEnum = QMetaEnum::fromType<QSerialPort::DataBits>();
        auto _intVal1 =settings.value("DataBits","").toInt();
        QSerialPort::DataBits _dataBits = static_cast<QSerialPort::DataBits>(_intVal1);
        qDebug()<< "DataBits: " <<settings.value("DataBits","").toInt();
        connection->setDataBits(_dataBits);

        metaEnum = QMetaEnum::fromType<QSerialPort::Parity>();
        auto _intVal2 =settings.value("Parity","").toInt();
        QSerialPort::Parity _parity = static_cast<QSerialPort::Parity>(_intVal2);
        qDebug()<<"Parity: " <<settings.value("Parity","").toInt();
        connection->setParity(_parity);

        metaEnum = QMetaEnum::fromType<QSerialPort::StopBits>();
        auto _intVal3 =settings.value("StopBits","").toInt();
        QSerialPort::StopBits _stopBit = static_cast<QSerialPort::StopBits>(_intVal3);
        qDebug()<<"StopBits: " <<settings.value("StopBits","").toInt();
        connection->setStopBits(_stopBit);

        metaEnum = QMetaEnum::fromType<QSerialPort::FlowControl>();
        auto _intVal4 =settings.value("FlowControl","").toInt();
        QSerialPort::FlowControl _flowControl = static_cast<QSerialPort::FlowControl>(_intVal4);
        qDebug()<< "FlowControl: " <<settings.value("FlowControl","").toInt();
        connection->setFlowControl(_flowControl);

        timer = new QTimer(this);
        Wireup();


    }

    void GulmayGenerator::ResetMod()
    {

        QString _cmd="?M\r";
        //        connection->write(_cmd.toUtf8());
        //        QString _msg= ReadPort();
        QString _msg = SafeWriteToChannel(_cmd);
        int _mod =StringToInteger(_msg,1);
        if(_mod!=0)
        {
            _cmd="!O\r";
            //connection->write(_cmd.toUtf8());
            SafeWriteToChannel(_cmd);
        }

    }


    void GulmayGenerator::StartMachine()
    {
        InitConnection();

        if (connection->open(QIODevice::ReadWrite))
        {
            //emit NotifyConnectionIsOpned("Connection to generator is opened");
            //Step 1: Control Errors
            if(!IsAnyError())
            {
                //Step 1: Reset Mode
                ResetMod();
                //Step 2: Control warm up
                if(ControlAndStartWarmup())
                {
                    emit NotifyWarmupFinished();
                    //Step 3: Update Exposure Parameters on the screen
                    GetExposureParameters();

                    //Step 4: suppress errors such as E18,E19
                    SuppressErrorsVoltageAndAmpError();

                    //Step 5: Set to manual Program mode
                    SetGeneratorInUserMode();
                }
                else
                {
                    emit NotifyWarmupFailed("Warmup failed");
                }
            }
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Can not open connection.");
            msgBox.setInformativeText("Please check the device connection.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.exec();

            emit NotifyErrorMessage(connection->errorString());
        }

    }

    void GulmayGenerator::StopMachine()
    {
        if (connection->isOpen())
        {
            connection->close();

            //emit NotifyConnectionIsClosed("Connection to generator is closed");
        }
    }

    void GulmayGenerator::StopExposureFast()
    {
        QString cmd="!O\r";
        connection->write(cmd.toUtf8());
    }

    //    void GulmayGenerator::StopExposureAsync()
    //    {
    //        QFuture<void> future = QtConcurrent::run(this,&GulmayGenerator::StopExposure);


    //    }

    void GulmayGenerator::UpdateKvp(const QString &newValue)
    {
        QString _cmd="!V"+newValue+"\r";
        //qDebug()<<"The KVP: "<<_kvp;
        //connection->write(_kvp.toUtf8());
        QString _res =  SafeWriteToChannel(_cmd);

        int _val= StringToInteger(_res,1);
        emit NotifyMessage(_res);
        emit NotifyKvpValue(_val);

    }
    void GulmayGenerator::UpdateAmp(const QString &newValue)
    {
        QString _cmd="!I"+newValue+"\r";
        //connection->write(_kvp.toUtf8());

        QString _res =  SafeWriteToChannel(_cmd);

        int _val= StringToInteger(_res,1);
        emit NotifyMessage(_res);
        emit NotifyAmpValue(_val);


    }
    void GulmayGenerator::UpdateShootTime(const QString &newValue)
    {
        QString _cmd="!T"+newValue+"\r";
        //connection->write(_kvp.toUtf8());


        QString _res =  SafeWriteToChannel(_cmd);
        int _val= StringToInteger(_res,1);
        emit NotifyMessage(_res);
        emit NotifyShootTime(_val);


    }

    void GulmayGenerator::StartExposure()
    {
        QString _cmd="!X\r";
        SafeWriteToChannel(_cmd);

    }

    void GulmayGenerator::StopExposure()
    {
        QString _cmd="!O\r";
        SafeWriteToChannel(_cmd);
    }
    void GulmayGenerator::GetExposureParameters()
    {
        QString _cmd="?X\r";
        QString _res=SafeWriteToChannel(_cmd);

        _KVP=StringToInteger(_res,1);
        _mA=StringToInteger(_res,2);
        _Time=StringToInteger(_res,3);
        emit NotifyKvpValue(_KVP);
        emit NotifyAmpValue(_mA);
        emit NotifyShootTime(_Time);
    }

    void GulmayGenerator::SetGeneratorInUserMode()
    {
        QString _cmd="!P002\r";
        SafeWriteToChannel(_cmd);

    }

    void GulmayGenerator::SuppressErrorsVoltageAndAmpError()
    {
        // Engineering Flags on
        QString _cmd="!P902\r";
        SafeWriteToChannel(_cmd);

        // Engineering Auto restart function on three times in five min
        _cmd="!P978,3,5\r";
        SafeWriteToChannel(_cmd);

        // Just for debuging: check the Eninering flag
        _cmd="?P888\r";
        SafeWriteToChannel(_cmd);

        // Just for debuging: count the retry to restart
        _cmd="?P978\r";
        SafeWriteToChannel(_cmd);
    }

    int GulmayGenerator::GetWarmupProgram()
    {
        QString _cmd="?W\r";
        //connection->write(_cmd.toLocal8Bit());
        int _result= StringToInteger(SafeWriteToChannel(_cmd),1);
        switch (_result)
        {
        case 100:
            emit NotifyMessage("Short-warmup program (15 min).");
            break;
        case 101:
            emit NotifyMessage("Long-warmup-program (35 min).");
            break;
        default:
            emit NotifyMessage("No warmup is required.");
            break;
        }
        return _result;
    }

    bool GulmayGenerator::ControlAndStartWarmup()
    {
        int _wprog=GetWarmupProgram();

        if(_wprog != 0)
        {
            QString _cmd =  "!P"+ QString::number(_wprog)+"\r";

            //connection->write(_cmd.toLocal8Bit());
            //ReadPort();
            SafeWriteToChannel(_cmd);

            if(IsAnyError())
            {
                return false;
            }

            _cmd="!X\r";

            //connection->write(_cmd.toLocal8Bit());
            //ReadPort();
            SafeWriteToChannel(_cmd);

            timer->setInterval(4000);
            timer->start(4000);
            _warmuploop.exec();

            ReadChannel();
            if(IsAnyError())
            {
                return false;
            }

        }

        return true;
    }

    bool GulmayGenerator::IsAnyError()
    {
        _Errors.clear();

        QString _cmd="?E\r";
        QString _result;
        connection->write(_cmd.toLocal8Bit());
        _result=ReadChannel();
        if(_result != "?E")
        {
            _Errors=_result;
            emit NotifyErrorMessage(_result);
            return true;
        }

        return false;
    }




    void GulmayGenerator::Wireup()
    {
        //connect(connection, &QSerialPort::readyRead, this, &GulmayGenerator::ReadEventDataHandler);
        connect(timer, SIGNAL(timeout()), this, SLOT(WarmupRemainingTime()));
        connect(this, SIGNAL(NotifyWarmupFinished()), &_warmuploop, SLOT(quit()));


    }
    void GulmayGenerator::Unwire()
    {
        //disconnect(connection, &QSerialPort::readyRead, this, &GulmayGenerator::ReadEventDataHandler);
        disconnect(timer, SIGNAL(timeout()), this, SLOT(RemainingTime()));
    }

    void GulmayGenerator::WarmupRemainingTime()
    {
        qDebug()<<"Warmup timer is triggered";
        QString _cmd="?T\r";
        //connection->write(_cmd.toUtf8());
        QString result = SafeWriteToChannel(_cmd);

        //QString result = ReadPort();
        int _remTime=StringToInteger(result,1);

        // Check is there any error
        if(IsAnyError())
        {
            timer->stop();
            _cmd="!O\r";
            connection->write(_cmd.toUtf8());

            emit NotifyWarmupFailed("Failed to warmup");
            return;
        }

        // Check if there is any error
        if(_remTime==0)
        {
            timer->stop();
            emit NotifyWarmupFinished();
        }

        emit NotifyMessage(result);
    }

    //    void GulmayGenerator::ReadEventDataHandler()
    //    {

    //                if(!data.contains("\r"))
    //                {
    //                    charBuffer.append(data);
    //                }
    //                if(data.contains(")"))
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    emit NotifyErrorMessage(_string);
    //                }
    //                if(charBuffer.contains("X")&& charBuffer.length()==13)
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    _KVP=StringToInteger(_string,1);
    //                    _mA=StringToInteger(_string,2);
    //                    _Time=StringToInteger(_string,3);
    //                    emit NotifyKvpValue(_KVP);
    //                    emit NotifyAmpValue(_mA);
    //                    emit NotifyShootTime(_Time);
    //                }
    //                if(charBuffer.contains("P")&& charBuffer.length()==5)
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    emit NotifyMessage(_string);
    //                }
    //                if(charBuffer.contains("I")&& charBuffer.length()==5)
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    emit NotifyMessage(_string);
    //                }
    //                if(charBuffer.contains("V")&& charBuffer.length()==5)
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    int _val= StringToInteger(_string,1);
    //                    emit NotifyMessage(_string);
    //                    emit NotifyKvpValue(_val);

    //                }
    //                if(charBuffer.contains("T")&& charBuffer.length()==5)
    //                {
    //                    QString _string = QString(charBuffer);
    //                    charBuffer.clear();
    //                    emit NotifyMessage(_string);
    //                }
    //        if(charBuffer.contains("E") && charBuffer.length()==2)
    //        {
    //            QString _string = QString(charBuffer);
    //            //charBuffer.clear();
    //            emit NotifyErrorDetected(_string);
    //            return;
    //        }
    //        if(charBuffer.contains("W")&& charBuffer.length()==5)
    //        {
    //            //Write a warm up procedure...
    //            QString _string = QString(charBuffer);
    //            int program= StringToInteger(_string,1);
    //            charBuffer.clear();
    //            emit NotifyWarmupResult(program);
    //            return;
    //        }

    //    }
    QString GulmayGenerator::ReadChannel()
    {
        //Unwire();
        QByteArray data;

        while (connection->waitForReadyRead(500))
        {
            data = connection->readAll();
            if(!data.contains("\r"))
            {
                charBuffer.append(data);
            }
        }

        QString _message = QString(charBuffer);
        qDebug()<<"The received message is: "<<_message;
        charBuffer.clear();
        //Wireup();

        return _message;
    }

    QString GulmayGenerator::SafeWriteToChannel(const QString &cmd)
    {
        if(!IsAnyError())
        {
            connection->write(cmd.toUtf8());
        }
        //        QFuture<QString> future = QtConcurrent::run(this,&GulmayGenerator::ReadChannel);
        //        QString _result = future.result();
        QString _result= ReadChannel();

        return _result;
    }
}
