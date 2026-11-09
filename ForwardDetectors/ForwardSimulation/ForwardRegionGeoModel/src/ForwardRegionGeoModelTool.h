/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ForwardRegionGeoModel_ForwardRegionGeoModelTool_H
#define ForwardRegionGeoModel_ForwardRegionGeoModelTool_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "ForwardRegionGeoModelFactory.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/SystemOfUnits.h"

class ForwardRegionGeoModelTool final : public GeoModelTool {

public:

    // Standard Constructor
    ForwardRegionGeoModelTool( const std::string& type, const std::string& name, const IInterface* parent );

    // Standard Destructor
    virtual ~ForwardRegionGeoModelTool() override final;

    virtual StatusCode create() override final;
private:
    // Copy property values into m_Config
    void fillConfig();

    // Properties copied into m_Config by fillConfig()
    Gaudi::Property<double> m_tcl4JawDistB1I{this, "TCL4JawDistB1I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl4JawDistB2I{this, "TCL4JawDistB2I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl5JawDistB1I{this, "TCL5JawDistB1I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl5JawDistB2I{this, "TCL5JawDistB2I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl6JawDistB1I{this, "TCL6JawDistB1I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl6JawDistB2I{this, "TCL6JawDistB2I", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl4JawDistB1O{this, "TCL4JawDistB1O", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl4JawDistB2O{this, "TCL4JawDistB2O", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl5JawDistB1O{this, "TCL5JawDistB1O", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl5JawDistB2O{this, "TCL5JawDistB2O", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl6JawDistB1O{this, "TCL6JawDistB1O", 57*Gaudi::Units::mm};
    Gaudi::Property<double> m_tcl6JawDistB2O{this, "TCL6JawDistB2O", 57*Gaudi::Units::mm};
    Gaudi::Property<bool> m_vp1Compatibility{this, "vp1Compatibility", false};
    Gaudi::Property<bool> m_buildTCL4{this, "buildTCL4", false};
    Gaudi::Property<bool> m_buildTCL6{this, "buildTCL6", false};
    Gaudi::Property<bool> m_alfaInNewPosition{this, "ALFAInNewPosition", false};
    Gaudi::Property<double> m_newPosB7L1{this, "newPosB7L1", 245656.77*Gaudi::Units::mm};
    Gaudi::Property<double> m_newPosB7R1{this, "newPosB7R1", -245656.11*Gaudi::Units::mm};
    Gaudi::Property<double> m_posAFPL1{this, "posAFPL1", 204500*Gaudi::Units::mm};
    Gaudi::Property<double> m_posAFPR1{this, "posAFPR1", -204500*Gaudi::Units::mm};
    Gaudi::Property<double> m_posAFPL2{this, "posAFPL2", 212675*Gaudi::Units::mm};
    Gaudi::Property<double> m_posAFPR2{this, "posAFPR2", -212675*Gaudi::Units::mm};
    Gaudi::Property<double> m_posZDC1{this, "posZDC1", 141580*Gaudi::Units::mm};
    Gaudi::Property<double> m_posZDC2{this, "posZDC2", -141580*Gaudi::Units::mm};

    // property configuration
    FWD_CONFIGURATION m_Config{};
};


#endif // ForwardRegionGeoModel_ForwardRegionGeoModelTool_H
