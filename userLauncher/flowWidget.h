#ifndef FLOWWIDGET_H
#define FLOWWIDGET_H

#include <QWidget>
#include <QJsonObject>

namespace Ui {
class FlowWidget;
}

class FlowWidget : public QWidget
{
	Q_OBJECT

public:
	explicit FlowWidget(QString id, QJsonObject userObj, QWidget *parent = 0);
	~FlowWidget();

private:
	Ui::FlowWidget *ui;
	QJsonObject m_obj;
};

#endif // FLOWWIDGET_H
