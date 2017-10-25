#ifndef DELTAB2MOTORDRIVERCONTROLLER_H
#define DELTAB2MOTORDRIVERCONTROLLER_H

#include <QObject>
namespace SimpleScanner
{
    class DeltaB2MotorDriverConnector;
    class DeltaB2MotorDriverPanelView;
    class DeltaB2MotorDriverController : public QObject
    {
        Q_OBJECT
    public:
        explicit DeltaB2MotorDriverController(QObject *parent,
                                              DeltaB2MotorDriverPanelView *view ,
                                              DeltaB2MotorDriverConnector* connector);

         void PrepareForShutdown();
        ~DeltaB2MotorDriverController();

    signals:

        void NotifyActuatorError(const QString& message) const;
        void NotifyActuatorMessage(const QString& message) const;


    public slots:

    private:
        DeltaB2MotorDriverPanelView& _view;
        DeltaB2MotorDriverConnector& _connector;

        void Wireup();
        void InitializeView(const int& minSpeed,const int& maxSpeed,const int& currentSpeed);
        void InitializeConnector(const int &minSpeed, const int &maxSpeed, const int &currentSpeed);
    };

}


#endif // DELTAB2MOTORDRIVERCONTROLLER_H
