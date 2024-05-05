/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 -----------------------------------------
 ***************************************************************************
 this class reads in the electronic mapping from the TBElectronics.map-file to return the Muon-Offline-Identifier to a given channel.
 ***************************************************************************/

#include "IOVDbTestConditions/IOVDbTestAmdbCorrection.h"

IOVDbTestAmdbCorrection::IOVDbTestAmdbCorrection()
{ }


IOVDbTestAmdbCorrection::~IOVDbTestAmdbCorrection()
{ }

void 
IOVDbTestAmdbCorrection::set(const HepGeom::Point3D<double>& trans, const HepGeom::Point3D<double>& rot, const std::string& name)
{
  m_name        = name;
  m_translation = trans;
  m_rotation    = rot;
}


const std::string& IOVDbTestAmdbCorrection::name() const
{
    return (m_name);
}

HepGeom::Point3D<double> IOVDbTestAmdbCorrection::getTranslation() const 
{
  return m_translation;
}

HepGeom::Point3D<double> IOVDbTestAmdbCorrection::getRotation() const 
{
  return m_rotation;
}
