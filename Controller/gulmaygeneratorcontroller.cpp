#include "Controller/gulmaygeneratorcontroller.h"
#include "View/gulmaygeneratorpanelview.h"
#include "Model/gulmaygeneratorconnector.h"
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

namespace SimpleScanner
{

    /*! The generator is a simple device
     * instead of state design pattern, I prefer the enum technique.
     * The main application of this state variable is to memorize the current state
     * inorder to handel the STOP command. When a stop command is received, it
     * returns the device into the previous state.
     * You can see the is if inside the OnExposureStoped() that makes
     * great deal of the _state variable.
     */
    enum GENERATOR_STATE {
        NOT_CONNECTED,
        CONNECTED,
        CHECKING_WARMUP,
        READY_FOR_SHORT_WARMUP,
        READY_FOR_LONG_WARMUP,
        SHORT_TERM_WARMING_UP,
        LONG_TERM_WARMING_UP,
        READY,
        SHOOTING,        
        GENERATOR_ERROR,
        CONNECTION_ERROR
    } _state ;


    /*! Boundry checking and data validation is done via controller class.
     * The model class has no idea about the detials. For now lets ASSUME this
     * is correct way and keep going. For example I may read them from a setting file.
     * Why not? For different generators different boundries may applied.*/
    const static int KVP_MIN=1;
    const static int KVP_MAX=160;
    const static int KVP_INIT=80;

    const static int AMP_MIN=1;
    const static int AMP_MAX=250;
    const static int AMP_INIT=250;

    const static int TIME_MIN=1;
    const static int TIME_MAX=999;
    const static int TIME_INIT=5;

    const static int MAX_EXPOSURE_PARAM_VAL=999;

    static const int TIMER_INTERVAL=4000;
    /*
     * A timer is required to periodically check the device sitation.
     * The timer needs QEventLoop. Our thread does not need this loop
     * So we start it on the UI thread which has this loop.
     * This is not good practice to relay on the caller loop but
     * we use it for now. For next iterations please push it into worker thread
     */
    GulmayGeneratorController::GulmayGeneratorController(QObject *parent,
                                                         GulmayGeneratorPanelView* view,
                                                         GulmayGeneratorConnector* connector) :
        QObject(parent),
        _view(*view),
        _connector(*connector),
        _timer(new QTimer(this) )
    {
        Wireup();
        InitializeViewBoundries();
        SetIntoNotConnectedState();
    }

    void GulmayGeneratorController::Wireup()
    {
        connect(&_view,&GulmayGeneratorPanelView::NotifyPowerOn,
                this,&GulmayGeneratorController::OnPoweredOn);

        connect(&_connector,SIGNAL(NotifySuccessfullConnectionEstablished()),
                this,SLOT(OnConnect()));

        connect(&_view, &GulmayGeneratorPanelView::NotifyCheckWarmup,
                &_connector,&GulmayGeneratorConnector::QueryWarmupProgram);

        connect(&_connector,SIGNAL(NotifyShortWarmupIsRequired()),
                this,SLOT(OnShortWarmupIsRequired()));

        connect(&_connector,SIGNAL(NotifyLongWarmupIsRequired()),
                this,SLOT(OnLongWarmupIsRequired()));


        connect(&_view, SIGNAL(NotifyStartShortTermWarmup()),
                this,SLOT(OnStartShortTermWarmup()));

        connect(&_view, SIGNAL(NotifyStartLongTermWarmup()),
                this,SLOT(OnStartLongTermWarmup()));

        connect(&_view,SIGNAL(NotifyEmergencyStop()),
                this,SLOT(OnEmergencyStop()));


        connect(&_view, &GulmayGeneratorPanelView::NotifyLargFocalSpotTriggered,
                &_connector,&GulmayGeneratorConnector::SetFocalSpotSizeLarg);

        connect(&_view, &GulmayGeneratorPanelView::NotifySmallFocalSpotTriggered,
                &_connector,&GulmayGeneratorConnector::SetFocalSpotSizeSmall);

        connect(&_view,&GulmayGeneratorPanelView::NotifyUpdateParameters,
                this,&GulmayGeneratorController::OnUpdateExposureParameters);

        connect(&_view,SIGNAL(NotifyStopExposure()),
                this,SLOT(OnStopExposure()));

        connect(&_view,SIGNAL(NotifyStartExposure()),
                this,SLOT(OnStartExposure()));

        connect(&_connector,SIGNAL(NotifyKvpValue(int)),
                &_view,SLOT(UpdateKvpValue(int)));

        connect(&_connector,SIGNAL(NotifyCurrentValue(int)),
                &_view,SLOT(UpdateCurrentValue(int)));

        connect(&_connector,SIGNAL(NotifyShootTime(int)),
                &_view,SLOT(UpdateShootTime(int)));

        connect(&_connector,SIGNAL(NotifyEllapsedExposureTime(int)),
                &_view,SLOT(UpdateEllapsedExposureTime(int)));

        connect(&_connector,SIGNAL(NotifyLargFocalSpotSizeTriggered()),
                &_view,SLOT(SetFocalSpotLarg()));

        connect(&_connector,SIGNAL(NotifySmallFocalSpotSizeTriggered()),
                &_view,SLOT(SetFocalSpotSmall()));

        connect(&_connector,SIGNAL(NotifyWarmupFinished()),
                this,SLOT(OnWarmupFinished()));

        connect(&_connector,SIGNAL(NotifyConnectionClosed()),
                this,SLOT(OnDisconnected()));

        connect(&_connector,SIGNAL(NotifyConnectionError(QString)),
                this,SLOT(OnConnectionError(QString)));

        connect(&_connector,SIGNAL(NotifyGeneratorMessage(QString)),
                this,SLOT(OnGeneratorMessageReceived(QString)));

        connect(&_connector,SIGNAL(NotifyGeneratorError(QString)),
                this,SLOT(OnGeneratorErrorReceived(QString)));

        connect(&_connector,SIGNAL(NotifyGeneratorMode(int)),
                this,SLOT(OnReadingGeneratorMode(int)));

        /*! When _connector says: Exposure started, this means I am running
         * the !X command BUT I have no idea if this is for shooting or warming-up.
         * do not confuse this!
         * By the way, the request may come from a View object or another controller.
         * connector has no idea about the origin!
         * I can use it to turn led on or make a beep alarm.
         */
        connect(&_connector,SIGNAL(NotifyExposureStarted()),
                this,SLOT(OnShootingStart()));

        connect(&_connector,SIGNAL(NotifyExposureStoped()),
                this,SLOT(OnShootingStop()));


        connect(_timer, SIGNAL(timeout()), this, SLOT(OnControlGenerator()));
        connect(this,SIGNAL(NotifyConnectionError(const QString&)),_timer,SLOT(stop()));

    }


    void GulmayGeneratorController::OnPoweredOn()
    {
        // if there is no connection already then start one.
        // Otherwise, do not reconnect.
        // The StartGenerator is safe for multiple start
        // but for further furtification I put this check here.
        if(_state== GENERATOR_STATE::NOT_CONNECTED)
        {
            _connector.StartGenerator();
            _timer->start(TIMER_INTERVAL);
        }

        SetIntoNotConnectedState();
    }

    void GulmayGeneratorController::OnControlGenerator()
    {
        qDebug()<<"Periodic Check is triggered";
        _connector.CheckAnyError();
        _connector.QueryGeneratorMode();

        if(_state==GENERATOR_STATE::SHORT_TERM_WARMING_UP ||
                _state==GENERATOR_STATE::LONG_TERM_WARMING_UP)
            _connector.QueryEllapsedExposureTime();
    }

    void GulmayGeneratorController::OnShortWarmupIsRequired()
    {
        _state = GENERATOR_STATE::READY_FOR_SHORT_WARMUP;
        SetIntoWarmupState();
    }
    void GulmayGeneratorController::OnLongWarmupIsRequired()
    {
        _state = GENERATOR_STATE::READY_FOR_LONG_WARMUP;
        SetIntoWarmupState();
    }


    void GulmayGeneratorController::OnSuccessfullConnectionEstablished()
    {
        _view.EnableCheckWarmup();
    }

    void GulmayGeneratorController::OnStartShortTermWarmup()
    {
        _state=GENERATOR_STATE::SHORT_TERM_WARMING_UP;
        _connector.StartShortTermWarmup();
        SetIntoWarmupState();
    }

    void GulmayGeneratorController::OnStartLongTermWarmup()
    {
        _state=GENERATOR_STATE::LONG_TERM_WARMING_UP;
        _connector.StartLongTermWarmup();
        SetIntoWarmupState();
    }

    void GulmayGeneratorController::OnEmergencyStop()
    {
        if(_state != GENERATOR_STATE::NOT_CONNECTED)
        {
            _connector.EmergencyStop();
            SetIntoConnectedState();
        }

    }


    void GulmayGeneratorController::InitializeViewBoundries()
    {
        _view.SetKvpBoundries(KVP_MIN,KVP_MAX);
        _view.SetCurrentBoundries(AMP_MIN,AMP_MAX);
        _view.SetTimeBoundries(TIME_MIN,TIME_MAX);
    }
    void GulmayGeneratorController::InitializeExposureParameters()
    {
        _connector.UpdateKvp(KVP_INIT);
        _connector.UpdateAmp(AMP_INIT);
        _connector.UpdateShootTime(TIME_INIT);
    }

    /*! If the cable is disconnected then the user has to restart the program.
     * Change it for future versions. */
    void GulmayGeneratorController::OnConnectionError(const QString &message)
    {

        //SetIntoConnectionErrorState();
        SetIntoNotConnectedState();
        emit NotifyConnectionError(message);
    }


    void GulmayGeneratorController::OnDisconnected()
    {
        SetIntoNotConnectedState();
    }

    void GulmayGeneratorController::OnGeneratorMessageReceived(const QString &message)
    {
        emit NotifyGeneratorMessage(message);
    }

    void GulmayGeneratorController::OnGeneratorErrorReceived(const QString &message)
    {
        SetIntoNotConnectedState();
        emit NotifyGeneratorError(message);
    }

    void GulmayGeneratorController::OnStopExposure()
    {
        _connector.StopExposure();
        SetIntoReadyState();
    }

    void GulmayGeneratorController::OnStartExposure()
    {
        _connector.StartExposure();
        SetIntoShootingState();
    }

    void GulmayGeneratorController::SetIntoGeneratorErrorState()
    {
        _state=GENERATOR_STATE::GENERATOR_ERROR;
        _view.DisableFocalSpotChange();
        _view.DisableReadingExposureParameters();
        _view.DisablePowerOnButton();
        _view.DisableShooting();
        _view.DisableWarmup();

    }

    void GulmayGeneratorController::SetIntoConnectionErrorState()
    {
        _state= GENERATOR_STATE::CONNECTION_ERROR;
        _view.DisableControlPanel();

    }

    void GulmayGeneratorController::SetIntoWarmupState()
    {
        switch (_state)
        {
        case GENERATOR_STATE::READY_FOR_SHORT_WARMUP:
            _view.DisableLongWarmup();
            _view.EnableShortWarmup();
            break;
        case GENERATOR_STATE::READY_FOR_LONG_WARMUP:
            _view.DisableShortWarmup();
            _view.EnableLongWarmup();
        case GENERATOR_STATE::SHORT_TERM_WARMING_UP:
            _view.DisableShortWarmup();
            break;
        case GENERATOR_STATE::LONG_TERM_WARMING_UP:
            _view.DisableLongWarmup();
            break;
        }

        _view.DisableCheckWarmup();
        _view.DisableFocalSpotChange();
        _view.DisableReadingExposureParameters();
        _view.DisableShooting();
        _view.DisablePowerOnButton();

    }


    void GulmayGeneratorController::SetIntoNotConnectedState()
    {
        _state=GENERATOR_STATE::NOT_CONNECTED;
        _view.DisableFocalSpotChange();
        _view.DisableReadingExposureParameters();
        _view.DisableShooting();
        _view.DisableCheckWarmup();
        _view.DisableWarmup();
    }

    void GulmayGeneratorController::SetIntoConnectedState()
    {
        _state = GENERATOR_STATE::CONNECTED;
        _view.DisableFocalSpotChange();
        _view.DisableReadingExposureParameters();
        _view.DisableShooting();
        _view.DisableWarmup();
        _view.EnableCheckWarmup();
        _view.DisablePowerOnButton();

    }

    void GulmayGeneratorController::SetIntoReadyState()
    {
        _state=GENERATOR_STATE::READY;
        _view.EnableFocalSpotChange();
        _view.EnableReadingExposureParameters();
        _view.EnableShooting();
        _view.DisableWarmup();
        _view.EnableFocalSpotChange();
        _view.DisableCheckWarmup();

        //InitializeExposureParameters();

    }

    void GulmayGeneratorController::SetIntoShootingState()
    {
        _state=GENERATOR_STATE::SHOOTING;
        _view.EnableFocalSpotChange();// Are you sure?!
        _view.DisableReadingExposureParameters();
        _view.DisableFocalSpotChange();
        _view.DisableWarmup();
    }



    void GulmayGeneratorController::OnWarmupFinished()
    {
        SetIntoReadyState();
    }

    void GulmayGeneratorController::OnShootingStart()
    {
        // turn Alarm LED on here.
    }

    void GulmayGeneratorController::OnShootingStop()
    {
        // turn Alarm LED off here.
    }

    void GulmayGeneratorController::OnConnect()
    {
        SetIntoConnectedState();
    }

    void GulmayGeneratorController::OnReadingGeneratorMode(const int mode)
    {
        // If exposure ended then the gen mode goes from 4 to 3 then to 0
        // We can detect this to go to ready state
        if((currectMode==4 && mode==0) || (currectMode==3 && mode==0))
        {
            qDebug()<<"current mod:"<<currectMode;
            qDebug()<<"mod:"<<mode;
            _state=GENERATOR_STATE::READY;
            SetIntoReadyState();
        }

        if(mode != currectMode)
        {
            currectMode=mode;
            emit NotifyGeneratorMessage("Generator mode: " + QString::number(mode));
        }


    }

    /*! Validate the parameters came from View and pass them into Model Object.
     * Classic MVC*/
    void GulmayGeneratorController::OnUpdateExposureParameters(const int &kvp,
                                                               const int &amper,
                                                               const int &duration)
    {
        if(KVP_MIN <= kvp && kvp <= KVP_MAX)
            _connector.UpdateKvp(kvp);

        if(AMP_MIN <= amper && amper <= AMP_MAX)
            _connector.UpdateAmp(amper);

        if(TIME_MIN <= duration && duration <= TIME_MAX)
            _connector.UpdateShootTime(duration);
    }
}

