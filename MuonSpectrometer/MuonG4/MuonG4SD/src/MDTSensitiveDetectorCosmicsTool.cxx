/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSensitiveDetectorCosmicsTool.h"
#include "MDTSensitiveDetectorCosmics.h"

#include "HitManagement/HitCollectionMap.h"
#include "MuonSimEvent/MDTSimHitCollection.h"
#include <TString.h> // for Form

MDTSensitiveDetectorCosmicsTool::MDTSensitiveDetectorCosmicsTool(const std::string& type, const std::string& name, const IInterface* parent)
  : SensitiveDetectorBase( type , name , parent )
{
}

StatusCode MDTSensitiveDetectorCosmicsTool::SetupEvent(HitCollectionMap& hitCollections)
{
  hitCollections.Emplace<MDTSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

StatusCode MDTSensitiveDetectorCosmicsTool::Gather(HitCollectionMap& hitCollections)
{
  hitCollections.Record<MDTSimHitCollection>(m_outputCollectionNames[0]);
  return StatusCode::SUCCESS;
}

G4VSensitiveDetector* MDTSensitiveDetectorCosmicsTool::makeSD() const
{
  ATH_MSG_DEBUG( "Initializing SD" );
  StatusCode sc = m_idHelperSvc.retrieve();
  if (sc.isFailure()) throw std::runtime_error(Form("File: %s, Line: %d\nMDTSensitiveDetectorCosmicsTool::makeSD() - could not retrieve MuonIdHelperSvc", __FILE__, __LINE__));
  return new MDTSensitiveDetectorCosmics(name(), m_outputCollectionNames[0], m_idHelperSvc->mdtIdHelper().tubeMax());
}
