#ifndef SPINNERWINDOW_H
#define SPINNERWINDOW_H
#include "cacherest.h"

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

protected slots:
	void onLoggedIn(bool didLogin);
	void onRepliedIds(QString strData);

private:
	Ui::SpinnerWindow *ui;
};

#endif // SPINNERWINDOW_H
