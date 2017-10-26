#include "settingsmanager.h"
#include <QSettings>
#include <QApplication>
#include  "utils.h"

namespace SimpleScanner
{
    static QString _settingsPath(QApplication::applicationDirPath() + "/settings.ini");
    static QSettings* _settings(new QSettings(_settingsPath, QSettings::IniFormat));
    SettingsManager::SettingsManager(QObject *parent) :
        QObject(parent)
    {

    }

    bool SettingsManager::IsSettingsValid()
    {

        return Utils::FileExists(_settingsPath);
    }

    QString SettingsManager::GetSettingFilePath()
    {
        return _settingsPath;
    }

    QMap<QString, QString>* SettingsManager::GetGeneratorSettings()
    {
        auto _settingsMap = new QMap<QString,QString>();
        _settings->beginGroup("GEN");
        QStringList childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();

        return _settingsMap;

    }

    QMap<QString, QString>* SettingsManager::GetActuatorSettings()
    {

        auto _settingsMap= new QMap<QString,QString>();
        _settings->beginGroup("ACT");
        QStringList childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();
        return _settingsMap;
    }


    /*!
        QSettings* _settings = new QSettings(_settingsPath, QSettings::IniFormat);
        QMap<QString,QString>* _settingsMap= new QMap<QString,QString>();
        _settings->beginGroup("ACT");
        QStringList childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();
        _machineConnector.UpdateConnectionSettings(_settingsMap);
        _settingsMap= new QMap<QString,QString>();
        _settings->beginGroup("GEN");
        childKeys = _settings->childKeys();
        foreach (const QString &childKey, childKeys)
        {
            _settingsMap->insert( childKey, _settings->value(childKey).toString() );
        }
        _settings->endGroup();

*/

}

