#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings(QString("%1.ini")
                       .arg(QCoreApplication::instance()->applicationName())
                       , QSettings::IniFormat);

    settings.beginGroup("RequesterCredentials");
    ui->usernameLineEdit->setText(settings.value("Username").toString());
    ui->passwordLineEdit->setText(settings.value("Password").toString());
    ui->signatureLineEdit->setText(settings.value("Signature").toString());
    ui->appIdLineEdit->setText(settings.value("AppId").toString());
    ui->subjectLineEdit->setText(settings.value("Subject").toString());
    settings.endGroup();

    service.setEndPoint(QLatin1String("http://localhost:8081"));
    service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
    service.clientInterface()->setStyle(KDSoapClientInterface::RPCStyle);

    typedef PayPalAPIInterfaceService::PayPalAPISoapBinding PPAPI;

    connect(&service, &PPAPI::getBalanceDone, this, &MainWindow::getBalanceDone);
    connect(&service, &PPAPI::getBalanceError, this, &MainWindow::getBalanceError);
}

MainWindow::~MainWindow()
{
    QSettings settings(QString("%1.ini")
                       .arg(QCoreApplication::instance()->applicationName())
                       , QSettings::IniFormat);

    settings.beginGroup("RequesterCredentials");
    settings.setValue("Username", ui->usernameLineEdit->text());
    settings.setValue("Password", ui->passwordLineEdit->text());
    settings.setValue("Signature", ui->signatureLineEdit->text());
    settings.setValue("AppId", ui->appIdLineEdit->text());
    settings.setValue("Subject", ui->subjectLineEdit->text());
    settings.endGroup();

    delete ui;
}

void MainWindow::getBalanceDone(const NS__GetBalanceResponseType &getBalanceResponse)
{
    CC__BasicAmountType balance = getBalanceResponse.balance();
    ui->balanceLineEdit->setText(QString("%1 %2")
                                 .arg(balance.value())
                                 .arg(balance.currencyID().serialize().toString()));
}

void MainWindow::getBalanceError(const KDSoapMessage &fault)
{
    ui->balanceLineEdit->clear();

    processFaultMessage(fault);
}

void MainWindow::on_getBalancePushButton_clicked()
{
    NS__GetBalanceReq req;
    NS__GetBalanceRequestType request;
    prepareRequest(request);
    req.setGetBalanceRequest(request);
    service.asyncGetBalance(req);
}

void MainWindow::updateRequesterCredentials()
{
    EBL__CustomSecurityHeaderType requesterCredentials;
    EBL__UserIdPasswordType credentials;
    credentials.setUsername(ui->usernameLineEdit->text());
    credentials.setPassword(ui->passwordLineEdit->text());
    QString signature = ui->signatureLineEdit->text();
    if(!signature.isEmpty())
    {
        credentials.setSignature(signature);
    }
    QString appId = ui->appIdLineEdit->text();
    if(!appId.isEmpty())
    {
        credentials.setAppId(appId);
    }
    QString subject = ui->subjectLineEdit->text();
    if(!subject.isEmpty())
    {
        credentials.setSubject(subject);
    }
    requesterCredentials.setCredentials(credentials);
    KDSoapValue value = requesterCredentials.serialize("RequesterCredentials");
    KDSoapMessage message;
    message.addArgument("RequesterCredentials", value.childValues(),
                        QString::fromLatin1("urn:ebay:api:PayPalAPI"),
                        QString::fromLatin1("ebl:CustomSecurityHeaderType"));
    service.clientInterface()->setHeader("RequesterCredentials", message);
}

void MainWindow::processFaultMessage(const KDSoapMessage &fault)
{
    qDebug() << "Error:" << fault.faultAsString();
}

void MainWindow::on_endpointComboBox_activated(const QString &arg1)
{
    service.clientInterface()->setEndPoint(arg1);
}

void MainWindow::on_endpointComboBox_activated(int index)
{
    ui->styleComboBox->setCurrentIndex(index ? 1 : 0);
}

void MainWindow::on_styleComboBox_currentIndexChanged(int index)
{
    service.clientInterface()->setStyle(
                index ? KDSoapClientInterface::DocumentStyle : KDSoapClientInterface::RPCStyle);
}

void MainWindow::prepareRequest(EBL__AbstractRequestType &request)
{
    // Version from PayPalSvc.wsdl
    request.setVersion(QLatin1String("204.0"));
}
