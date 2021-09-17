/****************************************************************************
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

    createStatusGroupBox();
    createIconGroupBox();
    createMessageGroupBox();

    iconLabel->setMinimumWidth(durationLabel->sizeHint().width());

    createActions();
    createTrayIcon();

    connect(updateButton, &QAbstractButton::clicked, this, &Window::updateStatus);
    connect(startButton, &QAbstractButton::clicked, this, &Window::startMachine);
    connect(stopButton, &QAbstractButton::clicked, this, &Window::stopMachine);
    connect(initButton, &QAbstractButton::clicked, this, &Window::initMachine);
    connect(removeButton, &QAbstractButton::clicked, this, &Window::removeMachine);
    connect(showMessageButton, &QAbstractButton::clicked, this, &Window::showMessage);
    connect(showIconCheckBox, &QAbstractButton::toggled, trayIcon, &QSystemTrayIcon::setVisible);
    connect(iconComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Window::setIcon);
    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &Window::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Window::iconActivated);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(textLabel);
    mainLayout->addWidget(statusGroupBox);
    //mainLayout->addWidget(iconGroupBox);
    //mainLayout->addWidget(messageGroupBox);
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
    case QSystemTrayIcon::MiddleClick:
        showMessage();
        break;
    default:
        ;
    }
}
//! [4]

//! [5]
void Window::showMessage()
{
    showIconCheckBox->setChecked(true);
    int selectedIcon = typeComboBox->itemData(typeComboBox->currentIndex()).toInt();
    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(selectedIcon);

    if (selectedIcon == -1) { // custom icon
        QIcon icon(iconComboBox->itemIcon(iconComboBox->currentIndex()));
        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), icon,
                          durationSpinBox->value() * 1000);
    } else {
        trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), msgIcon,
                          durationSpinBox->value() * 1000);
    }
}
//! [5]

//! [6]
void Window::messageClicked()
{
    QMessageBox::information(0, tr("Systray"),
                             tr("Sorry, I already gave what help I could.\n"
                                "Maybe you should try asking a human?"));
}
//! [6]

void Window::createStatusGroupBox()
{
    statusGroupBox = new QGroupBox(tr("Status"));

    QIcon updateIcon = QIcon::fromTheme("view-refresh");
    updateButton = new QPushButton(updateIcon, "");
    updateButton->setFixedWidth(32);
    statusLabel = new QLabel("Unknown");

    startButton = new QPushButton(tr("Start"));
    stopButton = new QPushButton(tr("Stop"));
    initButton = new QPushButton(tr("Init"));
    removeButton = new QPushButton(tr("Remove"));

    QGridLayout *statusLayout = new QGridLayout;
    statusLayout->addWidget(updateButton, 0, 0, 1, 4);
    statusLayout->addWidget(statusLabel, 0, 1);
    statusLayout->addWidget(startButton, 1, 2);
    statusLayout->addWidget(stopButton, 1, 3);
    statusLayout->addWidget(initButton, 2, 2);
    statusLayout->addWidget(removeButton, 2, 3);
    statusGroupBox->setLayout(statusLayout);

    updateStatus();
}

void Window::updateStatus()
{
    bool success;

    QString program = "podman";
    QStringList arguments;
    arguments << "machine" << "list" << "--noheading" << "--format" << "{{.LastUp}}";

    QProcess *process = new QProcess(this);
    process->start(program, arguments);
    success = process->waitForFinished();
    if (success) {
        QString *text = new QString(process->readAllStandardOutput());

        if (text == QString("Currently running\n")) {
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
            statusLabel->setText(tr("Not Initialized"));
            startButton->setEnabled(false);
            stopButton->setEnabled(false);
            initButton->setEnabled(true);
            removeButton->setEnabled(false);
        }

        delete text;
    }
    delete process;
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
    success = process->waitForFinished();
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

void Window::removeMachine()
{
    QStringList args = {"rm", "--force"};
    sendMachineCommand(args);
    updateStatus();
}

void Window::createIconGroupBox()
{
    iconGroupBox = new QGroupBox(tr("Tray Icon"));

    iconLabel = new QLabel("Icon:");

    iconComboBox = new QComboBox;
    iconComboBox->addItem(QIcon(":/images/podman.png"), tr("Podman"));

    showIconCheckBox = new QCheckBox(tr("Show icon"));
    showIconCheckBox->setChecked(true);

    QHBoxLayout *iconLayout = new QHBoxLayout;
    iconLayout->addWidget(iconLabel);
    iconLayout->addWidget(iconComboBox);
    iconLayout->addStretch();
    iconLayout->addWidget(showIconCheckBox);
    iconGroupBox->setLayout(iconLayout);
}

void Window::createMessageGroupBox()
{
    messageGroupBox = new QGroupBox(tr("Balloon Message"));

    typeLabel = new QLabel(tr("Type:"));

    typeComboBox = new QComboBox;
    typeComboBox->addItem(tr("None"), QSystemTrayIcon::NoIcon);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxInformation), tr("Information"),
            QSystemTrayIcon::Information);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxWarning), tr("Warning"),
            QSystemTrayIcon::Warning);
    typeComboBox->addItem(style()->standardIcon(
            QStyle::SP_MessageBoxCritical), tr("Critical"),
            QSystemTrayIcon::Critical);
    typeComboBox->addItem(QIcon(), tr("Custom icon"),
            -1);
    typeComboBox->setCurrentIndex(1);

    durationLabel = new QLabel(tr("Duration:"));

    durationSpinBox = new QSpinBox;
    durationSpinBox->setRange(5, 60);
    durationSpinBox->setSuffix(" s");
    durationSpinBox->setValue(15);

    durationWarningLabel = new QLabel(tr("(some systems might ignore this "
                                         "hint)"));
    durationWarningLabel->setIndent(10);

    titleLabel = new QLabel(tr("Title:"));

    titleEdit = new QLineEdit(tr("Cannot connect to network"));

    bodyLabel = new QLabel(tr("Body:"));

    bodyEdit = new QTextEdit;
    bodyEdit->setPlainText(tr("Don't believe me. Honestly, I don't have a "
                              "clue.\nClick this balloon for details."));

    showMessageButton = new QPushButton(tr("Show Message"));
    showMessageButton->setDefault(true);

    QGridLayout *messageLayout = new QGridLayout;
    messageLayout->addWidget(typeLabel, 0, 0);
    messageLayout->addWidget(typeComboBox, 0, 1, 1, 2);
    messageLayout->addWidget(durationLabel, 1, 0);
    messageLayout->addWidget(durationSpinBox, 1, 1);
    messageLayout->addWidget(durationWarningLabel, 1, 2, 1, 3);
    messageLayout->addWidget(titleLabel, 2, 0);
    messageLayout->addWidget(titleEdit, 2, 1, 1, 4);
    messageLayout->addWidget(bodyLabel, 3, 0);
    messageLayout->addWidget(bodyEdit, 3, 1, 2, 4);
    messageLayout->addWidget(showMessageButton, 5, 4);
    messageLayout->setColumnStretch(3, 1);
    messageLayout->setRowStretch(4, 1);
    messageGroupBox->setLayout(messageLayout);
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
