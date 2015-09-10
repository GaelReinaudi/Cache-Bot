#include "userViewer.h"
#include "ui_userViewer.h"
#include "user.h"
#include "botContext.h"
#include "bot.h"
#include "cacherest.h"
#include "cacheAccountConnector.h"

UserViewer::UserViewer(QString userID, QVector<int> onlyLoadHashes)
	: QMainWindow()
	, ui(new Ui::UserViewer)
{
	ui->setupUi(this);
	Transaction::onlyLoadHashes = onlyLoadHashes;

	m_pConnector = new CacheAccountConnector(userID);
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
	Transaction::setCurrentDay(QDate::currentDate().addDays(-ui->spinAgo->value()));
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
	itRect->topLeft->setCoords(QPointF(x - 4*3600*24, y + (10+6*isTarget)*0.01));
	itRect->bottomRight->setCoords(QPointF(x + 4*3600*24, y - (10+6*isTarget)*0.01));
	QColor colZone = isTarget ? QColor(239, 64, 53, 32) : QColor(0, 64, 253, 32);
	if (flag & 2)
		colZone = QColor(0, 253, 64, 32);
	if (flag & 4)
		colZone = QColor(255, 165, 0, 32);
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
	for (const auto f : jsonObj["features"].toArray()) {
		ui->listBills->addItem(QString(QJsonDocument(f.toObject()).toJson()));
	}
}

