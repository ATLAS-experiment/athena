/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_TRIGPARTICLEEVENT
#define TRIGSTORAGEDEF_TRIGPARTICLEEVENT
#include "TrigStorageDefinitions/EDM_TypeInformation.h"

namespace Analysis{
  class TauDetails;
  class TauDetailsContainer;
  class TauJet;
  class TauJetContainer;  
}

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"

#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetTrigAuxContainer.h"

#include "xAODTrigEgamma/TrigElectron.h"
#include "xAODTrigEgamma/TrigElectronContainer.h"
#include "xAODTrigEgamma/TrigElectronAuxContainer.h"


#include "xAODTrigEgamma/TrigPhoton.h"
#include "xAODTrigEgamma/TrigPhotonContainer.h"
#include "xAODTrigEgamma/TrigPhotonAuxContainer.h"
#include "xAODTrigEgamma/ElectronTrigAuxContainer.h"
#include "xAODTrigEgamma/PhotonTrigAuxContainer.h"


#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTaggingTrigAuxContainer.h"

#include "xAODBTagging/BTagVertex.h"
#include "xAODBTagging/BTagVertexContainer.h"
#include "xAODBTagging/BTagVertexAuxContainer.h"


HLT_BEGIN_TYPE_REGISTRATION
  HLT_REGISTER_TYPE(xAOD::Electron, xAOD::ElectronContainer, xAOD::ElectronContainer, xAOD::ElectronTrigAuxContainer)
  HLT_REGISTER_TYPE(xAOD::Photon, xAOD::PhotonContainer, xAOD::PhotonContainer, xAOD::PhotonTrigAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TauJet, xAOD::TauJetContainer, xAOD::TauJetContainer, xAOD::TauJetAuxContainer)  
  HLT_REGISTER_TYPE(xAOD::TauTrack, xAOD::TauTrackContainer, xAOD::TauTrackContainer, xAOD::TauTrackAuxContainer)
  HLT_REGISTER_TYPE(xAOD::Jet, xAOD::JetContainer, xAOD::JetContainer, xAOD::JetTrigAuxContainer)

  HLT_REGISTER_TYPE(xAOD::TrigElectron, xAOD::TrigElectronContainer, xAOD::TrigElectronContainer, xAOD::TrigElectronAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigPhoton, xAOD::TrigPhotonContainer, xAOD::TrigPhotonContainer, xAOD::TrigPhotonAuxContainer)
  HLT_REGISTER_TYPE(xAOD::BTagging, xAOD::BTaggingContainer, xAOD::BTaggingContainer, xAOD::BTaggingTrigAuxContainer)
  HLT_REGISTER_TYPE(xAOD::BTagVertex, xAOD::BTagVertexContainer, xAOD::BTagVertexContainer, xAOD::BTagVertexAuxContainer)
  
HLT_END_TYPE_REGISTRATION(TrigParticle)

#endif
