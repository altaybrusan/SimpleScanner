#ifndef TELEDYNEDETECTORCONTROLLER_H
#define TELEDYNEDETECTORCONTROLLER_H

#include <QObject>

class QTimer;

namespace SimpleScanner
{
    class TeledyneDetectorPanelView;
    class TeledyneDetectorConnector;

    class TeledyneDetectorController : public QObject
    {
        Q_OBJECT
    public:
        explicit TeledyneDetectorController(QObject* parent,
                                            TeledyneDetectorPanelView* view,
                                            TeledyneDetectorConnector* connector);
        ~TeledyneDetectorController();

    signals:
        void NotifyDetectorMessage(const QString& message)const;
        void NotifyDetectorError(const QString& error)const;
        void NotifyProgress(float percent);

    public slots:

    private slots:
        void OnConnectorMessageReceived(const QString& message);
        void OnConnectorError(const QString& error) const;
        void OnGrabImage(int& binningIndex,
                         int& speed,
                         int& height,
                         QString& mode);
        void OnCommandFinished();
        void OnDetectorConnectionError();

    private:

        TeledyneDetectorPanelView& _view;
        TeledyneDetectorConnector& _connector;
        QTimer* _timer;
        float _totalAcqTimeEstimate;
//        bool _isConnected;

        void Wireup();
        int ConvertBinningIndexToBinningMode(int& index);
    };
}
#endif // TELEDYNEDETECTORCONTROLLER_H
