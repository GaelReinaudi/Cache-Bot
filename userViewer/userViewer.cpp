#include "userViewer.h"
#include "ui_userViewer.h"
#include "user.h"
#include "botContext.h"
#include "bot.h"
#include "cacherest.h"
#include "cacheAccountConnector.h"

UserViewer::UserViewer(QString userID, QJsonObject jsonArgs)
	: QMainWindow()
	, ui(new Ui::UserViewer)
{
	ui->setupUi(this);
	setWindowTitle(QString("..")+userID.right(5));

	m_pConnector = new CacheAccountConnector(userID, jsonArgs);

	connect(m_pConnector, SIGNAL(injected(User*)), this, SLOT(onUserInjected(User*)));
	connect(m_pConnector, SIGNAL(botInjected(Bot*)), this, SLOT(onBotInjected(Bot*)));
	connect(ui->spinAgo, SIGNAL(valueChanged(int)), this, SLOT(onAgo()));
}

UserViewer::~UserViewer()
{
	if (m_pConnector) {
		delete m_pConnector;
	}
	delete ui;
}

User *UserViewer::user() const
{
	return m_pConnector->user();
}

void UserViewer::onAgo()
{
	Transaction::setCurrentDay(Transaction::actualCurrentDay().addDays(-ui->spinAgo->value()));
	ui->dateEdit->setDate(Transaction::currentDay());
	user()->reInjectBot();
}

void UserViewer::onUserInjected(User* pUser)
{
	connect(user()->botContext(), &BotContext::matchedTransaction, this, &UserViewer::plotMask, Qt::UniqueConnection);
	connect(user()->botContext(), &BotContext::summarizingTree, this, &UserViewer::clearMasks, Qt::UniqueConnection);
	connect(user()->botContext(), &BotContext::needsReplot, this, &UserViewer::replotCharts, Qt::UniqueConnection);
	connect(user()->botContext(), &BotContext::newSummarizedTree, this, &UserViewer::onNewSummarizedTree, Qt::UniqueConnection);

	CacheRest::Instance()->getBestBot(pUser->id(), pUser);

	ui->acPlot->loadCompressedAmount(user());
	ui->sliderHash->setRange(-1, ui->acPlot->hashKeys().count() - 1);
	ui->sliderHash->setValue(-1);

	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->acPlot, SLOT(showHash(int)), Qt::UniqueConnection);
	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->ahPlot, SLOT(showHash(int)), Qt::UniqueConnection);
	connect(ui->sliderHash, SIGNAL(valueChanged(int)), ui->spinHash, SLOT(setValue(int)), Qt::UniqueConnection);
	connect(ui->spinHash, SIGNAL(valueChanged(int)), ui->sliderHash, SLOT(setValue(int)), Qt::UniqueConnection);
	connect(ui->acPlot, SIGNAL(newLabel(QString)), ui->labelBundle, SLOT(setText(QString)), Qt::UniqueConnection);
	connect(ui->acPlot, SIGNAL(newSum(double)), ui->spinSum, SLOT(setValue(double)), Qt::UniqueConnection);
	connect(ui->acPlot, SIGNAL(newHashValue(int)), ui->spinHashVal, SLOT(setValue(int)), Qt::UniqueConnection);
}

void UserViewer::onBotInjected(Bot* bestBot)
{
	NOTICE() << "UserViewer::onBotInjected";
	bestBot->summarize();

	ui->ahPlot->loadCompressedAmount(user());


	replotCharts();
}

void UserViewer::clearMasks()
{
	ui->acPlot->clearItems();
}

void UserViewer::plotMask(double x, double y, int flag)
{
	QCPItemRect* itRect = new QCPItemRect(ui->acPlot);
	y = kindaLog(y);
	bool isTarget = flag == 0;
	itRect->topLeft->setCoords(QPointF(x - 3600*24, y + (10+6*isTarget)*0.01));
	itRect->bottomRight->setCoords(QPointF(x + 3600*24, y - (10+6*isTarget)*0.01));
	QColor colZone = flag == 0 ? QColor(239, 64, 53, 32) : QColor(0, 64, 253, 64);
	if (flag & 2)
		colZone = QColor(0, 253, 64, 32);
	if (flag & 4)
		colZone = QColor(255, 165, 0, 32);
	if (flag & 8)
		colZone = QColor(255, 69, 0, 64);
	if (flag & 16)
		colZone = QColor(0, 165, 0, 64);
	itRect->setPen(QPen(QBrush(colZone), 3.0));
	itRect->setBrush(QBrush(colZone));
	itRect->setClipToAxisRect(false);
	ui->acPlot->addItem(itRect);
}

void UserViewer::replotCharts()
{
	ui->acPlot->replot(QCustomPlot::rpQueued);
	ui->ahPlot->replot(QCustomPlot::rpQueued);
}

void UserViewer::onNewSummarizedTree(QJsonObject jsonObj)
{
	ui->listBills->clear();
	// sort per tot amount
	QList<QJsonObject> listObj;
	for (const auto f : jsonObj["features"].toArray()) {
		listObj.append(f.toObject());
	}
	std::sort(listObj.begin(), listObj.end(), [](const QJsonObject& a, const QJsonObject& b){
		return qAbs(a["_daily"].toDouble()) > qAbs(b["_daily"].toDouble());
	});
	for (const auto f : listObj) {
		ui->listBills->addItem(QString(QJsonDocument(f).toJson()));
	}
}

