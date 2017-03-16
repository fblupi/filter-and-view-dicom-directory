#include "ReadDICOMSeriesQt.h"
#include "ui_ReadDICOMSeriesQt.h"

#include <QFileDialog>

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

	filter();

    minSlice = viewer->GetSliceMin();
    maxSlice = viewer->GetSliceMax();

    ui->sliderSlices->setMinimum(minSlice);
    ui->sliderSlices->setMaximum(maxSlice);
    ui->labelSlicesNumber->setText(QString::number(maxSlice - minSlice));
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
	typedef itk::ThresholdImageFilter<Real2ImageType> ThresholdFilterType;

	typedef itk::ImageToVTKImageFilter<Char2ImageType> ImageToVTKImageType;
	typedef itk::VTKImageToImageFilter<CharImageType> VTKImageToImageType;

	VTKImageToImageType::Pointer connectorInput = VTKImageToImageType::New();

	connectorInput->SetInput(reader->GetOutput());
	connectorInput->Update();

	Extract2DImageFilter::Pointer to2D = Extract2DImageFilter::New();
	CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
	CannyFilterType::Pointer filter = CannyFilterType::New();
	RescaleFilterType::Pointer rescale = RescaleFilterType::New();
	ThresholdFilterType::Pointer threshold = ThresholdFilterType::New();

	to2D->SetInput(connectorInput->GetOutput());
	to2D->InPlaceOn();
	to2D->SetDirectionCollapseToSubmatrix();
	
	CharImageType::RegionType inputRegion = connectorInput->GetOutput()->GetLargestPossibleRegion();
	CharImageType::SizeType size = inputRegion.GetSize();
	size[2] = 0;

	CharImageType::IndexType start = inputRegion.GetIndex();
	start[2] = currentPosition;

	CharImageType::RegionType desiredRegion;
	desiredRegion.SetSize(size);
	desiredRegion.SetIndex(start);

	to2D->SetExtractionRegion(desiredRegion);
	to2D->Update();

	toReal->SetInput(to2D->GetOutput());

	threshold->SetInput(toReal->GetOutput());
	threshold->SetOutsideValue(-1000);
	threshold->ThresholdOutside(-800, -300);
	threshold->Update();

	filter->SetInput(threshold->GetOutput());

	filter->SetVariance(ui->doubleSpinBoxVariance->value());
	filter->SetUpperThreshold(ui->doubleSpinBoxHigherThreshold->value());
	filter->SetLowerThreshold(ui->doubleSpinBoxLowerThreshold->value());
	filter->Update();
	rescale->SetInput(filter->GetOutput());

	ImageToVTKImageType::Pointer connectorOutput = ImageToVTKImageType::New();
	connectorOutput->SetInput(rescale->GetOutput());
	connectorOutput->Update();

	viewerFilter->SetInputData(connectorOutput->GetOutput());

	viewerFilter->Render();
}
