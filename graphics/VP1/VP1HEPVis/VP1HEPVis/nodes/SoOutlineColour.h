/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef VP1_SOOUTLINECOLOUR_H
#define VP1_SOOUTLINECOLOUR_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoSFBool.h>
class SoState;
class SoGLRenderAction;

// Optional, scoped outline colour. Absence of this node retains inherited colour.
class SoOutlineColour : public SoNode {
  SO_NODE_HEADER(SoOutlineColour);
public:
  SoOutlineColour();
  static void initClass();
  SoSFColor colour;
  SoSFBool enabled;
  static bool get(SoState*, SbColor&);
protected:
  ~SoOutlineColour() override = default;
  void GLRender(SoGLRenderAction*) override;
};

#endif
