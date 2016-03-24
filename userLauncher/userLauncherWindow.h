#ifndef SPINNERWINDOW_H
#define SPINNERWINDOW_H

#include <QMainWindow>
#include <QJsonObject>

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
	QJsonObject m_UserIds;
};

#endif // SPINNERWINDOW_H
