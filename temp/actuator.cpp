#include "actuator.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMetaEnum>
#include <QApplication>
#include <QSettings>
#include <QDebug>

namespace SimpleScanner {
    Actuator::Actuator(QObject *parent) : QObject(parent)
    {
        connection = new QSerialPort(this);

        QString _SettingsFile = QApplication::applicationDirPath() + "/actuatorsettings.ini";

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

//        timer = new QTimer(this);
//        Wireup();

    }

    void Actuator::StartMoving()
    {

    }

    void Actuator::StopMoving()
    {

    }

    void Actuator::UpdateSpeed(int speed)
    {

    }
}

