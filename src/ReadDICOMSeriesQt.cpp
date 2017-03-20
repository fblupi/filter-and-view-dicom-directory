#include "ReadDICOMSeriesQt.h"
#include "ui_ReadDICOMSeriesQt.h"

#include <QFileDialog>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <itkVTKImageToImageFilter.h>
#include <itkCastImageFilter.h>
#include <itkCannyEdgeDetectionImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkExtractImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkHoughTransform2DLinesImageFilter.h>

ReadDICOMSeriesQt::ReadDICOMSeriesQt(QWidget *parent) : QMainWindow(parent), ui(new Ui::ReadDICOMSeriesQt) {
    ui->setupUi(this);
    reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    viewer = vtkSmartPointer<vtkImageViewer2>::New();
	viewerFilter = vtkSmartPointer<vtkImageViewer2>::New();
}

ReadDICOMSeriesQt::~ReadDICOMSeriesQt() {
    delete ui;
}

void ReadDICOMSeriesQt::openDICOMFolder() {
    QString folderNameDICOM = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), QDir::currentPath(), QFileDialog::ShowDirsOnly);
    drawDICOMSeries(folderNameDICOM.toUtf8().constData());
}

void ReadDICOMSeriesQt::drawDICOMSeries(std::string folderDICOM) {
    reader->SetDirectoryName(folderDICOM.c_str());
    reader->Update();
	currentPosition = 0;

    viewer->SetInputConnection(reader->GetOutputPort());

    ui->qvtkWidget->SetRenderWindow(viewer->GetRenderWindow());
	ui->qvtkWidgetFilter->SetRenderWindow(viewerFilter->GetRenderWindow());

    viewer->SetupInteractor(ui->qvtkWidget->GetInteractor());
	viewerFilter->SetupInteractor(ui->qvtkWidgetFilter->GetInteractor());

    viewer->Render();

	minSlice = viewer->GetSliceMin();
	maxSlice = viewer->GetSliceMax();

	filter();

    ui->sliderSlices->setMinimum(minSlice);
    ui->sliderSlices->setMaximum(maxSlice);
    ui->labelFolderName->setText(QString::fromStdString(folderDICOM));
}

void ReadDICOMSeriesQt::on_buttonOpenFolder_clicked() {
    openDICOMFolder();
}

void ReadDICOMSeriesQt::on_sliderSlices_sliderMoved(int position) {
	currentPosition = position;
    viewer->SetSlice(position);
    viewer->Render();
	filter();
}

void ReadDICOMSeriesQt::on_buttonNextSlice_clicked() {
	currentPosition++;
	ui->sliderSlices->setSliderPosition(currentPosition);
	viewer->SetSlice(currentPosition);
	viewer->Render();
	filter();
}

void ReadDICOMSeriesQt::on_buttonPreviousSlice_clicked() {
	currentPosition++;
	ui->sliderSlices->setSliderPosition(currentPosition);
	viewer->SetSlice(currentPosition);
	viewer->Render();
	filter();
}

void ReadDICOMSeriesQt::on_doubleSpinBoxVariance_valueChanged(double value) {
	filter();
}

void ReadDICOMSeriesQt::on_doubleSpinBoxHigherThreshold_valueChanged(double value) {
	filter();
}

void ReadDICOMSeriesQt::on_doubleSpinBoxLowerThreshold_valueChanged(double value) {
	filter();
}

void ReadDICOMSeriesQt::filter() {
	const unsigned int nLines = 1;

	const unsigned int InputDimension = 3;
	const unsigned int Dimension = 2;
	typedef signed short InputPixelType;
	typedef float PixelType;

	typedef itk::Image<InputPixelType, InputDimension> InputImageType3D;
	typedef itk::Image<PixelType, InputDimension> ImageType3D;
	typedef itk::Image<InputPixelType, Dimension> InputImageType;
	typedef itk::Image<PixelType, Dimension> ImageType;

	typedef itk::VTKImageToImageFilter<InputImageType3D> VTKImageToImageType;
	typedef itk::ExtractImageFilter<InputImageType3D, InputImageType> Extract2DImageFilter;
	typedef itk::CastImageFilter<InputImageType, ImageType> CastToRealFilterType;
	typedef itk::ThresholdImageFilter<ImageType> ThresholdFilterType;
	typedef itk::CannyEdgeDetectionImageFilter<ImageType, ImageType> CannyFilterType;
	typedef itk::HoughTransform2DLinesImageFilter<PixelType, PixelType> HoughTransformFilterType;
	typedef itk::RescaleIntensityImageFilter<ImageType, InputImageType> RescaleFilterType;
	typedef itk::ImageToVTKImageFilter<InputImageType> ImageToVTKImageType;

	VTKImageToImageType::Pointer connectorInput = VTKImageToImageType::New();
	Extract2DImageFilter::Pointer to2D = Extract2DImageFilter::New();
	CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
	ThresholdFilterType::Pointer threshold = ThresholdFilterType::New();
	CannyFilterType::Pointer canny = CannyFilterType::New();
	HoughTransformFilterType::Pointer hough = HoughTransformFilterType::New();
	RescaleFilterType::Pointer rescale = RescaleFilterType::New();
	ImageToVTKImageType::Pointer connectorOutput = ImageToVTKImageType::New();
	ImageToVTKImageType::Pointer connectorOutputLines = ImageToVTKImageType::New();

	// VTK to ITK
	connectorInput->SetInput(reader->GetOutput());
	connectorInput->Update();

	// 3D to 2D
	to2D->SetInput(connectorInput->GetOutput());
	to2D->InPlaceOn();
	to2D->SetDirectionCollapseToSubmatrix();
	
	InputImageType3D::RegionType inputRegion = connectorInput->GetOutput()->GetLargestPossibleRegion();
	InputImageType3D::SizeType size = inputRegion.GetSize();
	size[2] = 0;

	InputImageType3D::IndexType start = inputRegion.GetIndex();
	start[2] = currentPosition;

	InputImageType3D::RegionType desiredRegion;
	desiredRegion.SetSize(size);
	desiredRegion.SetIndex(start);

	to2D->SetExtractionRegion(desiredRegion);
	to2D->Update();

	// Cast to float
	toReal->SetInput(to2D->GetOutput());

	// Filtering only wood density values
	threshold->SetInput(toReal->GetOutput());
	threshold->SetOutsideValue(-1000);
	threshold->ThresholdOutside(-800, -300);
	threshold->Update();

	// Edge detection
	canny->SetInput(threshold->GetOutput());
	canny->SetVariance(ui->doubleSpinBoxVariance->value());
	canny->SetUpperThreshold(ui->doubleSpinBoxHigherThreshold->value());
	canny->SetLowerThreshold(ui->doubleSpinBoxLowerThreshold->value());
	canny->Update();

	// Lines detection
	hough->SetInput(canny->GetOutput());
	hough->SetNumberOfLines(nLines);
	hough->Update();

	ImageType::Pointer localAccumulator = hough->GetOutput();
	HoughTransformFilterType::LinesListType lines;
	lines = hough->GetLines(nLines);

	InputImageType::Pointer localOutputImage = InputImageType::New();
	InputImageType::RegionType region(to2D->GetOutput()->GetLargestPossibleRegion());
	localOutputImage->SetRegions(region);
	localOutputImage->CopyInformation(to2D->GetOutput());
	localOutputImage->Allocate(true);

	typedef HoughTransformFilterType::LinesListType::const_iterator LineIterator;
	LineIterator itLines = lines.begin();
	while (itLines != lines.end()) {
		typedef HoughTransformFilterType::LineType::PointListType  PointListType;
		PointListType pointsList = (*itLines)->GetPoints();
		PointListType::const_iterator itPoints = pointsList.begin();
		double u[2];
		u[0] = (*itPoints).GetPosition()[0];
		u[1] = (*itPoints).GetPosition()[1];
		itPoints++;
		double v[2];
		v[0] = u[0] - (*itPoints).GetPosition()[0];
		v[1] = u[1] - (*itPoints).GetPosition()[1];
		double norm = std::sqrt(v[0] * v[0] + v[1] * v[1]);
		v[0] /= norm;
		v[1] /= norm;

		ImageType::IndexType localIndex;
		itk::Size<2> size = localOutputImage->GetLargestPossibleRegion().GetSize();
		float diag = std::sqrt((float)(size[0] * size[0] + size[1] * size[1]));
		for (int i = static_cast<int>(-diag); i < static_cast<int>(diag); i++) {
			localIndex[0] = (long int)(u[0] + i*v[0]);
			localIndex[1] = (long int)(u[1] + i*v[1]);
			InputImageType::RegionType outputRegion = localOutputImage->GetLargestPossibleRegion();
			if (outputRegion.IsInside(localIndex)) {
				localOutputImage->SetPixel(localIndex, 255);
			}
		}
		itLines++;
	}

	// Cast to short
	rescale->SetInput(canny->GetOutput());

	// ITK to VTK
	connectorOutput->SetInput(rescale->GetOutput());
	connectorOutput->Update();

	connectorOutputLines->SetInput(localOutputImage);
	connectorOutputLines->Update();

	// View
	viewerFilter->SetInputData(connectorOutput->GetOutput());
	viewerFilter->Render();

	viewer->SetInputData(connectorOutputLines->GetOutput());
	viewer->Render();
}
