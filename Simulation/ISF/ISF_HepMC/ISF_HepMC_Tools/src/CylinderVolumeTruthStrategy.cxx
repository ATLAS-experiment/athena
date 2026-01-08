/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "CylinderVolumeTruthStrategy.h"

// ISF includes
#include "ISF_Event/ITruthIncident.h"

// HepMC includes
#include "AtlasHepMC/SimpleVector.h"

/** Constructor **/
ISF::CylinderVolumeTruthStrategy::CylinderVolumeTruthStrategy(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

// Athena algtool's Hooks
StatusCode  ISF::CylinderVolumeTruthStrategy::initialize()
{
  ATH_MSG_VERBOSE("Initializing ...");

  for(auto region : m_regionListProperty.value()) {
    if(region < AtlasDetDescr::fFirstAtlasRegion || region >= AtlasDetDescr::fNumAtlasRegions) {
      ATH_MSG_ERROR("Unknown Region (" << region << ") specified. Please check your configuration.");
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_VERBOSE("Initialize successful");
  return StatusCode::SUCCESS;
}

bool ISF::CylinderVolumeTruthStrategy::pass( ITruthIncident& ti) const
{
  // the current truth incident radius
  auto t_pos=ti.position();
  double r = std::sqrt(t_pos.x()*t_pos.x()+t_pos.y()*t_pos.y()+t_pos.z()*t_pos.z());

  // is the current radius on the surface?
  bool onSurf = (r>m_ri) && (r<m_ro);

  return onSurf;
}

bool ISF::CylinderVolumeTruthStrategy::appliesToRegion(unsigned short geoID) const
{
  return std::find( m_regionListProperty.begin(),
                    m_regionListProperty.end(),
                    geoID ) != m_regionListProperty.end();
}
