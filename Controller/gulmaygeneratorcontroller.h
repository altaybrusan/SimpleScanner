#ifndef GULMAYGENERATORCONTROLLER_H
#define GULMAYGENERATORCONTROLLER_H

#include <QObject>
class QTimer;
namespace SimpleScanner
{

    class GulmayGeneratorPanelView;
    class GulmayGeneratorConnector;
    class GulmayGeneratorController : public QObject
    {
        Q_OBJECT
    public:
        explicit GulmayGeneratorController(QObject *parent,
                                           GulmayGeneratorPanelView *view ,
                                           GulmayGeneratorConnector* connector);

    signals:
        void NotifyConnectionError(const QString& message)const;
        void NotifyGeneratorMessage(const QString& message)const;
        void NotifyGeneratorError(const QString& error)const;




    public slots:

    private slots:
        void OnDisconnected();
        void OnPoweredOn();

        void OnConnectionError(const QString& message);
        void OnGeneratorMessageReceived(const QString& message);
        void OnGeneratorErrorReceived(const QString& message);

        //void OnStartShortTermWarmup();
        void OnStopExposure();
        //void OnEmergencyStop();
        void OnStartExposure();
        void OnWarmupFinished();
        void OnShootingStart();
        void OnShootingStop();
        void OnConnect();


        void OnReadingGeneratorMode(const int mode);

        void OnUpdateExposureParameters(const int& kvp, const int& amper, const int& duration);

        void OnControlGenerator();

        void OnShortWarmupIsRequired();
        void OnLongWarmupIsRequired();

        void OnSuccessfullConnectionEstablished();

        void OnStartShortTermWarmup();
        void OnStartLongTermWarmup();

        void OnEmergencyStop();



    private:
        GulmayGeneratorPanelView& _view;
        GulmayGeneratorConnector& _connector;
        QTimer* _timer;
        int currectMode=0;

        void Wireup();
        void InitializeViewBoundries();
        void InitializeExposureParameters();
        void SetIntoNotConnectedState();
        void SetIntoConnectedState();

        void SetIntoReadyState();
        void SetIntoShootingState();
        void SetIntoGeneratorErrorState();
        void SetIntoConnectionErrorState();
        void SetIntoWarmupState();
        void SetIntoCheckingWarmupState();


    };
}


#endif // GULMAYGENERATORCONTROLLER_H
