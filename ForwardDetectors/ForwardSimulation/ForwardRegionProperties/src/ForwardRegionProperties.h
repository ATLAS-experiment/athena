/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ForwardRegionProperties_H
#define ForwardRegionProperties_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ForwardRegionGeoModel/IForwardRegionProperties.h"
#include "CLHEP/Geometry/Point3D.h"
#include "Gaudi/Property.h"

#include <string>
#include <vector>

class ForwardRegionProperties : public extends<AthAlgTool, IForwardRegionProperties>
{
public:
    ForwardRegionProperties( const std::string& type, const std::string& name, const IInterface* parent );

    // Standard Destructor
    virtual ~ForwardRegionProperties();

    virtual StatusCode initialize() override;

    virtual PFWDMg_CONFIGURATION getConf() override final;

    void getMagTransforms(const std::string&, int, HepGeom::Point3D<double>&, HepGeom::Point3D<double>&, double&, bool twiss = false) override final;
    void getMagTransforms(const std::string&, int, HepGeom::Point3D<double>&, HepGeom::Point3D<double>&, bool twiss = false) override final;

private:
    // Copy property values into m_Config
    void fillConfig();

    // Properties copied into m_Config by fillConfig()
    Gaudi::Property<std::string> m_twissFileB1{this, "twissFileB1", ""};
    Gaudi::Property<std::string> m_twissFileB2{this, "twissFileB2", ""};
    Gaudi::Property<double> m_momentum{this, "momentum", 0};

    Gaudi::Property<std::vector<double>> m_pointQ1Start{this, "pointQ1Start", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ2aStart{this, "pointQ2aStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ2bStart{this, "pointQ2bStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ3Start{this, "pointQ3Start", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ4Start{this, "pointQ4Start", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ5Start{this, "pointQ5Start", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ6Start{this, "pointQ6Start", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ7aStart{this, "pointQ7aStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ7bStart{this, "pointQ7bStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1aStart{this, "pointD1aStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1bStart{this, "pointD1bStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1cStart{this, "pointD1cStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1dStart{this, "pointD1dStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1eStart{this, "pointD1eStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1fStart{this, "pointD1fStart", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD2Start{this, "pointD2Start", std::vector<double>(6, 0.0)};

    Gaudi::Property<std::vector<double>> m_pointQ1End{this, "pointQ1End", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ2aEnd{this, "pointQ2aEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ2bEnd{this, "pointQ2bEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ3End{this, "pointQ3End", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ4End{this, "pointQ4End", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ5End{this, "pointQ5End", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ6End{this, "pointQ6End", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ7aEnd{this, "pointQ7aEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointQ7bEnd{this, "pointQ7bEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1aEnd{this, "pointD1aEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1bEnd{this, "pointD1bEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1cEnd{this, "pointD1cEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1dEnd{this, "pointD1dEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1eEnd{this, "pointD1eEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD1fEnd{this, "pointD1fEnd", std::vector<double>(6, 0.0)};
    Gaudi::Property<std::vector<double>> m_pointD2End{this, "pointD2End", std::vector<double>(6, 0.0)};

    Gaudi::Property<std::vector<double>> m_fQ1RotZ{this, "fQ1RotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ2aRotZ{this, "fQ2aRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ2bRotZ{this, "fQ2bRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ3RotZ{this, "fQ3RotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ4RotZ{this, "fQ4RotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ5RotZ{this, "fQ5RotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ6RotZ{this, "fQ6RotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ7aRotZ{this, "fQ7aRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fQ7bRotZ{this, "fQ7bRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1aRotZ{this, "fD1aRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1bRotZ{this, "fD1bRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1cRotZ{this, "fD1cRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1dRotZ{this, "fD1dRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1eRotZ{this, "fD1eRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD1fRotZ{this, "fD1fRotZ", std::vector<double>(2, 0.0)};
    Gaudi::Property<std::vector<double>> m_fD2RotZ{this, "fD2RotZ", std::vector<double>(2, 0.0)};

    Gaudi::Property<bool> m_useFLUKAMapsForInnerTriplet{this, "bUseFLUKAMapsForInnerTriplet", false};

    // property configuration
    FWDMg_CONFIGURATION m_Config;
};

#endif // ForwardRegionProperties_H
