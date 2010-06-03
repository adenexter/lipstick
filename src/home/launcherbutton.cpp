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

#include "launcherbutton.h"
#include "launcher.h"
#include <MDesktopEntry>

LauncherButton::LauncherButton(MWidget *parent) : MButton(parent, new LauncherButtonModel)
{
    // Connect the button clicked signal to launch application slot.
    connect(this, SIGNAL(clicked()), this, SLOT(launch()));
}

LauncherButton::LauncherButton(const QString &desktopEntryPath, MWidget *parent) : MButton(parent, new LauncherButtonModel)
{
    // Connect the button clicked signal to launch application slot.
    connect(this, SIGNAL(clicked()), this, SLOT(launch()));

    updateFromDesktopEntry(desktopEntryPath);
}

LauncherButton::~LauncherButton()
{
}

void LauncherButton::setTargetType(const QString &type)
{
    model()->setTargetType(type);
}

QString LauncherButton::targetType() const
{
    return model()->targetType();
}

void LauncherButton::setTarget(const QString &target)
{
    model()->setTarget(target);
}

QString LauncherButton::target() const
{
    return model()->target();
}

QString LauncherButton::desktopEntry() const
{
    return model()->desktopEntryFile();
}

void LauncherButton::launch()
{
    if (targetType() == "Application") {
        Launcher::startApplication(target());
    } else if (targetType() == "Service") {
        Launcher::startMApplication(target());
    }
}

void LauncherButton::updateFromDesktopEntry(const QString &desktopEntryPath)
{
    // only update if not initialized yet or if from the same desktop entry
    if (model()->desktopEntryFile().isEmpty() || desktopEntryPath == model()->desktopEntryFile()) {
        MDesktopEntry entry(desktopEntryPath);
        setTargetType(entry.type());
        setText(entry.name());

        if (!entry.icon().isEmpty()) {
            setIconID(entry.icon());
        } else {
            // FIXME: change to use correct default icon id when available
            // as incorrect id icon-Application-Default will load default icon
            // (at the moment default icon seems to be icon-l-video)
            setIconID("icon-Application-Default");
        }

        // Set target based on type
        if (entry.type() == "Application") {
            QString thisXMaemoService = entry.xMaemoService();

            if (thisXMaemoService.isEmpty()) {
                setTarget(entry.exec());
            } else {
                setTarget(thisXMaemoService);
                setTargetType("Service");
            }
        }

        model()->setDesktopEntryFile(entry.fileName());
    }
}