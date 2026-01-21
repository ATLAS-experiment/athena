/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// class header include
#include "LLPTruthStrategy.h"

// ISF includes
#include "ISF_Event/ITruthIncident.h"
#include "ISF_Event/ISFParticle.h"

#include "TruthUtils/HepMCHelpers.h"

/** Constructor **/
ISF::LLPTruthStrategy::LLPTruthStrategy(const std::string& t, const std::string& n, const IInterface* p) :
  base_class(t,n,p)
{
}

// Athena algtool's Hooks
StatusCode  ISF::LLPTruthStrategy::initialize()
{
  ATH_MSG_VERBOSE("Initializing ...");
  for (auto region : m_regionListProperty.value()) {
    if (region < AtlasDetDescr::fFirstAtlasRegion || region >= AtlasDetDescr::fNumAtlasRegions) {
      ATH_MSG_ERROR("Unknown Region (" << region << ") specified. Please check your configuration.");
      return StatusCode::FAILURE;
    }
  }
  ATH_MSG_VERBOSE("Initialization successful.");
  return StatusCode::SUCCESS;
}


bool ISF::LLPTruthStrategy::pass( ITruthIncident& ti) const
{
  const int processCode = ti.physicsProcessCode(); // == G4ProcessSubType
  if( (processCode>m_passProcessCodeRangeLow && processCode<m_passProcessCodeRangeHigh) ){ // All kinds of decay processes are included here...
    // Check if this is a sparticle - if not, return
    if ( !MC::isSUSY(ti.parentPdgCode()) ) {
      // not passed!
      return false;
    }
    ATH_MSG_VERBOSE("ACHLLP: saved a truth vertex for pdg "<< ti.parentPdgCode());
    // passed!
    return true;
  }
  const int processCategory = ti.physicsProcessCategory(); // == G4ProcessType
  if ( processCategory==m_passProcessCategory ){//save all interactions for user-defined processes, like rhadron interactions
    ATH_MSG_VERBOSE("ACHLLP: saved a truth interaction fUserDefined for pdg " << ti.parentPdgCode());
    // passed!
    return true;
  }
  // not passed!
  return false;
}


bool ISF::LLPTruthStrategy::appliesToRegion(unsigned short geoID) const
{
  return std::find( m_regionListProperty.begin(),
                    m_regionListProperty.end(),
                    geoID ) != m_regionListProperty.end();
}
