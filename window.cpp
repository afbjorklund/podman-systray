/****************************************************************************
**
** Copyright (C) 2021 Anders F Björklund
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
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

#include "window.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QCloseEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QDebug>
#include <QRegExp>

//! [0]
Window::Window()
{
    textLabel = new QLabel();
    textLabel->setFixedWidth(200);
    textLabel->setFixedHeight(50);
    QPixmap textPixmap(":/images/text.png");
    int w = textLabel->width();
    int h = textLabel->height();
    textLabel->setPixmap(textPixmap.scaled(w, h, Qt::KeepAspectRatio));

    createMachineGroupBox();
    createConnectionGroupBox();

    updateName();
    updateStatus();
    updateConnections();

    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/podman.png"), tr("Podman"));

    createActions();
    createTrayIcon();

    connect(updateButton, &QAbstractButton::clicked, this, &Window::updateStatus);
    connect(connectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Window::setConnection);
    connect(startButton, &QAbstractButton::clicked, this, &Window::startMachine);
    connect(stopButton, &QAbstractButton::clicked, this, &Window::stopMachine);
    connect(initButton, &QAbstractButton::clicked, this, &Window::initMachine);
    connect(removeButton, &QAbstractButton::clicked, this, &Window::removeMachine);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(textLabel);
    mainLayout->addWidget(machineGroupBox);
    mainLayout->addWidget(connectionGroupBox);
    setLayout(mainLayout);

    setIcon(0);
    trayIcon->show();

    setWindowTitle(tr("Podman"));
    resize(400, 300);
}
//! [0]

//! [1]
void Window::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    QDialog::setVisible(visible);
}
//! [1]

//! [2]
void Window::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        event->ignore();
    }
}
//! [2]

//! [3]
void Window::setIcon(int index)
{
    QIcon icon = iconComboBox->itemIcon(index);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);

    trayIcon->setToolTip(iconComboBox->itemText(index));
}
//! [3]

//! [4]
void Window::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
        break;
    default:
        ;
    }
}
//! [4]

void Window::createMachineGroupBox()
{
    machineGroupBox = new QGroupBox(tr("Machine"));

    QIcon updateIcon = QIcon(":/images/view-refresh.png");
    updateButton = new QPushButton(updateIcon, "");
    updateButton->setFixedWidth(32);
    nameLabel = new QLabel("");
    statusLabel = new QLabel("Unknown");

    startButton = new QPushButton(tr("Start"));
    stopButton = new QPushButton(tr("Stop"));
    initButton = new QPushButton(tr("Init"));
    removeButton = new QPushButton(tr("Remove"));

    versionLabel = new QLabel();
    osReleaseLabel = new QLabel();

    QGridLayout *machineLayout = new QGridLayout;
    machineLayout->addWidget(nameLabel, 0, 0, 1, 5);
    machineLayout->addWidget(updateButton, 1, 0, 1, 4);
    machineLayout->addWidget(statusLabel, 1, 1);
    machineLayout->addWidget(startButton, 2, 2);
    machineLayout->addWidget(stopButton, 2, 3);
    machineLayout->addWidget(versionLabel, 2, 0, 2, 2);
    machineLayout->addWidget(initButton, 3, 2);
    machineLayout->addWidget(removeButton, 3, 3);
    machineLayout->addWidget(osReleaseLabel, 4, 0, 1, 4);
    machineGroupBox->setLayout(machineLayout);
}

void Window::createConnectionGroupBox()
{
    connectionGroupBox = new QGroupBox(tr("Connection"));

    connectionComboBox = new QComboBox;

    QVBoxLayout *connectionLayout = new QVBoxLayout;
    connectionLayout->addWidget(connectionComboBox);
    connectionGroupBox->setLayout(connectionLayout);
}

bool Window::getProcessOutput(QStringList arguments, QString& text) {
    bool success;

    QString program = "podman";

    QProcess *process = new QProcess(this);
    process->start(program, arguments);
    success = process->waitForFinished();
    if (success) {
        text = process->readAllStandardOutput();
    } else {
        qDebug() << process->readAllStandardError();
    }
    delete process;
    return success;
}

void Window::updateName()
{
    QStringList arguments;
    arguments << "machine" << "list" << "--noheading" << "--format" << "{{.Name}}";

    QString text;
    bool success = getProcessOutput(arguments, text);
    if (success) {
        QString name = text.trimmed();
        if (name.endsWith("*")) {
            name.chop(1);
        }
        nameLabel->setText(name);
    } else {
        nameLabel->clear();
    }
}

void Window::updateStatus()
{
    QStringList arguments;
    arguments << "machine" << "list" << "--noheading" << "--format" << "{{.LastUp}}";

    QString *text = new QString();
    bool success = getProcessOutput(arguments, *text);
    if (success) {
        if (text == QString("Currently running\n")) {
            updateVersion();
            updateConnections();
            statusLabel->setText(tr("Running"));
            startButton->setEnabled(false);
            stopButton->setEnabled(true);
            initButton->setEnabled(false);
            removeButton->setEnabled(false);
        } else if (!text->isEmpty()){
            statusLabel->setText(tr("Not Running"));
            startButton->setEnabled(true);
            stopButton->setEnabled(false);
            initButton->setEnabled(false);
            removeButton->setEnabled(true);
        } else {
            clearVersion();
            clearConnections();
            statusLabel->setText(tr("Not Initialized"));
            startButton->setEnabled(false);
            stopButton->setEnabled(false);
            initButton->setEnabled(true);
            removeButton->setEnabled(false);
        }
    } else {
        startButton->setEnabled(false);
        stopButton->setEnabled(false);
        initButton->setEnabled(false);
        removeButton->setEnabled(false);
    }
    delete text;
}

QString Window::prettyName(QString release)
{
    QRegExp re("PRETTY_NAME=\"([^\"]*)\"");
    if (release.contains(re)) {
        return re.cap(1);
    }
    return "";
}

void Window::updateVersion()
{
    bool success;

    QString program = "podman";
    QStringList arguments;
    arguments << "machine" << "ssh" << "podman" << "--version";

    QString version;
    success = getProcessOutput(arguments, version);
    if (success) {
        versionLabel->setText(version);
    }

    arguments.clear();
    arguments << "machine" << "ssh" << "cat" << "/etc/os-release";

    QString release;
    success = getProcessOutput(arguments, release);
    if (success) {
        osReleaseLabel->setText(prettyName(release));
    }
}

void Window::clearVersion()
{
    versionLabel->setText("");
    osReleaseLabel->setText("");
}

void Window::updateConnections()
{
    QStringList arguments;
    arguments << "system" << "connection" << "list" << "--format" << "{{.Name}}";

    QString *text = new QString();
    bool success = getProcessOutput(arguments, *text);
    if (success) {
        connectionComboBox->clear();
        QStringList lines = text->split("\n");
        for (int i = 0; i < lines.length() - 1; i++)
        {
            QString name = lines[i];
            bool defaultConnection = false;
            if (name.endsWith("*")) {
                name.chop(1);
                defaultConnection = true;
            }
            connectionComboBox->addItem(name);
            if (defaultConnection) {
                connectionComboBox->setCurrentIndex(i);
            }
        }
    }
    delete text;
}

void Window::clearConnections()
{
    connectionComboBox->clear();
    connectionComboBox->setEnabled(false);
}

void Window::setConnection(int index)
{
    QString conn = connectionComboBox->itemText(index);

    QStringList arguments;
    arguments << "system" << "connection" << "default" << conn;

    QString text;
    bool success = getProcessOutput(arguments, text);
    if (!success) {
        QMessageBox::warning(this, tr("Podman"),
                             tr("Unable to set connection") +
                             QString(" ") + QString(conn));
    }
}

void Window::sendMachineCommand(QString cmd)
{
    sendMachineCommand(QStringList(cmd));
}

void Window::sendMachineCommand(QStringList cmds)
{
    QString program = "podman";
    QStringList arguments;
    arguments << "machine" << cmds;
    bool success;

    QProcess *process = new QProcess(this);
    process->start(program, arguments);
    this->setCursor(Qt::WaitCursor);
    int timeout = 30;
    if (cmds[0] == QString("init")) {
            // this might take a while
            timeout = 300;
    }
    success = process->waitForFinished(timeout * 1000);
    if (cmds[0] == QString("stop")) {
            // command returns too quick
            QThread::sleep(1);
    }
    this->unsetCursor();

    if (!success) {
        qDebug() << process->readAllStandardOutput();
        qDebug() << process->readAllStandardError();
    }
}

void Window::startMachine()
{
    sendMachineCommand(QString("start"));
    updateStatus();
}

void Window::stopMachine()
{
    sendMachineCommand(QString("stop"));
    updateStatus();
}

void Window::initMachine()
{
    sendMachineCommand(QString("init"));
    updateStatus();
}

bool Window::askConfirm()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(tr("Remove an existing machine"));
    msgBox.setInformativeText(tr("Are you sure you want to continue? This will delete all files for the podman machine."));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    return msgBox.exec() == QMessageBox::Ok;
}

void Window::removeMachine()
{
    if (askConfirm()) {
        QStringList args = {"rm", "--force"};
        sendMachineCommand(args);
    }
    updateStatus();
}

void Window::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);

    maximizeAction = new QAction(tr("Ma&ximize"), this);
    connect(maximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Window::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

#endif
