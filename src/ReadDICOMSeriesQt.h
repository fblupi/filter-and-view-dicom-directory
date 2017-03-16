#ifndef ReadDICOMSeriesQt_H
#define ReadDICOMSeriesQt_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkVTKImageToImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkExtractImageFilter.h>

namespace Ui {
    class ReadDICOMSeriesQt;
}

class ReadDICOMSeriesQt : public QMainWindow {
    Q_OBJECT

public:
    explicit ReadDICOMSeriesQt(QWidget *parent = 0);
    ~ReadDICOMSeriesQt();

private slots:
    void openDICOMFolder();
    void drawDICOMSeries(std::string folderDICOM);
    void on_buttonOpenFolder_clicked();
    void on_sliderSlices_sliderMoved(int posicion);

private:
    Ui::ReadDICOMSeriesQt *ui;
    vtkSmartPointer<vtkDICOMImageReader> reader;
    vtkSmartPointer<vtkImageViewer2> viewer;
	vtkSmartPointer<vtkImageViewer2> viewerFilter;
	vtkSmartPointer<vtkImageData> data = NULL;
    int minSlice;
    int maxSlice;

	typedef signed short CharPixelType;
	typedef float RealPixelType;

	typedef itk::Image<CharPixelType, 3> CharImageType;
	typedef itk::Image<RealPixelType, 3> RealImageType;
	typedef itk::Image<CharPixelType, 2> Char2ImageType;
	typedef itk::Image<RealPixelType, 2> Real2ImageType;

	typedef itk::ExtractImageFilter<CharImageType, Char2ImageType> Extract2DImageFilter;
	typedef itk::CastImageFilter<Char2ImageType, Real2ImageType> CastToRealFilterType;
	typedef itk::CannyEdgeDetectionImageFilter<Real2ImageType, Real2ImageType> CannyFilterType;
	typedef itk::RescaleIntensityImageFilter<Real2ImageType, Char2ImageType> RescaleFilterType;

	typedef itk::ImageToVTKImageFilter<Char2ImageType> ImageToVTKImageType;
	typedef itk::VTKImageToImageFilter<CharImageType> VTKImageToImageType;

	void filter(int pos);
};

#endif // ReadDICOMSeriesQt_H
