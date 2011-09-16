/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of mhome.
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "mainwindow.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QIcon>
#include <QX11Info>
#include <QDebug>

#include "homeapplication.h"
#include "homescreenservice.h"
#ifdef HAS_ADAPTER
#include "homescreenadaptor.h"
#endif
#include "windowinfo.h"
#include "xeventlistener.h"
#include "x11wrapper.h"

/*!
 * D-Bus names for the home screen service
 */
static const QString MEEGO_CORE_HOME_SCREEN_SERVICE_NAME = "com.meego.core.HomeScreen";
static const QString MEEGO_CORE_HOME_SCREEN_OBJECT_PATH = "/homescreen";

/*!
 * D-Bus names for the notification that's sent when home is ready
 */
static const QString HOME_READY_SIGNAL_PATH = "/com/nokia/duihome";
static const QString HOME_READY_SIGNAL_INTERFACE = "com.nokia.duihome.readyNotifier";
static const QString HOME_READY_SIGNAL_NAME = "ready";

HomeApplication::HomeApplication(int &argc, char **argv)
    : QApplication(argc, argv)
    , upstartMode(false)
    , lockedOrientation_(QVariant::Invalid)
    , homeScreenService(new HomeScreenService)
    , xEventListeners()
    , iteratorActiveForEventListenerContainer(false)
    , toBeRemovedEventListeners()
    , xDamageEventBase(0)
    , xDamageErrorBase(0)
{
    XDamageQueryExtension(QX11Info::display(), &xDamageEventBase, &xDamageErrorBase);

    parseArguments(argc, argv);
    // launch a timer for sending a dbus-signal upstart when basic construct is done
    connect(&startupNotificationTimer, SIGNAL(timeout()),
            this, SLOT(sendStartupNotifications()));
    startupNotificationTimer.setSingleShot(true);
    startupNotificationTimer.setInterval(0);
    startupNotificationTimer.start();

#ifdef HAS_ADAPTER
    new HomeScreenAdaptor(homeScreenService);
#endif

    QDBusConnection connection = QDBusConnection::sessionBus();

    connection.registerService(MEEGO_CORE_HOME_SCREEN_SERVICE_NAME);
    connection.registerObject(MEEGO_CORE_HOME_SCREEN_OBJECT_PATH, homeScreenService);

    connect(homeScreenService, SIGNAL(focusToLauncherApp(const QString&)), this, SIGNAL(focusToLauncherAppRequested(const QString &)));

    // Initialize the X11 atoms used in the UI components
    WindowInfo::initializeAtoms();
}

HomeApplication::~HomeApplication()
{
    delete homeScreenService;
}

void HomeApplication::addXEventListener(XEventListener *listener)
{
    if (listener != NULL && !xEventListeners.contains(listener)) {
        xEventListeners.append(listener);
    }
}

void HomeApplication::removeXEventListener(XEventListener *listener)
{
    if (iteratorActiveForEventListenerContainer) {
        toBeRemovedEventListeners.append(listener);
    } else {
        xEventListeners.removeOne(listener);
    }
}

QVariant HomeApplication::lockedOrientation() const
{
    return lockedOrientation_;
}

void HomeApplication::sendStartupNotifications()
{
    static QDBusConnection systemBus = QDBusConnection::systemBus();
    QDBusMessage homeReadySignal =
        QDBusMessage::createSignal(HOME_READY_SIGNAL_PATH,
                                   HOME_READY_SIGNAL_INTERFACE,
                                   HOME_READY_SIGNAL_NAME);
    systemBus.send(homeReadySignal);

    // Stop the application after the basic construction but only when run by upstart
    if (upstartMode) {
        static pid_t selfPid = getpid();
        kill(selfPid, SIGSTOP);
    }

    // For device boot performance reasons initializing Home scene window must be done
    // only after ready signal is sent (NB#277602)
    MainWindow::instance(true)->show();
}

bool HomeApplication::x11EventFilter(XEvent *event)
{
    bool eventHandled = false;
    iteratorActiveForEventListenerContainer = true;

    if (event->type == xDamageEventBase + XDamageNotify) {
        qDebug() << Q_FUNC_INFO << "Processing damage event";
        XDamageNotifyEvent *xevent = (XDamageNotifyEvent *) event;

        // xevent->more would inform us if there is more events for the
        // rendering operation. but there isn't interface to pass the
        // information to damageEvent.
        emit damageEvent(xevent->damage, xevent->area.x, xevent->area.y, xevent->area.width, xevent->area.height);
        eventHandled = true;
    }


    foreach (XEventListener* listener, xEventListeners) {
        if (!toBeRemovedEventListeners.contains(listener)) {
            if (listener->handleXEvent(*event)) {
                eventHandled = true;
            }
        }
    }
    iteratorActiveForEventListenerContainer = false;

    // Remove now any event listeners that got removed while going through the event listeners
    foreach (XEventListener* listener, toBeRemovedEventListeners) {
        xEventListeners.removeOne(listener);
    }
    toBeRemovedEventListeners.clear();

    if (!eventHandled) {
        eventHandled = QApplication::x11EventFilter(event);
    }

    return eventHandled;
}

void HomeApplication::parseArguments(int argc, char *argv[])
{
    if (argc >= 2) {
        static const char upstartChar = 'u';
        static const char orientationChar = 'o';
        static const char *optString = "uo::";
        static struct option optLong[] = {
            { "upstart", 0, NULL, upstartChar },
            { "locked-orientation", 2, NULL, orientationChar },
            { 0, 0, 0, 0 }
        };
        opterr = 0;
        int c = 0;
        while ((c = getopt_long_only(argc, argv, optString, optLong, 0)) != -1) {
            switch (c) {
            case upstartChar:
                upstartMode = true;
                break;
            case orientationChar:
                lockedOrientation_ = QVariant(optarg);
                break;
            default:
                break;
            }
        }
    }
}
