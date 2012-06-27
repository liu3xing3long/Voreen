/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Copyright (C) 2005-2009 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/qt/widgets/network/editor/textgraphicsitem.h"

#include "voreen/qt/widgets/network/editor/processorgraphicsitem.h"
#include <QKeyEvent>
#include <QFont>

namespace {
    QColor textColor = Qt::white;
    int fontSize = 10;
}

namespace voreen {

TextGraphicsItem::TextGraphicsItem(const QString& text, ProcessorGraphicsItem* parent)
    : QGraphicsTextItem(text, parent)
{
    setFlag(ItemIsSelectable, false);
    setDefaultTextColor(textColor);
    setFont(QFont("Helvetica", fontSize));
}

void TextGraphicsItem::keyPressEvent(QKeyEvent* event) {
    event->accept();
    if (event->key() == Qt::Key_Escape) {
        // restore saved text
        setPlainText(previousText_);
        emit renameFinished();
    }
    else if ((event->key() == Qt::Key_Return) && (event->modifiers() == Qt::NoModifier)) {
        previousText_ = toPlainText();
        emit renameFinished();
    }
    else {
        QGraphicsTextItem::keyPressEvent(event);
        emit textChanged();
    }
}

int TextGraphicsItem::type() const {
    return Type;
}

void TextGraphicsItem::setFocus(Qt::FocusReason focusReason) {
    // save old text
    previousText_ = toPlainText();
    QGraphicsTextItem::setFocus(focusReason);
}

void TextGraphicsItem::setPlainText(const QString& text) {
    previousText_ = text;
    QGraphicsTextItem::setPlainText(text);
}

} // namespace
