#include "displaycontroller.h"
#include <QDebug>

#include <vtkTIFFReader.h>
#include <vtkImageViewer2.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleImage.h>
#include <vtkDistanceWidget.h>
#include <vtkDistanceRepresentation.h>
#include <vtkLegendScaleActor.h>
#include <vtkAxisActor2D.h>
#include <vtkCommand.h>

//#include <vtkTextRendererStringToImage.h>
//#include <vtkRendererCollection.h>



namespace SimpleScanner
{
    /*!
     * Collection event handlers. For the next iteration this
     * should completely revisited. It should implement ? class
     * to disable not necessary shortcut keys.
     **/
    class InteractorStyleCallbackHandler
    {
    public:
        void KeypressCallbackFunction(vtkObject*,
                                      long unsigned int vtkNotUsed(eventId),
                                      void* vtkNotUsed(callData))
        {
            qDebug() << "Caught event in MyClass";
        }

    };

    const int LEVEL = 28816;
    const int WINDOW = 66438;
    /*!
     * This is going to be a very simple Display controller.
     * For the next iterations the should be more sufffisticated
     * more functional class.*/
    DisplayController::DisplayController(vtkSmartPointer<vtkRenderWindow> canvas,
                                         QObject *parent) :
        QObject(parent),
        reader(vtkSmartPointer<vtkTIFFReader>::New()),
        renderWindow(canvas),
        renderWindowInteractor(vtkSmartPointer<vtkRenderWindowInteractor>::New()),
        imageViewer(vtkSmartPointer<vtkImageViewer2>::New()),
        legendScaleActor(vtkSmartPointer<vtkLegendScaleActor>::New()),
        style(vtkSmartPointer<vtkInteractorStyleImage>::New()),
        distanceWidget(vtkSmartPointer<vtkDistanceWidget>::New()),
        renderer(vtkSmartPointer<vtkRenderer>::New()),
        showDistanceWidget(true),
        isFirstCall(false)
    {
        InitializeImageViewer();
        InitializeLegendScale();
        InitializeDistanceTool();


        renderWindowInteractor->SetInteractorStyle(style);
        imageViewer->SetupInteractor(renderWindowInteractor);

//        SimpleScanner::MyClass myClass;
//        renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, &myClass, &MyClass::KeypressCallbackFunction);
    }

    bool DisplayController::IsValidTiffFile(QString path)
    {

        if(!reader)
            return false;
        return reader->CanReadFile(path.toLocal8Bit());
    }

    void DisplayController::DisplayImage(QString path)
    {        
        SetRenderWindow(); // Maybe memory leak happens
        reader->SetFileName(path.toLocal8Bit());
        reader->Update();

        imageViewer->SetInputConnection(reader->GetOutputPort());
        imageViewer->GetRenderer()->ResetCamera();


        renderer->SetBackground(0,0,0);
        renderer->ResetCameraClippingRange();
        renderer->AddActor(legendScaleActor);

        imageViewer->SetRenderer(renderer);
        imageViewer->Render();

        renderWindowInteractor->Initialize();
        if(showDistanceWidget)
            distanceWidget->On();
        else
            distanceWidget->Off();

        if(!isFirstCall){

            isFirstCall=true;
            renderWindowInteractor->Start();
        }

    }

    void DisplayController::Terminate()
    {
        renderWindowInteractor->TerminateApp();
    }

    void DisplayController::SetRenderWindow()
    {
        if(!isFirstCall)
        {
            imageViewer->SetRenderWindow(renderWindow);
            renderWindowInteractor->SetRenderWindow(renderWindow);
        }
       }

    void DisplayController::InitializeImageViewer()
    {
        imageViewer->SetColorLevel(LEVEL);
        imageViewer->SetColorWindow(WINDOW);
    }

    void DisplayController::InitializeLegendScale()
    {
        legendScaleActor->TopAxisVisibilityOff();
        legendScaleActor->RightAxisVisibilityOff();
        legendScaleActor->SetLegendVisibility(0);

    }

    void DisplayController::InitializeDistanceTool()
    {
        distanceWidget->SetInteractor(renderWindowInteractor);
        distanceWidget->CreateDefaultRepresentation();
        static_cast<vtkDistanceRepresentation *>(distanceWidget->GetRepresentation())
                ->SetLabelFormat("%-#6.3g pix");
    }

}

