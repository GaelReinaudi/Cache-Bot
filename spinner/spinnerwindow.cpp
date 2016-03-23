#include "spinnerwindow.h"
#include "ui_spinnerwindow.h"

SpinnerWindow::SpinnerWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::SpinnerWindow)
{
	ui->setupUi(this);
}

SpinnerWindow::~SpinnerWindow()
{
	delete ui;
}
