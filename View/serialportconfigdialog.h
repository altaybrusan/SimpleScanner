#ifndef SERIALPORTCONFIGDIALOG_H
#define SERIALPORTCONFIGDIALOG_H

#include <QDialog>
#include <QtSerialPort/QSerialPort>

namespace Ui {
    class GeneratorDialog;
}

class QIntValidator;

namespace SimpleScanner {

    class SerialPortConfigDialog : public QDialog
    {
        Q_OBJECT

    public:
        struct Settings {
            QString name;
            qint32 baudRate;
            QString stringBaudRate;
            QSerialPort::DataBits dataBits;
            QString stringDataBits;
            QSerialPort::Parity parity;
            QString stringParity;
            QSerialPort::StopBits stopBits;
            QString stringStopBits;
            QSerialPort::FlowControl flowControl;
            QString stringFlowControl;
            bool localEchoEnabled;
        };
        explicit SerialPortConfigDialog(QString filePath,QString section,QWidget *parent = 0);
        ~SerialPortConfigDialog();


    private slots:
        void showPortInfo(int idx);
        void Apply();
        void Cancel();
        void checkCustomBaudRatePolicy(int idx);
        void checkCustomDevicePathPolicy(int idx);

    private:
        Ui::GeneratorDialog *ui;
        Settings currentSettings;
        QString  _settingFile;
        QString  _fileSection;
        QIntValidator *intValidator;
        void fillPortsParameters();
        void fillPortsInfo();
        void updateSettings();
    };
}


#endif // SERIALPORTCONFIGDIALOG_H
