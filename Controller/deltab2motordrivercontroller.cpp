#include "deltab2motordrivercontroller.h"
#include "Model/deltab2motordriverconnector.h"
#include "View/deltab2motordriverpanelview.h"

namespace SimpleScanner
{
    /*! Controller object is doing boundary updating.
     * I am not sure if this is good or not?!
     * Maybe the object that construct these object
     * (for me MainWinow for now ) may be better responsible.
     * However, I think I have to keep all these parameters
     * in on location such that in future I can move them
     * into external setting file and inject it into the controller.
     */
    const static int MIN_SPEED=100;
    const static int MAX_SPEED=1500;
    const static qint16 INIT_SPEED=100;

    DeltaB2MotorDriverController::DeltaB2MotorDriverController(QObject *parent,
                                                               DeltaB2MotorDriverPanelView *view ,
                                                               DeltaB2MotorDriverConnector* connector) :
        QObject(parent),
        _view(*view),
        _connector(*connector)
    {
        InitializeConnector(MIN_SPEED,MAX_SPEED,INIT_SPEED);
        InitializeView(MIN_SPEED,MAX_SPEED,INIT_SPEED);
        Wireup();
    }

    void DeltaB2MotorDriverController::PrepareForShutdown()
    {
        _connector.StopMachine();
        _connector.UnloadMotor();
    }

    DeltaB2MotorDriverController::~DeltaB2MotorDriverController()
    {
//        _connector.deleteLater();
//        delete _view;
    }

    void DeltaB2MotorDriverController::Wireup()
    {
        connect(&_view,SIGNAL(NotifyMoveForward()),
                &_connector,SLOT(MoveForward()));
        connect(&_view,SIGNAL(NotifyMoveBackward()),
                &_connector,SLOT(MoveBackWard()));
        connect(&_view,SIGNAL(NotifyStop()),
                &_connector,SLOT(StopMachine()));
        connect(&_view,SIGNAL(NotifyUpdateSpeed(int)),
                &_connector,SLOT(UpdateSpeed(int)));


        connect(&_connector,&DeltaB2MotorDriverConnector::NotifyMotorDriverError,
                this,[this](const QString& message){
            emit NotifyActuatorError(message);
        });

        connect(&_connector,&DeltaB2MotorDriverConnector::NotifyMotorDriverError,
                this,[this](const QString& message){
            emit NotifyActuatorMessage(message);
        });
    }

    void DeltaB2MotorDriverController::InitializeView(const int& minSpeed,const int& maxSpeed,const int& currentSpeed)
    {
        _view.UpdateSpeedBoundaries(minSpeed,maxSpeed);
        _view.UpdateSpeed(currentSpeed);
    }

    void DeltaB2MotorDriverController::InitializeConnector(const int& minSpeed, const int& maxSpeed, const int& currentSpeed)
    {
        _connector.UpdateSpeedBoundaries(minSpeed,maxSpeed);
        _connector.UpdateInitialSpeed(currentSpeed);

    }
}
