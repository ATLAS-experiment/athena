/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORTOOL_H
#define COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORTOOL_H

// Base class header
#include "G4AtlasTools/DetectorGeometryBase.h"

// STL library
#include <string>

/** @class CombinedScintillatorTool CombinedScintillatorTool.h "Combinedscintillator/CombinedScintillatorTool.h"
 *
 *  Tool for building the Combinedscintillator detector.
 */

class CombinedScintillatorTool final : public DetectorGeometryBase {
  public:
    // Basic constructor and destructor
    CombinedScintillatorTool(const std::string& type, const std::string& name, const IInterface *parent);
    ~CombinedScintillatorTool() = default;

    /** virtual methods being implemented here */
    virtual void BuildGeometry() override final;

  private:
    Gaudi::Property<double> m_rMin{this, "RMin", 0.0};
    Gaudi::Property<double> m_rMax{this, "RMax", 0.0};
    Gaudi::Property<double> m_dzSci{this, "DZSci", 0.0};
    Gaudi::Property<double> m_phiPos{this, "PhiPos", 0.0};
    Gaudi::Property<double> m_phiNeg{this, "PhiNeg", 0.0};
};

#endif //COMBINEDSCINTILLATOR_COMBINEDSCINTILLATORTOOL_H
