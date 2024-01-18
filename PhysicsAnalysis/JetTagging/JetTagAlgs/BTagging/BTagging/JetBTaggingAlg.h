/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGGING_JETBTAGGINGALG_HH
#define BTAGGING_JETBTAGGINGALG_HH
///////////////////////////////////////////
///
/// \class JetBTaggingAlg
/// Algorithm to run and add btagging information.
////////////////////////////////////////////

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "BTagging/IBTagTool.h"
#include "BTagging/IBTagLightSecVertexing.h"

// For magneticfield
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

namespace Analysis{
class IJetFitterVariablesFactory;

class  JetBTaggingAlg: 
  public AthReentrantAlgorithm
   { 
  public:
  
    /** Constructors and destructors */
    JetBTaggingAlg(const std::string& name, ISvcLocator *pSvcLocator);
    virtual ~JetBTaggingAlg();
    
    /** Main routines specific to an ATHENA algorithm */
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
  
    SG::ReadHandleKey<xAOD::JetContainer > m_JetCollectionName {this, "JetCollectionName", "", "Input jet container"};

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_IncomingTracks{ this, "IncomingTracks", "", "Element Link vector from jet to particle container"};
    SG::WriteDecorHandleKey<xAOD::BTaggingContainer> m_OutgoingTracks{ this, "OutgoingTracks", "", "Element Link vector from BTagging to track container"};
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_IncomingMuons{ this, "IncomingMuons", "", "Element Link vector from jet to particle container"};
    SG::WriteDecorHandleKey<xAOD::BTaggingContainer> m_OutgoingMuons{ this, "OutgoingMuons", "", "Element Link vector from BTagging to muon container"};
    
    // Read handle for conditions object to get the field cache
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj", "Name of the Magnetic Field conditions object key"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetBTaggingLinkName {this, "BTaggingLinkName", "", "Element link from jet to BTagging container"};
    SG::WriteHandleKey<xAOD::BTaggingContainer> m_BTaggingCollectionName {this, "BTaggingCollectionName", "", "Output BTagging container"};
    SG::WriteDecorHandleKey<xAOD::BTaggingContainer> m_bTagJetDecorLinkName {this, "JetLinkName", "", "Element Link from BTagging to Jet container"};

    std::string m_JetName;

    bool m_DoMuons = false;

    ToolHandle< IBTagTool > m_bTagTool;
    ToolHandle< IBTagLightSecVertexing > m_bTagSecVtxTool;

};

}

#endif
