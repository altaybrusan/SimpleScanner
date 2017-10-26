#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#include <QObject>
#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>


class vtkTIFFReader;
class vtkRenderWindow;
class vtkImageViewer2;
class vtkLegendScaleActor;
class vtkInteractorStyleImage;
class vtkDistanceWidget;

namespace SimpleScanner
{

    class DisplayController : public QObject
    {
        Q_OBJECT
    public:
        explicit DisplayController(vtkSmartPointer<vtkRenderWindow> canvas ,
                                   QObject *parent = nullptr);
        bool IsValidTiffFile(QString path);
        void DisplayImage(QString path);
        void Terminate();

    signals:

    public slots:
    private:
        vtkSmartPointer<vtkTIFFReader> reader;
        vtkSmartPointer<vtkRenderWindow> renderWindow;
        vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
        vtkSmartPointer<vtkImageViewer2> imageViewer;
        vtkSmartPointer<vtkLegendScaleActor> legendScaleActor;
        vtkSmartPointer<vtkInteractorStyleImage> style;
        vtkSmartPointer<vtkDistanceWidget> distanceWidget;
        vtkSmartPointer<vtkRenderer> renderer;
        bool showDistanceWidget;
        bool isFirstCall;

        void InitializeImageViewer();
        void InitializeLegendScale();
        void InitializeDistanceTool();
        void SetRenderWindow();

    };
}


#endif // DISPLAYCONTROLLER_H
