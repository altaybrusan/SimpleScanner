#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QMap>

class QSettings;

namespace SimpleScanner
{
    class SettingsManager : public QObject
    {
        Q_OBJECT
    public:
        explicit SettingsManager(QObject *parent = nullptr);
        static bool IsSettingsValid();
        static QString GetSettingFilePath();
        static QMap<QString,QString>* GetGeneratorSettings();
        static QMap<QString, QString>* GetActuatorSettings();


    signals:

    public slots:
    private:

    };
}


#endif // SETTINGSMANAGER_H
