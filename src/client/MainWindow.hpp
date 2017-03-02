#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "PayPalSvc.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void getBalanceDone( const NS__GetBalanceResponseType& getBalanceResponse );
    void getBalanceError( const KDSoapMessage& fault );

private:
    Ui::MainWindow *ui;

    PayPalAPIInterfaceService::PayPalAPISoapBinding service;
};

#endif // MAINWINDOW_HPP
