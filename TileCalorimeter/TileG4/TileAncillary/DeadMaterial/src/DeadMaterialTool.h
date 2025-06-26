/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DEADMATERIAL_DEADMATERIALTOOL_H
#define DEADMATERIAL_DEADMATERIALTOOL_H

// Base class header
#include "G4AtlasTools/DetectorGeometryBase.h"

// STL library
#include <string>

/** @class DeadMaterialTool DeadMaterialTool.h "DeadMaterial/DeadMaterialTool.h"
*
*  Tool for building the DeadMaterial detector.
*/

class DeadMaterialTool final : public DetectorGeometryBase
{
public:
  // Basic constructor and destructor
  DeadMaterialTool(const std::string& type, const std::string& name, const IInterface *parent);
  ~DeadMaterialTool() = default;

  /** virtual methods being implemented here */
  virtual void BuildGeometry() override final;

private:
  Gaudi::Property<double> m_zLength{this, "ZLength", 0.};
  Gaudi::Property<double> m_yLength{this, "YLength", 0.};
  Gaudi::Property<double> m_xLength{this, "XLength", 0.};
};

#endif //DEADMATERIAL_DEADMATERIALTOOL_H
