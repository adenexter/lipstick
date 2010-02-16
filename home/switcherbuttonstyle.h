/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of duihome.
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

#ifndef SWITCHERBUTTONSTYLE_H_
#define SWITCHERBUTTONSTYLE_H_

#include <QFont>
#include <QColor>
#include <QPointF>
#include <QSize>
#include <duibuttonstyle.h>

class QPixmap;
class DuiScalableImage;

class SwitcherButtonStyle : public DuiButtonStyle
{
    Q_OBJECT
    DUI_STYLE(SwitcherButtonStyle)

    //! The opacity of the title
    DUI_STYLE_ATTRIBUTE(qreal, textOpacity, TextOpacity)

    //! The position of the icon relative to the top-left corner of the button
    DUI_STYLE_ATTRIBUTE(QPointF, iconPosition, IconPosition)

    //! The image to be used for masking the window thumbnail
    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage *, maskImage, MaskImage)

    //! The image to be drawn as the container for the button
    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage *, containerImage, ContainerImage)

    //! The image to be drawn on top of the button when the button is dragged over the close area
    DUI_STYLE_PTR_ATTRIBUTE(DuiScalableImage*, closeImage, CloseImage)
};

class SwitcherButtonStyleContainer : public DuiButtonStyleContainer
{
    DUI_STYLE_CONTAINER(SwitcherButtonStyle)
    DUI_STYLE_MODE(Emphasized)
};

#endif /* SWITCHERBUTTONSTYLE_H_ */
