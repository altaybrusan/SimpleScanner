#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <QObject>
#include <QtSerialPort/QSerialPort>

namespace SimpleScanner
{
    class Actuator : public QObject
    {
        Q_OBJECT
    public:
        explicit Actuator(QObject *parent = nullptr);
        void StartMoving();
        void StopMoving();
        void UpdateSpeed(int speed);


    signals:

    public slots:
    private:
        QSerialPort *connection;
    };
}


#endif // ACTUATOR_H
