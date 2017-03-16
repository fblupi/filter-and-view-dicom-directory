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

    viewer->SetInputConnection(reader->GetOutputPort());

    ui->qvtkWidget->SetRenderWindow(viewer->GetRenderWindow());
	ui->qvtkWidgetFilter->SetRenderWindow(viewerFilter->GetRenderWindow());

    viewer->SetupInteractor(ui->qvtkWidget->GetInteractor());
	viewerFilter->SetupInteractor(ui->qvtkWidgetFilter->GetInteractor());

    viewer->Render();

	filter(0);

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
    viewer->SetSlice(position);
    viewer->Render();
	filter(position);
}

void ReadDICOMSeriesQt::filter(int pos) {
	VTKImageToImageType::Pointer connectorInput = VTKImageToImageType::New();

	connectorInput->SetInput(reader->GetOutput());
	connectorInput->Update();

	Extract2DImageFilter::Pointer to2D = Extract2DImageFilter::New();
	CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
	CannyFilterType::Pointer filter = CannyFilterType::New();
	RescaleFilterType::Pointer rescale = RescaleFilterType::New();

	to2D->SetInput(connectorInput->GetOutput());
	to2D->InPlaceOn();
	to2D->SetDirectionCollapseToSubmatrix();
	
	CharImageType::RegionType inputRegion = connectorInput->GetOutput()->GetLargestPossibleRegion();
	CharImageType::SizeType size = inputRegion.GetSize();
	size[2] = 0;

	CharImageType::IndexType start = inputRegion.GetIndex();
	start[2] = pos;

	CharImageType::RegionType desiredRegion;
	desiredRegion.SetSize(size);
	desiredRegion.SetIndex(start);

	to2D->SetExtractionRegion(desiredRegion);
	to2D->Update();

	toReal->SetInput(to2D->GetOutput());
	filter->SetInput(toReal->GetOutput());

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
	viewerFilter->GetRenderer()->ResetCamera();
	viewerFilter->Render();
}
