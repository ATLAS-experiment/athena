/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ForwardRegionProperties.h"

ForwardRegionProperties::ForwardRegionProperties( const std::string& type, const std::string& name, const IInterface* parent )
: base_class( type, name, parent )
{
    ATH_MSG_INFO("ForwardRegionProperties created");
}

ForwardRegionProperties::~ForwardRegionProperties(){}

StatusCode ForwardRegionProperties::initialize()
{
    fillConfig();
    return StatusCode::SUCCESS;
}

void ForwardRegionProperties::fillConfig()
{
    m_Config.clear();
    m_Config.twissFileB1 = m_twissFileB1.value();
    m_Config.twissFileB2 = m_twissFileB2.value();
    m_Config.momentum = m_momentum;

    m_Config.pointQ1Start = m_pointQ1Start.value();
    m_Config.pointQ2aStart = m_pointQ2aStart.value();
    m_Config.pointQ2bStart = m_pointQ2bStart.value();
    m_Config.pointQ3Start = m_pointQ3Start.value();
    m_Config.pointQ4Start = m_pointQ4Start.value();
    m_Config.pointQ5Start = m_pointQ5Start.value();
    m_Config.pointQ6Start = m_pointQ6Start.value();
    m_Config.pointQ7aStart = m_pointQ7aStart.value();
    m_Config.pointQ7bStart = m_pointQ7bStart.value();
    m_Config.pointD1aStart = m_pointD1aStart.value();
    m_Config.pointD1bStart = m_pointD1bStart.value();
    m_Config.pointD1cStart = m_pointD1cStart.value();
    m_Config.pointD1dStart = m_pointD1dStart.value();
    m_Config.pointD1eStart = m_pointD1eStart.value();
    m_Config.pointD1fStart = m_pointD1fStart.value();
    m_Config.pointD2Start = m_pointD2Start.value();

    m_Config.pointQ1End = m_pointQ1End.value();
    m_Config.pointQ2aEnd = m_pointQ2aEnd.value();
    m_Config.pointQ2bEnd = m_pointQ2bEnd.value();
    m_Config.pointQ3End = m_pointQ3End.value();
    m_Config.pointQ4End = m_pointQ4End.value();
    m_Config.pointQ5End = m_pointQ5End.value();
    m_Config.pointQ6End = m_pointQ6End.value();
    m_Config.pointQ7aEnd = m_pointQ7aEnd.value();
    m_Config.pointQ7bEnd = m_pointQ7bEnd.value();
    m_Config.pointD1aEnd = m_pointD1aEnd.value();
    m_Config.pointD1bEnd = m_pointD1bEnd.value();
    m_Config.pointD1cEnd = m_pointD1cEnd.value();
    m_Config.pointD1dEnd = m_pointD1dEnd.value();
    m_Config.pointD1eEnd = m_pointD1eEnd.value();
    m_Config.pointD1fEnd = m_pointD1fEnd.value();
    m_Config.pointD2End = m_pointD2End.value();

    m_Config.fQ1RotZ = m_fQ1RotZ.value();
    m_Config.fQ2aRotZ = m_fQ2aRotZ.value();
    m_Config.fQ2bRotZ = m_fQ2bRotZ.value();
    m_Config.fQ3RotZ = m_fQ3RotZ.value();
    m_Config.fQ4RotZ = m_fQ4RotZ.value();
    m_Config.fQ5RotZ = m_fQ5RotZ.value();
    m_Config.fQ6RotZ = m_fQ6RotZ.value();
    m_Config.fQ7aRotZ = m_fQ7aRotZ.value();
    m_Config.fQ7bRotZ = m_fQ7bRotZ.value();
    m_Config.fD1aRotZ = m_fD1aRotZ.value();
    m_Config.fD1bRotZ = m_fD1bRotZ.value();
    m_Config.fD1cRotZ = m_fD1cRotZ.value();
    m_Config.fD1dRotZ = m_fD1dRotZ.value();
    m_Config.fD1eRotZ = m_fD1eRotZ.value();
    m_Config.fD1fRotZ = m_fD1fRotZ.value();
    m_Config.fD2RotZ = m_fD2RotZ.value();

    m_Config.bUseFLUKAMapsForInnerTriplet = m_useFLUKAMapsForInnerTriplet;
}

PFWDMg_CONFIGURATION ForwardRegionProperties::getConf() {
    return &m_Config;
}

void ForwardRegionProperties::getMagTransforms(const std::string& name, int beam, HepGeom::Point3D<double> & pointMagStart, HepGeom::Point3D<double> & pointMagEnd, double & rotAxis, bool twiss)
{
    if(name.find("MagQ1") != std::string::npos || (twiss && name.find("MQXA.1") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ1Start[(beam-1)*3],m_Config.pointQ1Start[(beam-1)*3+1],m_Config.pointQ1Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ1End[(beam-1)*3],m_Config.pointQ1End[(beam-1)*3+1],m_Config.pointQ1End[(beam-1)*3+2]);
        rotAxis = m_Config.fQ1RotZ[beam-1];
    }
    else if(name.find("MagQ2a") != std::string::npos || (twiss && name.find("MQXB.A2") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ2aStart[(beam-1)*3],m_Config.pointQ2aStart[(beam-1)*3+1],m_Config.pointQ2aStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ2aEnd[(beam-1)*3],m_Config.pointQ2aEnd[(beam-1)*3+1],m_Config.pointQ2aEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fQ2aRotZ[beam-1];
    }
    else if(name.find("MagQ2b") != std::string::npos || (twiss && name.find("MQXB.B2") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ2bStart[(beam-1)*3],m_Config.pointQ2bStart[(beam-1)*3+1],m_Config.pointQ2bStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ2bEnd[(beam-1)*3],m_Config.pointQ2bEnd[(beam-1)*3+1],m_Config.pointQ2bEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fQ2bRotZ[beam-1];
    }
    else if(name.find("MagQ3") != std::string::npos || (twiss && name.find("MQXA.3") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ3Start[(beam-1)*3],m_Config.pointQ3Start[(beam-1)*3+1],m_Config.pointQ3Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ3End[(beam-1)*3],m_Config.pointQ3End[(beam-1)*3+1],m_Config.pointQ3End[(beam-1)*3+2]);
        rotAxis = m_Config.fQ3RotZ[beam-1];
    }
    else if(name.find("MagD1a") != std::string::npos || (twiss && name.find("MBXW.A4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1aStart[(beam-1)*3],m_Config.pointD1aStart[(beam-1)*3+1],m_Config.pointD1aStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1aEnd[(beam-1)*3],m_Config.pointD1aEnd[(beam-1)*3+1],m_Config.pointD1aEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1aRotZ[beam-1];
    }
    else if(name.find("MagD1b") != std::string::npos || (twiss && name.find("MBXW.B4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1bStart[(beam-1)*3],m_Config.pointD1bStart[(beam-1)*3+1],m_Config.pointD1bStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1bEnd[(beam-1)*3],m_Config.pointD1bEnd[(beam-1)*3+1],m_Config.pointD1bEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1bRotZ[beam-1];
    }
    else if(name.find("MagD1c") != std::string::npos || (twiss && name.find("MBXW.C4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1cStart[(beam-1)*3],m_Config.pointD1cStart[(beam-1)*3+1],m_Config.pointD1cStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1cEnd[(beam-1)*3],m_Config.pointD1cEnd[(beam-1)*3+1],m_Config.pointD1cEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1cRotZ[beam-1];
    }
    else if(name.find("MagD1d") != std::string::npos || (twiss && name.find("MBXW.D4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1dStart[(beam-1)*3],m_Config.pointD1dStart[(beam-1)*3+1],m_Config.pointD1dStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1dEnd[(beam-1)*3],m_Config.pointD1dEnd[(beam-1)*3+1],m_Config.pointD1dEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1dRotZ[beam-1];
    }
    else if(name.find("MagD1e") != std::string::npos || (twiss && name.find("MBXW.E4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1eStart[(beam-1)*3],m_Config.pointD1eStart[(beam-1)*3+1],m_Config.pointD1eStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1eEnd[(beam-1)*3],m_Config.pointD1eEnd[(beam-1)*3+1],m_Config.pointD1eEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1eRotZ[beam-1];
    }
    else if(name.find("MagD1f") != std::string::npos || (twiss && name.find("MBXW.F4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD1fStart[(beam-1)*3],m_Config.pointD1fStart[(beam-1)*3+1],m_Config.pointD1fStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD1fEnd[(beam-1)*3],m_Config.pointD1fEnd[(beam-1)*3+1],m_Config.pointD1fEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fD1fRotZ[beam-1];
    }
    else if(name.find("MagD2") != std::string::npos || (twiss && name.find("MBRC.4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointD2Start[(beam-1)*3],m_Config.pointD2Start[(beam-1)*3+1],m_Config.pointD2Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointD2End[(beam-1)*3],m_Config.pointD2End[(beam-1)*3+1],m_Config.pointD2End[(beam-1)*3+2]);
        rotAxis = m_Config.fD2RotZ[beam-1];
    }
    else if(name.find("MagQ4") != std::string::npos || (twiss && name.find("MQY.4") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ4Start[(beam-1)*3],m_Config.pointQ4Start[(beam-1)*3+1],m_Config.pointQ4Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ4End[(beam-1)*3],m_Config.pointQ4End[(beam-1)*3+1],m_Config.pointQ4End[(beam-1)*3+2]);
        rotAxis = m_Config.fQ4RotZ[beam-1];
    }
    else if(name.find("MagQ5") != std::string::npos || (twiss && name.find("MQML.5") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ5Start[(beam-1)*3],m_Config.pointQ5Start[(beam-1)*3+1],m_Config.pointQ5Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ5End[(beam-1)*3],m_Config.pointQ5End[(beam-1)*3+1],m_Config.pointQ5End[(beam-1)*3+2]);
        rotAxis = m_Config.fQ5RotZ[beam-1];
    }
    else if(name.find("MagQ6") != std::string::npos || (twiss && name.find("MQML.6") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ6Start[(beam-1)*3],m_Config.pointQ6Start[(beam-1)*3+1],m_Config.pointQ6Start[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ6End[(beam-1)*3],m_Config.pointQ6End[(beam-1)*3+1],m_Config.pointQ6End[(beam-1)*3+2]);
        rotAxis = m_Config.fQ6RotZ[beam-1];
    }
    else if(name.find("MagQ7a") != std::string::npos || (twiss && name.find("MQM.A7") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ7aStart[(beam-1)*3],m_Config.pointQ7aStart[(beam-1)*3+1],m_Config.pointQ7aStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ7aEnd[(beam-1)*3],m_Config.pointQ7aEnd[(beam-1)*3+1],m_Config.pointQ7aEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fQ7aRotZ[beam-1];
    }
    else if(name.find("MagQ7b") != std::string::npos || (twiss && name.find("MQM.B7") != std::string::npos))
    {
        pointMagStart = HepGeom::Point3D<double>(m_Config.pointQ7bStart[(beam-1)*3],m_Config.pointQ7bStart[(beam-1)*3+1],m_Config.pointQ7bStart[(beam-1)*3+2]);
        pointMagEnd = HepGeom::Point3D<double>(m_Config.pointQ7bEnd[(beam-1)*3],m_Config.pointQ7bEnd[(beam-1)*3+1],m_Config.pointQ7bEnd[(beam-1)*3+2]);
        rotAxis = m_Config.fQ7bRotZ[beam-1];
    }
    else
    {
        pointMagStart = HepGeom::Point3D<double>(0,0,0);
        pointMagEnd = HepGeom::Point3D<double>(0,0,0);
        rotAxis = 0;
    }
}

void ForwardRegionProperties::getMagTransforms(const std::string& name, int beam, HepGeom::Point3D<double> & pointMagStart, HepGeom::Point3D<double> & pointMagEnd, bool twiss)
{
    double dummy;
    getMagTransforms(name, beam, pointMagStart, pointMagEnd, dummy, twiss);
}


