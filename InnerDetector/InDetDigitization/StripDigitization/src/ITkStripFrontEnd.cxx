/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripFrontEnd.h"

#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"

// Random number
#include "CLHEP/Random/RandomEngine.h"

using namespace InDetDD;

// constructor
ITkStripFrontEnd::ITkStripFrontEnd(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type, name, parent) {
}

// ----------------------------------------------------------------------
// Initialize
// ----------------------------------------------------------------------
StatusCode ITkStripFrontEnd::initialize() {
  ATH_MSG_DEBUG("ITkStripFrontEnd::initialize()");
  // Get SCT helper
  ATH_CHECK(detStore()->retrieve(m_ITkStripId, "SCT_ID"));
  // Get SCT detector manager
  ATH_CHECK(detStore()->retrieve(m_ITkStripMgr,m_detMgrName));
  //(note : we need an amplifier here, can probably be hard coded but separate class, not an Athena component)
  return StatusCode::SUCCESS;
}

// ----------------------------------------------------------------------
// process the collection of pre digits this will need to go through
// all single-strip pre-digits calculate the amplifier response add noise
// (this could be moved elsewhere later) apply threshold do clustering
// ----------------------------------------------------------------------
void 
ITkStripFrontEnd::process(SiChargedDiodeCollection& collection, CLHEP::HepRandomEngine * /*rndmEngine*/) const {
  // get ITk module side design
  [[maybe_unused]] const SCT_ModuleSideDesign *p_design = static_cast<const SCT_ModuleSideDesign*>(&(collection.design()));
}
