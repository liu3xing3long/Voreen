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

#include "voreen/core/vis/processors/image/quadview.h"

namespace voreen {

QuadView::QuadView()
    : RenderProcessor()
    , showGrid_("showGrid", "Show grid", true)
    , gridColor_("gridColor", "Grid color", tgt::vec4(1.0f, 1.0f, 1.0f, 1.0f))
    , maximized_("maximized", "Maximized sub-view", 0, 0, 4)
    , maximizeOnDoubleClick_("maximizeOnDoubleClick", "Maximize on double click", true)
    , outport_(Port::OUTPORT, "outport")
    , inport1_(Port::INPORT, "inport1")
    , inport2_(Port::INPORT, "inport2")
    , inport3_(Port::INPORT, "inport3")
    , inport4_(Port::INPORT, "inport4")
{

    addProperty(showGrid_);
    addProperty(gridColor_);
    addProperty(maximized_);
    maximized_.setVisible(false);
    addProperty(maximizeOnDoubleClick_);

    addPort(outport_);
    addPort(inport1_);
    addPort(inport2_);
    addPort(inport3_);
    addPort(inport4_);

    inport1_.sizeOriginChanged(this);
    inport2_.sizeOriginChanged(this);
    inport3_.sizeOriginChanged(this);
    inport4_.sizeOriginChanged(this);
}

QuadView::~QuadView() {
}

Processor* QuadView::create() const {
    return new QuadView();
}

const std::string QuadView::getProcessorInfo() const {
    return "Allows to arrange four renderings in one multiview layout.";
}

bool QuadView::isReady() const {
    if (!outport_.isReady())
        return false;

    if (!inport1_.isReady() && !inport2_.isReady() && !inport3_.isReady() && !inport4_.isReady())
        return false;

    if(maximized_.get() == 0) {
        switch(maximized_.get()) {
            case 1: if(!inport1_.isReady())
                        return false;
                    break;
            case 2: if(!inport2_.isReady())
                        return false;
                    break;
            case 3: if(!inport3_.isReady())
                        return false;
                    break;
            case 4: if(!inport4_.isReady())
                        return false;
                    break;
        }
    }
    return true;
}

void QuadView::process() {
    if(maximized_.get() == 0) {
        glMatrixMode(GL_MODELVIEW);
        outport_.activateTarget();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (inport1_.isReady()) {
            inport1_.bindColorTexture(GL_TEXTURE0);
            inport1_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(-0.5f, 0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport1_.getColorTexture()->disable();
        }

        if (inport2_.isReady()) {
            inport2_.bindColorTexture(GL_TEXTURE0);
            inport2_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(0.5f, 0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport2_.getColorTexture()->disable();
        }

        if (inport3_.isReady()) {
            inport3_.bindColorTexture(GL_TEXTURE0);
            inport3_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(-0.5f, -0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport3_.getColorTexture()->disable();
        }

        if (inport4_.isReady()) {
            inport4_.bindColorTexture(GL_TEXTURE0);
            inport4_.getColorTexture()->enable();
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            glTranslatef(0.5f, -0.5f, 0.0f);
            glScalef(0.5f, 0.5f, 1.0f);

            glDepthFunc(GL_ALWAYS);
            renderQuad();
            glDepthFunc(GL_LESS);

            glLoadIdentity();
            inport4_.getColorTexture()->disable();
        }


        glActiveTexture(GL_TEXTURE0);

        if(showGrid_.get()) {
            glDepthFunc(GL_ALWAYS);
            glColor4f(gridColor_.get().r, gridColor_.get().g, gridColor_.get().b, gridColor_.get().a);
            glBegin(GL_LINES);
            glVertex2f(-1.0f, 0.0f);
            glVertex2f(1.0f, 0.0f);

            glVertex2f(0.0f, 1.0f);
            glVertex2f(0.0f, -1.0f);
            glEnd();
            glDepthFunc(GL_LESS);
        }

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        LGL_ERROR;
    }
    else {
        //maximized:
        switch(maximized_.get()) {
            case 1: if(!inport1_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport1_.bindColorTexture(GL_TEXTURE0);
                    inport1_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport1_.getColorTexture()->disable();
                    break;
            case 2: if(!inport2_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport2_.bindColorTexture(GL_TEXTURE0);
                    inport2_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport2_.getColorTexture()->disable();
                    break;
            case 3: if(!inport3_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport3_.bindColorTexture(GL_TEXTURE0);
                    inport3_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport3_.getColorTexture()->disable();
                    break;
            case 4: if(!inport4_.isReady())
                        return;

                    outport_.activateTarget();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    inport4_.bindColorTexture(GL_TEXTURE0);
                    inport4_.getColorTexture()->enable();
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

                    glDepthFunc(GL_ALWAYS);
                    renderQuad();
                    glDepthFunc(GL_LESS);

                    glLoadIdentity();
                    inport4_.getColorTexture()->disable();
                    break;
        }
    }
}

void QuadView::initialize() throw (VoreenException) {
    RenderProcessor::initialize();
}

void QuadView::portResized(RenderPort* /*p*/, tgt::ivec2 newsize) {
    outport_.resize(newsize);

    updateSizes();
    invalidate();
}

void QuadView::updateSizes() {
    if(maximized_.get() == 0) {
        tgt::ivec2 subsize = outport_.getSize() / 2;
        inport1_.resize(subsize);
        inport2_.resize(subsize);
        inport3_.resize(subsize);
        inport4_.resize(subsize);
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.resize(outport_.getSize());
                    break;
            case 2: inport2_.resize(outport_.getSize());
                    break;
            case 3: inport3_.resize(outport_.getSize());
                    break;
            case 4: inport4_.resize(outport_.getSize());
                    break;
            default:;
        }
    }
}

void QuadView::sizeOriginChanged(RenderPort* /*p*/) {
}

bool QuadView::testSizeOrigin(const RenderPort* p, void* so) const {
    if(p->getSizeOrigin() == so)
        return true;

    if(!so)
        return true;

    if(!p->getSizeOrigin())
        return true;

    return false;
}

void QuadView::invalidate(InvalidationLevel inv) {
    RenderProcessor::invalidate(inv);
}

void QuadView::onEvent(tgt::Event* e) {
    tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e);

    if (!me) {
        RenderProcessor::onEvent(e);
        return;
    }

    if ((me->action() == tgt::MouseEvent::DOUBLECLICK) && maximizeOnDoubleClick_.get()) {
        if(maximized_.get() == 0) {
            if (me->y() < (me->viewport().y / 2)) {
                if (me->x() < (me->viewport().x / 2)) {
                    maximized_.set(1);
                }
                else {
                    maximized_.set(2);
                }
            }
            else {
                if (me->x() < (me->viewport().x / 2)) {
                    maximized_.set(3);
                }
                else {
                    maximized_.set(4);
                }
            }
        }
        else {
            maximized_.set(0);
        }
        updateSizes();
        me->accept();
        return;
    }

    if(maximized_.get() == 0) {
        if (me->y() < (me->viewport().y / 2)) {
            if (me->x() < (me->viewport().x / 2)) {
                tgt::MouseEvent newme(me->x(), me->y(), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                inport1_.distributeEvent(&newme);
            }
            else {
                tgt::MouseEvent newme(me->x() - (me->viewport().x / 2), me->y(), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                inport2_.distributeEvent(&newme);
            }
        }
        else {
            if (me->x() < (me->viewport().x / 2)) {
                tgt::MouseEvent newme(me->x(), me->y() - (me->viewport().y / 2), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                inport3_.distributeEvent(&newme);
            }
            else {
                tgt::MouseEvent newme(me->x() - (me->viewport().x / 2), me->y() - (me->viewport().y / 2), me->action(), me->modifiers(), me->button(), me->viewport() / 2);
                inport4_.distributeEvent(&newme);
            }
        }
    }
    else {
        switch(maximized_.get()) {
            case 1: inport1_.distributeEvent(me);
                    break;
            case 2: inport2_.distributeEvent(me);
                    break;
            case 3: inport3_.distributeEvent(me);
                    break;
            case 4: inport4_.distributeEvent(me);
                    break;
            default:;
        }
    }
}

} // namespace voreen
