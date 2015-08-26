#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    aboutWindow(0),
    newProjectWindow(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNew_Project_triggered()
{
    if (!this->newProjectWindow)
    {
        this->newProjectWindow = new NewProjectWindow(this);
    }

    this->newProjectWindow->show();
    this->newProjectWindow->raise();
    this->newProjectWindow->activateWindow();
}

void MainWindow::on_actionAbout_triggered()
{
    if (!this->aboutWindow)
    {
        this->aboutWindow = new AboutWindow(this);
    }

    this->aboutWindow->show();
    this->aboutWindow->raise();
    this->aboutWindow->activateWindow();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
