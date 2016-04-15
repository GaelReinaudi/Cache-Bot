#ifndef SPAMMERWINDOW_H
#define SPAMMERWINDOW_H
#include "cacherest.h"

#include <QMainWindow>
class CacheAccountConnector;

namespace Ui {
class SpammerWindow;
}

class SpammerWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit SpammerWindow(QWidget *parent = 0);
	~SpammerWindow();


public slots:
	void onStartStop(bool doStart);
	void sendSignup();
	void onRepliedFakeSignup(QString response);

private:
	Ui::SpammerWindow *ui;
	CacheAccountConnector* m_pConnector = 0;
};

#endif // SPAMMERWINDOW_H
