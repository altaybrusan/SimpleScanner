#include "View/serialportconfigdialog.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QSettings>
#include <QDebug>
#include <QMetaEnum>
#include <ui_serialportconfigdialog.h>


namespace SimpleScanner {

    static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

    SerialPortConfigDialog::SerialPortConfigDialog(QString filePath,
                                                   QString section, QWidget *parent) :
        QDialog(parent),
        _settingFile(filePath),
        _fileSection(section),
        ui(new Ui::GeneratorDialog)
    {
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        this->setFixedSize(280,290);

        ui->setupUi(this);
        intValidator = new QIntValidator(0, 4000000, this);

        ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);

        connect(ui->applyButton, &QPushButton::clicked,
                this, &SerialPortConfigDialog::Apply);
        connect(ui->cancelBtn, &QPushButton::clicked,
                this, &SerialPortConfigDialog::Cancel);
        connect(ui->serialPortInfoListBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &SerialPortConfigDialog::showPortInfo);
        connect(ui->baudRateBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &SerialPortConfigDialog::checkCustomBaudRatePolicy);
        connect(ui->serialPortInfoListBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &SerialPortConfigDialog::checkCustomDevicePathPolicy);

        fillPortsParameters();
        fillPortsInfo();

        updateSettings();

    }

    void SerialPortConfigDialog::showPortInfo(int idx)
    {
        if (idx == -1)
            return;

        QStringList list = ui->serialPortInfoListBox->itemData(idx).toStringList();
    }
    void SerialPortConfigDialog::Apply()
    {
        updateSettings();

        QSettings settings(_settingFile, QSettings::IniFormat);
        settings.beginGroup(_fileSection);

        settings.setValue("Name",currentSettings.name);
        settings.setValue("BaudRate",currentSettings.baudRate);
        settings.setValue("DataBits",currentSettings.dataBits);
        settings.setValue("Parity",currentSettings.parity);
        settings.setValue("StopBits",currentSettings.stopBits);
        settings.setValue("FlowControl",currentSettings.flowControl);

        settings.endGroup();
        this->close();
    }
    void SerialPortConfigDialog::Cancel()
    {
        this->close();
    }
    void SerialPortConfigDialog::checkCustomBaudRatePolicy(int idx)
    {
        bool isCustomBaudRate = !ui->baudRateBox->itemData(idx).isValid();
        ui->baudRateBox->setEditable(isCustomBaudRate);
        if (isCustomBaudRate) {
            ui->baudRateBox->clearEditText();
            QLineEdit *edit = ui->baudRateBox->lineEdit();
            edit->setValidator(intValidator);
        }
    }
    void SerialPortConfigDialog::checkCustomDevicePathPolicy(int idx)
    {
        bool isCustomPath = !ui->serialPortInfoListBox->itemData(idx).isValid();
        ui->serialPortInfoListBox->setEditable(isCustomPath);
        if (isCustomPath)
            ui->serialPortInfoListBox->clearEditText();
    }

    void SerialPortConfigDialog::fillPortsParameters()
    {
        ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
        ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
        ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
        ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);


        ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
        ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
        ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
        ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
        ui->dataBitsBox->setCurrentIndex(3);

        ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
        ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
        ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
        ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
        ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

        ui->stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
        ui->stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
        ui->stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

        ui->flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
        ui->flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
        ui->flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    }
    void SerialPortConfigDialog::fillPortsInfo()
    {
        ui->serialPortInfoListBox->clear();
        QString description;
        QString manufacturer;
        QString serialNumber;
        const auto infos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &info : infos) {
            QStringList list;
            description = info.description();
            manufacturer = info.manufacturer();
            serialNumber = info.serialNumber();
            list << info.portName()
                 << (!description.isEmpty() ? description : blankString)
                 << (!manufacturer.isEmpty() ? manufacturer : blankString)
                 << (!serialNumber.isEmpty() ? serialNumber : blankString)
                 << info.systemLocation()
                 << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
                 << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

            ui->serialPortInfoListBox->addItem(list.first(), list);
        }

    }

    void SerialPortConfigDialog::updateSettings()
    {
        currentSettings.name = ui->serialPortInfoListBox->currentText();

        if (ui->baudRateBox->currentIndex() == 4) {
            currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
        } else {
            currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                        ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
        }
        currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

        currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                    ui->dataBitsBox->itemData(ui->dataBitsBox->currentIndex()).toInt());
        currentSettings.stringDataBits = ui->dataBitsBox->currentText();

        currentSettings.parity = static_cast<QSerialPort::Parity>(
                    ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
        currentSettings.stringParity = ui->parityBox->currentText();

        currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                    ui->stopBitsBox->itemData(ui->stopBitsBox->currentIndex()).toInt());
        currentSettings.stringStopBits = ui->stopBitsBox->currentText();

        currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                    ui->flowControlBox->itemData(ui->flowControlBox->currentIndex()).toInt());
        currentSettings.stringFlowControl = ui->flowControlBox->currentText();


    }

    SerialPortConfigDialog::~SerialPortConfigDialog()
    {
        delete ui;
    }
}

