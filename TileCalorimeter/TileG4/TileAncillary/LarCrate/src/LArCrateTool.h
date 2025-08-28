/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCRATE_LARCRATETOOL_H
#define LARCRATE_LARCRATETOOL_H

// Base class header
#include "G4AtlasTools/DetectorGeometryBase.h"

// STL library
#include <string>

/** @class LArCrateTool LArCrateTool.h "LarCrate/LArCrateTool.h"
*
*  Tool for building the LArCrate detector.
*/

class LArCrateTool final : public DetectorGeometryBase
{
public:
  // Basic constructor and destructor
  LArCrateTool(const std::string& type, const std::string& name, const IInterface *parent);
  ~LArCrateTool() = default;

  /** virtual methods being implemented here */
  virtual void BuildGeometry() override final;

private:
  Gaudi::Property<double> m_zLength{this, "ZLength", 0.};
  Gaudi::Property<double> m_yLength{this, "YLength", 0.};
  Gaudi::Property<double> m_xLength{this, "XLength", 0.};
};

#endif //LARCRATE_LARCRATETOOL_H
