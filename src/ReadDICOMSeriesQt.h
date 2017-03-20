#ifndef ReadDICOMSeriesQt_H
#define ReadDICOMSeriesQt_H

#include <QMainWindow>
#include <vtkSmartPointer.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

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
	void on_doubleSpinBoxVariance_valueChanged(double value);
	void on_doubleSpinBoxHigherThreshold_valueChanged(double value);
	void on_doubleSpinBoxLowerThreshold_valueChanged(double value);

private:
    Ui::ReadDICOMSeriesQt *ui;
    vtkSmartPointer<vtkDICOMImageReader> reader;
    vtkSmartPointer<vtkImageViewer2> viewer;
	vtkSmartPointer<vtkImageViewer2> viewerFilter;
	vtkSmartPointer<vtkImageData> data = NULL;
    int minSlice;
    int maxSlice;
	int currentPosition;

	void filter();
};

#endif // ReadDICOMSeriesQt_H
