#ifndef SPINNERWINDOW_H
#define SPINNERWINDOW_H

#include <QMainWindow>

namespace Ui {
class SpinnerWindow;
}

class SpinnerWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SpinnerWindow(QWidget *parent = 0);
	~SpinnerWindow();

private:
	Ui::SpinnerWindow *ui;
};

#endif // SPINNERWINDOW_H
