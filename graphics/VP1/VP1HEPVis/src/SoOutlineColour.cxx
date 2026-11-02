/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1HEPVis/nodes/SoOutlineColour.h"
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/misc/SoState.h>

namespace {
class OutlineColourElement : public SoElement {
  SO_ELEMENT_HEADER(OutlineColourElement);
public:
  static void initClass() { SO_ELEMENT_INIT_CLASS(OutlineColourElement, SoElement); }
  void init(SoState*) override { m_defined = false; m_enabled = false; m_value.setValue(0, 0, 0); }
  SbBool matches(const SoElement* other) const override {
    const auto* element = static_cast<const OutlineColourElement*>(other);
    return m_defined == element->m_defined && m_enabled == element->m_enabled && m_value == element->m_value;
  }
  SoElement* copyMatchInfo() const override {
    auto* element = new OutlineColourElement;
    element->m_defined = m_defined;
    element->m_enabled = m_enabled;
    element->m_value = m_value;
    return element;
  }
  static void set(SoState* state, const SbColor& colour, bool enabled) {
    auto* element = static_cast<OutlineColourElement*>(getElement(state, getClassStackIndex()));
    element->m_defined = true;
    element->m_enabled = enabled;
    element->m_value = colour;
  }
  static bool get(SoState* state, SbColor& colour) {
    if (getClassStackIndex() < 0 || !state || !state->isElementEnabled(getClassStackIndex()))
      return false;
    const auto* element = static_cast<const OutlineColourElement*>(getConstElement(state, getClassStackIndex()));
    colour = element->m_value;
    return element->m_defined && element->m_enabled;
  }
protected:
  ~OutlineColourElement() override = default;
private:
  bool m_defined = false;
  bool m_enabled = false;
  SbColor m_value;
};
SO_ELEMENT_SOURCE(OutlineColourElement);
}

SO_NODE_SOURCE(SoOutlineColour);

void SoOutlineColour::initClass()
{
  static const bool initialized = [] {
    OutlineColourElement::initClass();
    SO_NODE_INIT_CLASS(SoOutlineColour, SoNode, "Node");
    SO_ENABLE(SoGLRenderAction, OutlineColourElement);
    return true;
  }();
  (void)initialized;
}

SoOutlineColour::SoOutlineColour()
{
  SO_NODE_CONSTRUCTOR(SoOutlineColour);
  SO_NODE_ADD_FIELD(colour, (1.0f, 1.0f, 1.0f));
  SO_NODE_ADD_FIELD(enabled, (TRUE));
}

void SoOutlineColour::GLRender(SoGLRenderAction* action)
{
  OutlineColourElement::set(action->getState(), colour.getValue(), enabled.getValue());
}

bool SoOutlineColour::get(SoState* state, SbColor& colour)
{
  return OutlineColourElement::get(state, colour);
}
