#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "ACChart/acustomplot.h"

class User;
class CacheAccountConnector;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QString userID);
	~MainWindow();

	User* user() const;

public slots:
	void onUserInjected(User* pUser);
	void onBotInjected(Bot *bestBot);

private:
	Ui::MainWindow *ui;
	CacheAccountConnector* m_pConnector = 0;
};

#endif // MAINWINDOW_H
