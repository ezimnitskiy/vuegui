// Copyright (c) 2015-2017, The Vuecash developers
//
// This file is part of Vuecash.
//
// Vuecash is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vuecash is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Vuecash.  If not, see <http://www.gnu.org/licenses/>.

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QTextStream>
#include <QThread>

#include "logger.h"

Q_DECLARE_LOGGING_CATEGORY(infoLogging)
Q_DECLARE_LOGGING_CATEGORY(Wallet)
Q_LOGGING_CATEGORY(Wallet, "Wallet")
Q_LOGGING_CATEGORY(infoLogging, "logger.info")

namespace WalletGUI {

namespace {

  const char OLD_LOG_FILE_NAME[] = "vuecashwalletgui.log";
  const char LOG_FILE_NAME[] = "vuecash-gui.log";

}

QThread* WalletLogger::m_loggerThread = nullptr;
WalletLogger* WalletLogger::m_instance = nullptr;
QFile* WalletLogger::m_logFile = nullptr;

void WalletLogger::init(const QDir& logDir, bool debug, QObject* parent)
{
    Q_ASSERT(m_instance == nullptr);
    m_loggerThread = new QThread(parent);
    m_instance = new WalletLogger(debug, nullptr);
    m_logFile = new QFile(nullptr);
    const QString absoluteOldFilePath = logDir.absoluteFilePath(OLD_LOG_FILE_NAME);
    const QString absoluteNewFilePath = logDir.absoluteFilePath(LOG_FILE_NAME);
    if (QFile::exists(absoluteOldFilePath))
        QFile::rename(absoluteOldFilePath, absoluteNewFilePath);

    m_logFile->setFileName(absoluteNewFilePath);
    if (!m_logFile->open(QFile::WriteOnly | QFile::Append | QFile::Text))
        fprintf(stderr, "[Logger] Can't open log file\n");

    m_instance->moveToThread(m_loggerThread);
    m_logFile->moveToThread(m_loggerThread);
    m_loggerThread->start();
}

void WalletLogger::deinit()
{
    qInstallMessageHandler(0);
    m_loggerThread->quit();
    m_loggerThread->wait();
    delete m_logFile; m_logFile = nullptr;
    delete m_loggerThread; m_loggerThread = nullptr;
    delete m_instance; m_instance = nullptr;
}

void WalletLogger::debug(const QString& message)
{
    Q_ASSERT(m_instance != nullptr);
    QMetaObject::invokeMethod(m_instance, "debugImpl", Qt::QueuedConnection, Q_ARG(QString, message));
}

void WalletLogger::info(const QString& message)
{
    Q_ASSERT(m_instance != nullptr);
    QMetaObject::invokeMethod(m_instance, "infoImpl", Qt::QueuedConnection, Q_ARG(QString, message));
}

void WalletLogger::warning(const QString& message)
{
    Q_ASSERT(m_instance != nullptr);
    QMetaObject::invokeMethod(m_instance, "warningImpl", Qt::QueuedConnection, Q_ARG(QString, message));
}

void WalletLogger::critical(const QString& message)
{
    Q_ASSERT(m_instance != nullptr);
    QMetaObject::invokeMethod(m_instance, "criticalImpl", Qt::QueuedConnection, Q_ARG(QString, message));
}

WalletLogger::WalletLogger(bool debug, QObject* parent)
    : QObject(parent)
{
    qInstallMessageHandler(&WalletLogger::messageHandler);
    if (debug)
        QLoggingCategory::setFilterRules("qt.network.ssl.warning=false\nWallet.debug=true");
    else
        QLoggingCategory::setFilterRules("qt.network.ssl.warning=false\nWallet.debug=false");
}

WalletLogger::~WalletLogger()
{
}

void WalletLogger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (!QLoggingCategory(context.category).isEnabled(type))
        return;

    QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString typeString;
    switch (type) {
    case QtDebugMsg:
        typeString = "debug";
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        typeString = "info";
        break;
#endif
    case QtWarningMsg:
#if QT_VERSION < 0x050500
        typeString = QString(context.category).compare("logger.info") ? "warning" : "info";
#else
        typeString = "warning";
#endif
        break;
    case QtCriticalMsg:
        typeString = "critical";
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s [fatal] %s\n", qPrintable(timeString), qPrintable(msg));
//        abort();
        break;
    }

    QString logString = QString("%1 [%2] %3").arg(timeString).arg(typeString).arg(msg);
    if (m_logFile->isOpen())
    {
        QTextStream logStream(m_logFile);
        logStream << logString << endl;
    }

    if (QLoggingCategory::defaultCategory()->isEnabled(QtDebugMsg))
    {
        fprintf(stderr, "%s\n", qPrintable(logString));
        fflush(stderr);
    }
}

void WalletLogger::debugImpl(const QString& message)
{
    qCDebug(Wallet) << qPrintable(message);
}

void WalletLogger::infoImpl(const QString& message)
{
#if QT_VERSION < 0x050500
    qCWarning(infoLogging) << qPrintable(message);
#else
    qCInfo(Wallet, "%s", qPrintable(message));
#endif
}

void WalletLogger::warningImpl(const QString& message)
{
    qCWarning(Wallet) << qPrintable(message);
}

void WalletLogger::criticalImpl(const QString& message)
{
    qCCritical(Wallet) << qPrintable(message);
}

}
