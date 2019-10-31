/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Copyright (C) 2012 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "settingsdialog.h"
#include <algorithm>

#include <QLabel>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

#define LENGHT 6700
#define A_COLOR QColor(255, 0, 0, 127)
#define B_COLOR QColor(0, 0, 255, 127)
#define C_COLOR QColor(0, 255, 0, 127)
#define D_COLOR QColor(255, 255, 0, 127)
#define E_COLOR QColor(0, 255, 255, 127)
#define F_COLOR QColor(255, 0, 255, 127)
#define ROW_TIME_FORMAT "hh:mm:ss.z"
#define FILENAME_TIME_FORMAT "YYYY/MM/DD hh:mm"

/****************** ATTENTION    *******************/
/*     ASCII FORM FOR UART "155 255 123 012\r\n"        */
/*     HEX FORM FOR UART "'a'0xff0xff0xff0xff\n"        */

//! [0]
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_status(new QLabel),
    m_console(new Console),
    m_settings(new SettingsDialog),
//! [1]
    m_serial(new QSerialPort(this))
//! [1]
{
//! [0]
    m_ui->setupUi(this);
    m_console->setEnabled(false);
    //setCentralWidget(m_console);

    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionQuit->setEnabled(true);
    m_ui->actionConfigure->setEnabled(true);

    m_ui->statusBar->addWidget(m_status);

    customPlot = new QCustomPlot();
    setupGraph();
    customPlot->addGraph();
    MainWindow::setupGraph();

    initActionsConnections();

    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::handleError);

//! [2]
    // connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
//! [2]
    connect(m_console, &Console::getData, this, &MainWindow::writeData);

//! [3]
}
//! [3]

MainWindow::~MainWindow()
{
    delete m_settings;
    delete m_ui;
}

//! [4]
void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_console->setEnabled(true);
        m_console->setLocalEchoEnabled(p.localEchoEnabled);
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->actionConfigure->setEnabled(false);
        showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), m_serial->errorString());

        showStatusMessage(tr("Open error"));
    }
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::readData));
    timer->start(900);


}
//! [4]

//! [5]
void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
        m_serial->close();
    m_console->setEnabled(false);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->actionConfigure->setEnabled(true);
    showStatusMessage(tr("Disconnected"));
    disconnect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::readData));
    timer->stop();
}
//! [5]

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Simple Terminal"),
                       tr("The <b>Simple Terminal</b> example demonstrates how to "
                          "use the Qt Serial Port module in modern GUI applications "
                          "using Qt, with a menu bar, toolbars, and a status bar."));
}

//! [6]
void MainWindow::writeData(const QByteArray &data)
{
    m_serial->write(data);
    m_serial->waitForReadyRead(10);
    QByteArray new_data = m_serial->read(6);
    qDebug() << "read6: " << new_data;
}
//! [6]

//! [7]
void MainWindow::readData()
{
    const QByteArray data = m_serial->readLine();
    m_console->putData(data);
    qDebug() << "read" << data;
    qDebug() << "time" << timer;
    qDebug() << "data.size" << data.size();


    m_serial->waitForReadyRead(10);

    // test_valid
    if(std::find(data.begin(), data.end(), 'a')
       && std::find(data.begin(), data.end(), 'b')
       && std::find(data.begin(), data.end(), 'c')
       && std::find(data.begin(), data.end(), 'd')
       && std::find(data.begin(), data.end(), 'd')
       && std::find(data.begin(), data.end(), 'e')
       && std::find(data.begin(), data.end(), 'f')
       && std::find(data.begin(), data.end(), '\n')
       && data.size() == 25) {
       qDebug() << "validate" << true;
    } else {
       qDebug() << "validate" << false;
       return;
    }

    int chA = (int(data[1]) - 48) * 100 + (int(data[2]) - 48) * 10 + (int(data[3]) - 48);
    int chB = (int(data[5]) - 48) * 100 + (int(data[6]) - 48) * 10 + (int(data[7]) - 48);
    int chC = (int(data[9]) - 48) * 100 + (int(data[10]) - 48) * 10 + (int(data[11]) - 48);
    int chD = (int(data[13]) - 48) * 100 + (int(data[14]) - 48) * 10 + (int(data[15]) - 48);
    int chE = (int(data[17]) - 48) * 100 + (int(data[18]) - 48) * 10 + (int(data[19]) - 48);
    int chF = (int(data[21]) - 48) * 100 + (int(data[22]) - 48) * 10 + (int(data[23]) - 48);


//    m_ui->customPlot->graph(0)->addData(timer, dataCh1);
//    m_ui->customPlot->replot();

    static QTime time(QTime::currentTime());
    // calculate two new data points:

    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
      // add data to lines:
      m_ui->customPlot->graph(0)->addData(key, chA);
      m_ui->customPlot->graph(1)->addData(key, chB);
      m_ui->customPlot->graph(2)->addData(key, chC);
      m_ui->customPlot->graph(3)->addData(key, chD);
      m_ui->customPlot->graph(4)->addData(key, chE);
      m_ui->customPlot->graph(5)->addData(key, chF);
      // rescale value (vertical) axis to fit the current data:
      //ui->customPlot->graph(0)->rescaleValueAxis();
      //ui->customPlot->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      m_ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(m_ui->customPlot->graph(0)->data()->size()+m_ui->customPlot->graph(1)->data()->size())
            , 0);
      lastFpsKey = key;
      frameCount = 0;

    }
    m_ui->customPlot->rescaleAxes();
    m_ui->customPlot->replot();

    QVector<QString> mapped_data;
    mapped_data.append(time.currentTime().toString(ROW_TIME_FORMAT));
    mapped_data.append(QVariant(chA).toString());
    mapped_data.append(QVariant(chB).toString());
    mapped_data.append(QVariant(chC).toString());
    mapped_data.append(QVariant(chD).toString());
    mapped_data.append(QVariant(chE).toString());
    mapped_data.append(QVariant(chF).toString());
    appendDataInFile(mapped_data);
    qDebug() << "time: " << time.currentTime().toString(ROW_TIME_FORMAT);
}
//! [7]

void MainWindow::appendDataInFile(const QVector<QString> &data) {
    QString path("data/");
    QDir dir; // Initialize to the desired dir if 'path' is relative
              // By default the program's working directory "." is used.

    // We create the directory if needed
    if (!dir.exists(path))
        dir.mkpath(path); // You can check the success if needed

    //QString fileName = QString( "%1%2" ).arg("some_file_name").arg( ".dat" );
    QString filename = "data.csv";

    QFile file(path + filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&file);
        foreach (auto i, data) {
            out << i << ";";
        }
        out << "\n";

    }
    file.close();
}
//! [8]
void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), m_serial->errorString());
        closeSerialPort();
    }
}
//! [8]

void MainWindow::initActionsConnections()
{
    connect(m_ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(m_ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(m_ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::show);
    connect(m_ui->actionClear, &QAction::triggered, m_console, &Console::clear);
    connect(m_ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);

    connect(m_ui->checkBoxChannelA, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    connect(m_ui->checkBoxChannelB, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    connect(m_ui->checkBoxChannelC, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    connect(m_ui->checkBoxChannelD, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    connect(m_ui->checkBoxChannelE, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    connect(m_ui->checkBoxChannelF, &QCheckBox::stateChanged, this, &MainWindow::updateChannels);
    m_ui->checkBoxChannelA->setStyleSheet("QCheckBox { color: red }");
    m_ui->checkBoxChannelB->setStyleSheet("QCheckBox { color: blue }");
    m_ui->checkBoxChannelC->setStyleSheet("QCheckBox { color: green }");
    m_ui->checkBoxChannelD->setStyleSheet("QCheckBox { color: yellow }");
    m_ui->checkBoxChannelE->setStyleSheet("QCheckBox { color: cyan }");
    m_ui->checkBoxChannelF->setStyleSheet("QCheckBox { color: violet }");

}

void MainWindow::showStatusMessage(const QString &message)
{
    m_status->setText(message);
}

void MainWindow::updateChannels()
{
    m_ui->customPlot->graph(0)->setVisible(m_ui->checkBoxChannelA->isChecked());
    m_ui->customPlot->graph(1)->setVisible(m_ui->checkBoxChannelB->isChecked());
    m_ui->customPlot->graph(2)->setVisible(m_ui->checkBoxChannelC->isChecked());
    m_ui->customPlot->graph(3)->setVisible(m_ui->checkBoxChannelD->isChecked());
    m_ui->customPlot->graph(4)->setVisible(m_ui->checkBoxChannelE->isChecked());
    m_ui->customPlot->graph(5)->setVisible(m_ui->checkBoxChannelF->isChecked());
    m_ui->customPlot->replot();
}

void MainWindow::setupGraph()
{
    m_ui->customPlot->addGraph(); // red line
    m_ui->customPlot->graph(0)->setPen(QPen(A_COLOR));
    m_ui->customPlot->addGraph(); // blue line
    m_ui->customPlot->graph(1)->setPen(QPen(B_COLOR));
    m_ui->customPlot->addGraph(); // green line
    m_ui->customPlot->graph(2)->setPen(QPen(C_COLOR));
    m_ui->customPlot->addGraph(); // yellow line
    m_ui->customPlot->graph(3)->setPen(QPen(D_COLOR));
    m_ui->customPlot->addGraph(); // tea line
    m_ui->customPlot->graph(4)->setPen(QPen(E_COLOR));
    m_ui->customPlot->addGraph(); // violet line
    m_ui->customPlot->graph(5)->setPen(QPen(F_COLOR));

    m_ui->customPlot->xAxis->setLabel("Время, чч:мм:сс");
    m_ui->customPlot->yAxis->setLabel("Температура, \u2103");
    m_ui->customPlot->setInteraction(QCP::iRangeZoom,true);   // Включаем взаимодействие удаления/приближения
    m_ui->customPlot->setInteraction(QCP::iRangeDrag, true);  // Включаем взаимодействие перетаскивания графика
    m_ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal);   // Включаем перетаскивание только по горизонтальной оси
    m_ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal);   // Включаем удаление/приближение только по горизонтальной оси


    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    m_ui->customPlot->xAxis->setTicker(timeTicker);
    m_ui->customPlot->axisRect()->setupFullAxesBox();
    m_ui->customPlot->yAxis->setRange(0, 300);

}
