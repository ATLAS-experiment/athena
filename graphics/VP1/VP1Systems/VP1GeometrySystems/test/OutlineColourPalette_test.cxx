/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1GeometrySystems/VisAttributes.h"
#include <Inventor/SoDB.h>
#include <Inventor/nodes/SoMaterial.h>
#include <iostream>

int main()
{
  SoDB::init();
  DetVisAttributes palette;
  const auto pink = palette.outlineColour("TGC");
  if (!pink || *pink != SbColor(1.0f, 0.6f, 0.85f)) {
    std::cerr << "TGC must define the light-pink outline colour\n";
    return 1;
  }
  if (palette.get("TGC")->diffuseColor[0] != SbColor(0.41f, 0.0f, 0.26f)) {
    std::cerr << "TGC faces must retain their dark-purple colour\n";
    return 1;
  }
  for (const auto* name : {"Muon", "BarrelInner", "BarrelMiddle", "BarrelOuter", "EndcapMdt", "CSC", "Unknown"}) {
    if (palette.outlineColour(name)) {
      std::cerr << name << " must retain inherited outline colour\n";
      return 1;
    }
  }
  if (palette.get("BarrelInner")->diffuseColor[0] != SbColor(0, 0.6667f, 1.0f)) {
    std::cerr << "MDT faces must retain their bright-blue colour\n";
    return 1;
  }
  // Old material-only settings must leave optional palette defaults available.
  const auto state = palette.getState(false);
  palette.applyState(state);
  if (palette.outlineColour("TGC") != pink || palette.outlineColour("Muon"))
    return 1;
  return 0;
}
