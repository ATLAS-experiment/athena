///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METTauAssociator.h 
// Header file for class METTauAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 
#ifndef METRECONSTRUCTION_METTAUASSOCIATOR_H
#define METRECONSTRUCTION_METTAUASSOCIATOR_H 1

// METReconstruction includes
#include "METReconstruction/METAssociator.h"
#include "StoreGate/ReadDecorHandle.h"


namespace met{
  class METTauAssociator final
    : public METAssociator
  { 
    // This macro defines the constructor with the interface declaration
    ASG_TOOL_CLASS(METTauAssociator, IMETAssocToolBase)


    /////////////////////////////////////////////////////////////////// 
    // Public methods: 
    /////////////////////////////////////////////////////////////////// 
    public: 

    // Constructor with name
    METTauAssociator(const std::string& name);
    ~METTauAssociator() = default;

    // AsgTool Hooks
    virtual StatusCode initialize() override;

    /////////////////////////////////////////////////////////////////// 
    // Private data: 
    /////////////////////////////////////////////////////////////////// 
    protected: 

    StatusCode executeTool(xAOD::MissingETContainer* metCont, xAOD::MissingETAssociationMap* metMap, const EventContext& ctx) const final;
    StatusCode extractTopoClusters(const xAOD::IParticle* obj,
                                   std::vector<const xAOD::IParticle*>& tclist,
                                   const met::METAssociator::ConstitHolder& constits, const EventContext& ctx) const final;

    StatusCode extractPFO(const xAOD::IParticle* obj,
                          std::vector<const xAOD::IParticle*>& pfolist,
                          const met::METAssociator::ConstitHolder& constits,
                          std::map<const xAOD::IParticle*,MissingETBase::Types::constvec_t> &momenta, const EventContext& ctx) const final;

    StatusCode extractFE(const xAOD::IParticle* obj,
                         std::vector<const xAOD::IParticle*>& felist,
                         const met::METAssociator::ConstitHolder& constits,
                         std::map<const xAOD::IParticle*,MissingETBase::Types::constvec_t> &momenta, const EventContext& ctx) const final;   

   StatusCode extractFEsFromLinks(const xAOD::TauJet* tau,
    				    std::vector<const xAOD::IParticle*>& felist,
				    const met::METAssociator::ConstitHolder& constits, const EventContext& ctx) const;

    StatusCode extractFEs(const xAOD::TauJet* tau, 
				 std::vector<const xAOD::IParticle*>& felist,
				 const met::METAssociator::ConstitHolder& constits) const;

    StatusCode extractTracks(const xAOD::IParticle* obj,
                             std::vector<const xAOD::IParticle*>& constlist,
                             const met::METAssociator::ConstitHolder& constits) const final;

    private:
 
    /// Default constructor: 
    METTauAssociator();

    SG::ReadHandleKey<xAOD::TauJetContainer> m_tauContKey{this, "InputCollection", "TauJets", "taus input key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_neutralFEReadDecorKey{this,"NeutralFEReadDecorKey", m_tauContKey, "neutralGlobalFELinks", "Neutral FlowElement links key"};
    SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_chargedFEReadDecorKey{this,"ChargedFEReadDecorKey", m_tauContKey, "chargedGlobalFELinks", "Charged FlowElement links key"};
  };

}

#endif //> !METRECONSTRUCTION_METTAUASSOCIATOR_H
