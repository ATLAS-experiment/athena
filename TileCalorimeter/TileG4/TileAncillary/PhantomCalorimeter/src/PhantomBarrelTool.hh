/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHANTOMCALORIMETER_PHANTOMBARRELTOOL_H
#define PHANTOMCALORIMETER_PHANTOMBARRELTOOL_H

// Base class header
#include "G4AtlasTools/DetectorGeometryBase.h"

// STL library
#include <string>

/** @class PhantomBarrelTool PhantomBarrelTool.h "PhantomCalorimeter/PhantomBarrelTool.h"
 *
 *  Tool for building the PhantomBarrel detector.
 */

class PhantomBarrelTool final : public DetectorGeometryBase {
  public:
    // Basic constructor and destructor
    PhantomBarrelTool(const std::string& type, const std::string& name, const IInterface *parent);
    ~PhantomBarrelTool() = default;

    /** virtual methods being implemented here */
    virtual void BuildGeometry() override final;

  private:
  Gaudi::Property<double> m_zLength {this, "ZLength", 0.0};
  Gaudi::Property<double> m_rMin {this, "RMin", 0.0};
  Gaudi::Property<double> m_rMax {this, "RMax", 0.0};
};

#endif //PHANTOMCALORIMETER_PHANTOMBARRELTOOL_H
