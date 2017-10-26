#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#include <QObject>

class DisplayController : public QObject
{
    Q_OBJECT
public:
    explicit DisplayController(QObject *parent = nullptr);

signals:

public slots:
};

#endif // DISPLAYCONTROLLER_H