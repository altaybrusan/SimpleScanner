#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSettings>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMetaEnum>
#include <QSettings>
#include <QLineEdit>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QEventLoop>
#include <qtconcurrentrun.h>
#include <QFileInfo>
#include <QMap>
#include "gulmaygeneratorconnector.h"



//Run on another thread
namespace SimpleScanner
{
    static const int WAIT_TIME=500;


    /*!
     * GulmayGeneratorConnector class provides a low level API for communicatig with Gulmay
     * generator. The generator is based on RS-232 communication protocol. The main duty
     * of the GulmayGeneratorConnector class is to handel the difficulties that are related to
     * multi-threading communication with the generator.
     * Settings of this protocol (such as baudrate, portname,etc.) are ssumed to be provided
     * by the clients of the class.
     */
    GulmayGeneratorConnector::GulmayGeneratorConnector(QObject *parent, QMap<QString,QString> *settings) :
        QThread(parent),
        quit(false),
        mutex(*new QMutex()),
        cond(*new QWaitCondition()),
        _settings(settings),
        //isWamrupCompleted(false),
        //connectionSettingFilePath(connectionSettingFilePath),
        emergencyStop(false)
    {

    }


    /*!
     * This is crucial to set quit the thread.
     */
    GulmayGeneratorConnector::~GulmayGeneratorConnector()
    {
        this->deleteLater();
        mutex.lock();
        quit = true;
        cond.wakeOne();
        mutex.unlock();
        wait();
    }

    void GulmayGeneratorConnector::run()
    {

        // STEP1: Make a connection objecct inside the worker thread
        QSerialPort *connection = new QSerialPort(this);
        // STEP2: Initialize the connection object
        InitConnection(connection,_settings);

        // STEP3: If you can not open the connection then COM adapter
        //        does not exists or not working properly
        if (!connection->open(QIODevice::ReadWrite))
        {
            // Set the end thread flag
            mutex.lock();
            quit = true;
            mutex.unlock();
            // inform the client-class that there is a serious connection problem
            emit NotifyConnectionError(QString("Can not connect to generator: %1").arg(connection->errorString()));
        }
        else
        {
            emit NotifySuccessfullConnectionEstablished();
        }


        // temporary var to hold the first command from the queue.
        QString currentRequest;

        // STEP4: while there is no quit command, keep the worker thread alive.
        while (!quit)
        {
            // Wait for new command to arraive.
            // REM: the worker thread plays the role of the consumer
            //      of the command queue. And also, the worker thread
            //      may lost some wake() signals of the producer thread
            //      so do not beg that when you receive a wake signal
            //      there is exactly one thread in the queue.
            mutex.lock();
            if(commandQueue.isEmpty())
                cond.wait(&mutex);
            int qsize= commandQueue.size();
            mutex.unlock();

            // if emergency stop is required immadiately execute
            // stop exposure and return ignor the query content;
            if(emergencyStop)
            {
                QString currentRequest = "!O\r";
                SendMessage(currentRequest, connection);
                mutex.lock();
                emergencyStop=false;
                // clear queue to clear all remaimaining exposure commands

                commandQueue.clear();
                mutex.unlock();
                continue;
            }


            // before the command is issud check if it is open or not
            // If the connection cable is disconnected then issue NotifyConnectionError
            if (connection->isOpen())
            {

                // Now, dequeue each command and send them to the generator.
                for(int i=0; i<qsize;i++)
                {
                    mutex.lock();
                    currentRequest  = commandQueue.dequeue();
                    mutex.unlock();

                    qDebug()<<"currentRequest "+currentRequest;


                    // write request
                    SendMessage(currentRequest, connection);
                }

            }
            else
            {
                /*! may be close and reopen connection is better idea*/
                emit NotifyConnectionError(connection->errorString());
            }
        }

        /*
         * If quit is equal true then this means the worker thread should be ended
         * As a house keeping task, we make sure that the connection is closed
         */

        qDebug()<<"Connection is closed";
        connection->close();
        //delete connection;
        //this->deleteLater();
        emit NotifyConnectionClosed();
    }

    /*!
     * Starts worker thread.
     * Except warming up this method prepare the generator.
     * As there is no exposre command, even if this method
     * is called multiple times, it is still exposure safe.
     * \note This method runs on UI thread.
     */
    void GulmayGeneratorConnector::StartGenerator()
    {
        // start worker thread for communicating with generator.
        if (!isRunning())
            start(); // start thread without creating event loop (check exec())

        //Step 1: Control Errors
        CheckAnyError();

        // Step 2: Turn off exposure
        StopExposure();

        //Step 5: suppress errors such as E18,E19
        SuppressErrorsVoltageAndAmpError();

        //Step 6: Set to manual Program mode
        SetGeneratorInUserMode();

        //        // Step 3: Control generator warmup and if not warmed then go through it
        //        StartWarmup(101);

        // Step 4: Get Exposure Parameters
        QueryExposureParameters();

        // Step 5: Get Focal spot size
        QueryFocalSpotSize();

        //Step 6: Get Generator mode
        QueryGeneratorMode();
    }

    /*!
      * Set the quit flag and force the worker thread to quite
      * the while loop, the first command after the loop is to close
      * the connection.
      * This method runs on UI thread.
     */
    void GulmayGeneratorConnector::DisconnectFromMachine()
    {
        mutex.lock();
        quit = true;
        cond.wakeOne();
        mutex.unlock();
    }

    /*!
     * Set the KVP value.
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::UpdateKvp(int newValue)
    {
        QString _val = IntToThreeDigitString(newValue);
        QString _cmd="!V"+_val+"\r";
        transaction(_cmd);
    }

    /*!
     * Set the Amp value.
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::UpdateAmp(int newValue)
    {
        QString _val = IntToThreeDigitString(newValue);
        QString _cmd="!I"+_val+"\r";
        transaction(_cmd);
    }

    /*!
     * Set the Exposure duration value.
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::UpdateShootTime(int newValue)
    {
        QString _val = IntToThreeDigitString(newValue);
        QString _cmd="!T"+_val+"\r";
        transaction(_cmd);
    }

    void GulmayGeneratorConnector::UpdateConnectionSettings(QMap<QString,QString> *parameters)
    {
        this->_settings = parameters;
    }

    /*!
     * Start Exposure
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::StartExposure()
    {
        QString _cmd="!X\r";
        transaction(_cmd);
    }

    /*!
     * Stop Exposure
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::StopExposure()
    {
        QString _cmd="!O\r";
        transaction(_cmd);
    }

    void GulmayGeneratorConnector::SetFocalSpotSizeSmall()
    {
        QString _cmd="!F\r";
        transaction(_cmd);
    }

    void GulmayGeneratorConnector::SetFocalSpotSizeLarg()
    {
        QString _cmd="!B\r";
        transaction(_cmd);
    }

    /*!
     * Close connection to generator
     */
    void GulmayGeneratorConnector::CloseConnection()
    {
        mutex.lock();
        quit=true;
        mutex.unlock();
    }


    void GulmayGeneratorConnector::StartShortTermWarmup()
    {
        QString _cmd =  "!P101\r";
        transaction(_cmd);

        _cmd="!X\r";
        transaction(_cmd);

        return;
    }

    void GulmayGeneratorConnector::StartLongTermWarmup()
    {
        QString _cmd =  "!P102\r";
        transaction(_cmd);

        _cmd="!X\r";
        transaction(_cmd);
    }


    /*!
     * Ignore all the commands inside the queue and run stop.
     * Be sure to receive NotifyExposureStoped signal.
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::EmergencyStop()
    {
        mutex.lock();
        // ignore all commands inside the queue.
        emergencyStop=true;
        cond.wakeOne();
        mutex.unlock();
    }

    /*!
     * Generator has four mode:
     * 0: No Activity
     * 1: key in position 2
     * 2: prewarming
     * 3: x-ray switching on or off
     * 4: x-ray on and exposure being logged.
     */
    void GulmayGeneratorConnector::QueryGeneratorMode()
    {
        QString _cmd="?M\r";
        transaction(_cmd);
    }

    /*!
     * Query the generator for exposure parameters, i.e. current KVP, mA, Sec
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::QueryExposureParameters()
    {
        QString _cmd="?X\r";
        transaction(_cmd);
    }


    /*!
     * This is an interface function to thread-safe enqueue
     * new commands
     */
    void GulmayGeneratorConnector::transaction(const QString &request)
    {
        QMutexLocker locker(&mutex);
        this->commandQueue.enqueue(request);
        cond.wakeOne();
    }


    /*!
     * Initialize the connection object for the worker thread.
     * This initialization includes: baudrate, portname,DataBits Stopbit.
     */
    void GulmayGeneratorConnector::InitConnection(QSerialPort *connection,QMap<QString,QString>* settings)
    {

        if(settings==nullptr)
        {
            emit NotifyConnectionError("Connection settings can not be empty."
                                       "Please firts update the connection settings");
            return;
        }
        //        if(!FileExists(settings))
        //        {
        //            emit NotifyConnectionSettingsError("Can not find connection setting file");
        //            mutex.lock();
        //            quit=true;
        //            mutex.unlock();
        //            return;
        //        }
        //        QString _SettingsFile = settings;
        //        QSettings settings(_SettingsFile, QSettings::IniFormat);



        //WHY THE PARAMETERS NAME ARE HARD CODED. IS IT GOOD IDEA?!
        auto _intVal0 = settings->value("Name","");
        qDebug()<< "Name: "<<settings->value("Name","");
        connection->setPortName(_intVal0);

        qDebug()<< "BaudRate: "<<settings->value("BaudRate","").toInt();
        connection->setBaudRate(settings->value("BaudRate","").toInt());

        QMetaEnum metaEnum = QMetaEnum::fromType<QSerialPort::DataBits>();
        auto _intVal1 =settings->value("DataBits","").toInt();
        QSerialPort::DataBits _dataBits = static_cast<QSerialPort::DataBits>(_intVal1);
        qDebug()<< "DataBits: " <<settings->value("DataBits","").toInt();
        connection->setDataBits(_dataBits);

        metaEnum = QMetaEnum::fromType<QSerialPort::Parity>();
        auto _intVal2 =settings->value("Parity","").toInt();
        QSerialPort::Parity _parity = static_cast<QSerialPort::Parity>(_intVal2);
        qDebug()<<"Parity: " <<settings->value("Parity","").toInt();
        connection->setParity(_parity);

        metaEnum = QMetaEnum::fromType<QSerialPort::StopBits>();
        auto _intVal3 =settings->value("StopBits","").toInt();
        QSerialPort::StopBits _stopBit = static_cast<QSerialPort::StopBits>(_intVal3);
        qDebug()<<"StopBits: " <<settings->value("StopBits","").toInt();
        connection->setStopBits(_stopBit);

        metaEnum = QMetaEnum::fromType<QSerialPort::FlowControl>();
        auto _intVal4 =settings->value("FlowControl","").toInt();
        QSerialPort::FlowControl _flowControl = static_cast<QSerialPort::FlowControl>(_intVal4);
        qDebug()<< "FlowControl: " <<settings->value("FlowControl","").toInt();
        connection->setFlowControl(_flowControl);
    }

    /*!
     * Process the generator responses and issue an corresponding signal
     */
    void GulmayGeneratorConnector::ProcessGeneratorResponse(QString& response)
    {
        emit NotifyGeneratorMessage("command: "+response.simplified());
        if(response.contains('M',Qt::CaseSensitive))
        {
            int _mod =StringToInteger(response,1);
            emit NotifyGeneratorMode(_mod);
        }
        if(response.contains("!O",Qt::CaseSensitive))
        {
            emit NotifyExposureStoped();
        }
        if(response.contains('V',Qt::CaseSensitive))
        {
            int _val= StringToInteger(response,1);
            emit NotifyGeneratorMessage(response);
            emit NotifyKvpValue(_val);
        }
        if(response.contains('I',Qt::CaseSensitive))
        {
            int _val= StringToInteger(response,1);
            emit NotifyGeneratorMessage(response);
            emit NotifyCurrentValue(_val);
        }
        if(response.contains("?T",Qt::CaseSensitive))
        {
            int _val= StringToInteger(response,1);
            emit NotifyEllapsedExposureTime(_val);
        }
        if(response.contains("!T",Qt::CaseSensitive))
        {
            int _val= StringToInteger(response,1);
            emit NotifyShootTime(_val);
        }
        if(response.contains("!X",Qt::CaseSensitive))
        {
            emit NotifyExposureStarted();
        }
        if(response.contains("E",Qt::CaseSensitive))
        {
            if(response == "?E\r")
            {
                // no error
            }
            else
            {
                // error happened
                StopExposure(); // Gulmay gen automatically do this!
                emit NotifyGeneratorError(response);
            }

        }
        if(response.contains("?X",Qt::CaseSensitive))
        {
            int _KVP=StringToInteger(response,1);
            int _mA=StringToInteger(response,2);
            int _Time=StringToInteger(response,3);
            emit NotifyKvpValue(_KVP);
            emit NotifyCurrentValue(_mA);
            emit NotifyShootTime(_Time);
        }
        if(response.contains("?W",Qt::CaseSensitive))
        {
            int _result= StringToInteger(response,1);
            switch (_result)
            {
            case 0:
                emit NotifyWarmupFinished();
                break;
            case 100:
                emit NotifyGeneratorMessage("Short warmup program (15 min) is required.");
                emit NotifyShortWarmupIsRequired();
                break;
            case 101:
                emit NotifyGeneratorMessage("Long warmup program (35 min) is required.");
                emit NotifyLongWarmupIsRequired();
                break;
            }
        }
        if(response.contains("?FB",Qt::CaseSensitive))
        {
            emit NotifyLargFocalSpotSizeTriggered();
        }
        if(response.contains("?FF",Qt::CaseSensitive))
        {
            emit NotifySmallFocalSpotSizeTriggered();
        }
    }

    /*!
     * Gulmay generator replies have special formats :
     * [COMMAND_SIGN|REQUEST_SIGN][PARAMETER_1_VALUE] (e.g. ?T000 or !T010)
     * [COMMAND_SIGN|REQUEST_SIGN][PARAMETER_1],[PARAMETER_1_VALUE] (e.g ?P888,1)
     * [COMMAND_SIGN|REQUEST_SIGN][PARAMETER_1_VALUE],[PARAMETER_2_VALUE],[PARAMETER_3_VALUE] (e.g ?X000,000,005)
     * This function extracts 'index-parameter2 from the generator 'reply-string'
     */
    int GulmayGeneratorConnector::StringToInteger(const QString &string, int index)
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


    /*!
     * Asks for most appropriate warmup program
     * 000: No warmup
     * 101: short wamup
     * 102: long warmup
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::QueryWarmupProgram()
    {
        QString _cmd="?W\r";
        transaction(_cmd);
        QueryExposureParameters();
    }

    void GulmayGeneratorConnector::QueryFocalSpotSize()
    {
        QString _cmd="?F\r";
        transaction(_cmd);
    }


//    bool GulmayGeneratorConnector::FileExists(QString path)
//    {
//        QFileInfo check_file(path);
//        // check if file exists and if yes:
//        //Is it really a file and no directory?
//        return check_file.exists() && check_file.isFile();
//    }

    /*!
     * In this mode user can adjust KVP,mA and Sec.
     * \note setting to an arbitrary exposure protocol (KVP,mA,Time)
     * DOES NOT necessary means those values will be applied during REAL exposure.
     * One should check the applied exposure by ?X command.
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::SetGeneratorInUserMode()
    {
        QString _cmd="!P002\r";
        transaction(_cmd);

    }

    void GulmayGeneratorConnector::SuppressErrorsVoltageAndAmpError()
    {
        // Engineering Flags on
        QString _cmd="!P902\r";
        transaction(_cmd);

        // Engineering Auto restart function on three times in five min
        _cmd="!P978,3,5\r";
        transaction(_cmd);

        // Just for debuging: check the Eninering flag
        _cmd="?P888\r";
        transaction(_cmd);

        // Just for debuging: count the retry to restart
        _cmd="?P978\r";
        transaction(_cmd);
    }

    /*!
     * Ask generator for any error
     * It runs on UI thread.
     */
    void GulmayGeneratorConnector::CheckAnyError()
    {
        QString _cmd="?E\r";
        transaction(_cmd);
    }

    /*!
     * Ask the generator how much time has been passed from exposure.
     */
    void GulmayGeneratorConnector::QueryEllapsedExposureTime()
    {
        QString _cmd="?T\r";
        transaction(_cmd);
    }

    /*!
     * Send message to serial port object and read the response.
     * It works on worker thread.
     */
    void GulmayGeneratorConnector::SendMessage(QString currentRequest, QSerialPort *connection)
    {
        QByteArray requestData = currentRequest.toUtf8();
        connection->write(requestData);
        if (connection->waitForBytesWritten(WAIT_TIME))
        {
            // read response
            if (connection->waitForReadyRead(WAIT_TIME)) {
                QByteArray responseData = connection->readAll();
                while (connection->waitForReadyRead(WAIT_TIME))
                    responseData += connection->readAll();

                QString response(responseData);
                ProcessGeneratorResponse(response);
            }
            else
            {

                emit NotifyConnectionError("Wait read response timeout");
                mutex.lock();
                quit=true;
                mutex.unlock();

            }

        }
        else
        {
            emit NotifyConnectionError("Wait write request timeout.");
            mutex.lock();
            quit=true;
            mutex.unlock();

        }
    }

    QString GulmayGeneratorConnector::IntToThreeDigitString(int& param)
    {
        QString _val = QString("%1").arg(param, 3, 10, QChar('0'));
        return _val;
    }

}

