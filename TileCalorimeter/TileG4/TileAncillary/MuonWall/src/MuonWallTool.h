/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONWALL_MUONWALLTOOL_H
#define MUONWALL_MUONWALLTOOL_H

// Base class header
#include "G4AtlasTools/DetectorGeometryBase.h"

// STL library
#include <string>

/** @class MuonWallTool MuonWallTool.h "MuonWall/MuonWallTool.h"
 *
 *  Tool for building the MuonWall detector.
 */

class MuonWallTool final : public DetectorGeometryBase {
public:
    // Basic constructor and destructor
    MuonWallTool(const std::string& type, const std::string& name, const IInterface *parent);
    ~MuonWallTool() = default;

    /** Override DetectorGeometryBase::BuildGeometry method */
    virtual void BuildGeometry() override final;

private:
  Gaudi::Property<double> m_zLength{this,  "ZLength", 0.};
  Gaudi::Property<double> m_yLength{this,  "YLength", 0.};
  Gaudi::Property<double> m_xLength{this,  "XLength", 0.};
  Gaudi::Property<bool>   m_backWall{this, "backWall", true};
  Gaudi::Property<bool>   m_sideWall{this, "sideWall", false};
};

#endif //MUONWALL_MUONWALLTOOL_H
