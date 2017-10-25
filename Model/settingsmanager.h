#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>

class settingsManager : public QObject
{
    Q_OBJECT
public:
    explicit settingsManager(QObject *parent = nullptr);

signals:

public slots:
};

#endif // SETTINGSMANAGER_H