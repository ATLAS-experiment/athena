/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Implementations of concrete input-to-PseudoJet conversions
/// Separated from PseudoJetAlgorithm for readability
///
/// IParticle is the generic type that is normally assumed.
/// Special treatment is needed for two cases:
///   * EMTopo clusters will be converted at the uncalibrated
///     cluster scale -- this is mostly obsolete in offline reco
///     but used by trigger for the moment
///   * ParticleFlowObjects that are charged will be filtered
///     out if they are not matched to the primary vertex

#ifndef PSEUDOJETGETTER_H
#define PSEUDOJETGETTER_H

#include "fastjet/PseudoJet.hh"
#include "xAODBase/IParticle.h"

#ifndef GENERATIONBASE
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODPFlow/PFO.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/Vertex.h"
#include "JetEDM/LabelIndex.h"
#include "JetEDM/VertexIndexedConstituentUserInfo.h"
#endif

namespace PseudoJetGetter {

  struct IParticleRejecter{
    bool null{false};
    bool negativeE{false};
    bool skipNegativeEnergy{false};
  
    IParticleRejecter(bool skip): skipNegativeEnergy(skip){
    }

    bool operator()(const xAOD::IParticle* ip) {
      null = (ip == nullptr);
      if (not null) negativeE = skipNegativeEnergy && ip->e() <= 0.0;
      return (null || negativeE);
    }
  };


  std::vector<fastjet::PseudoJet> 
  IParticlesToPJs(const xAOD::IParticleContainer& ips, bool skipNegativeEnergy) {

    IParticleRejecter rejecter(skipNegativeEnergy);

    std::vector<fastjet::PseudoJet> vpj;
    int index = -1;

    // loop over the input iparticles, select and  convert to pseudojets
    for(const xAOD::IParticle* ip: ips) {
      ++index;
      if(rejecter(ip)){continue;}
    
      // Create a Pseudojet with the momentum of the selected IParticles.
      fastjet::PseudoJet psj(ip->p4());

      // user index is used to identify the xAOD object used for the PseudoJet
      psj.set_user_index(index); 
      vpj.push_back(psj);

    }
    return vpj;
  }

  //**********************************************************************

#ifndef GENERATIONBASE

  struct EMTopoRejecter{
    const xAOD::CaloCluster* cluster{};

    bool operator()(const xAOD::IParticle* ip){
      cluster = dynamic_cast<const xAOD::CaloCluster*>(ip);
      return cluster == nullptr;  // reject if not a cluster
    }
  };

  std::vector<fastjet::PseudoJet> 
  EMToposToPJs(const xAOD::IParticleContainer& ips, bool skipNegativeEnergy) {

    // helper objects for selecting iparticles to be converted to pseudojets
    IParticleRejecter ipRejecter(skipNegativeEnergy);
    EMTopoRejecter emRejecter;
  
    std::vector<fastjet::PseudoJet> vpj;
    int index = -1;

    // loop over iparticles, select and  convert to pseudojets

    for(const xAOD::IParticle* ip: ips) {
      ++index;
      if(ipRejecter(ip) or emRejecter(ip)){continue;}
    
      // Create a Pseudojet with the momentum of the cluster.
      fastjet::PseudoJet 
	psj(emRejecter.cluster->p4(xAOD::CaloCluster::UNCALIBRATED));
    
      // user index is used to identify the xAOD object used for the PseudoJet
      psj.set_user_index(index); 
      vpj.push_back(psj);
    }
    return vpj;
  }

  //**********************************************************************

  struct PFlowRejecter{

    bool skipNegativeEnergy{false};
    bool useChargedPFOs{true};
    bool useNeutralPFOs{true};
    bool useChargedPV{true};
    bool useChargedPUsideband{false};
    bool inputIsUFO{false};

    PFlowRejecter(bool skip, bool useCharged, bool useNeutral, bool chargedPV, bool chargedPUsideband, bool isUFO):
      skipNegativeEnergy(skip),
      useChargedPFOs(useCharged),
      useNeutralPFOs(useNeutral),
      useChargedPV(chargedPV),
      useChargedPUsideband(chargedPUsideband),
      inputIsUFO(isUFO){
    }

    bool operator()(const xAOD::IParticle* ip){
      
      bool reject = false;

      // Reject PJs with invalid energy --- they will lead
      // to crashes in fastjet.  See ATLASRECTS-7137.
      float e = ip->e();
      if (std::isinf(e) || std::isnan(e)) return true;

      if(ip->type() == xAOD::Type::FlowElement){
        const xAOD::FlowElement* pfo = static_cast<const xAOD::FlowElement*>(ip);

	reject = (skipNegativeEnergy && e<FLT_MIN);
  
	if(!inputIsUFO){
	  if( pfo->isCharged() ){
	    if(!useChargedPFOs) reject = true;
	    const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
	    if(useChargedPV && !PVMatchedAcc(*pfo)) reject = true;
	    const static SG::AuxElement::ConstAccessor<char> PUsidebandMatchedAcc("matchedToPUsideband");
	    if (useChargedPUsideband && !PUsidebandMatchedAcc(*pfo)) reject = true;
	  }
	  else{
	    if(!useNeutralPFOs) reject = true;
	  }
	}
	else{
	  if(pfo->signalType() == xAOD::FlowElement::SignalType::Charged && !useChargedPFOs) reject = true;
	  if(pfo->signalType() == xAOD::FlowElement::SignalType::Neutral && !useNeutralPFOs) reject = true;
	}
        return reject;
      }
    
      const xAOD::PFO* pfo = static_cast<const xAOD::PFO*>(ip);
    
      // keep charged PFOs with energy==0 because for MET TST with PFlow, 
      // there may be high pt 
      // charged PFOs that receive a weight of 0 due to being in dense 
      // showers, but need to be present for overlap removal, because they 
      // don't retain these weights when added to the TST      

      reject = (skipNegativeEnergy && e<FLT_MIN);

      if( pfo->isCharged() ) {
	if(!useChargedPFOs) reject = true;
	const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
	if(useChargedPV && !PVMatchedAcc(*pfo)) reject = true;
	const static SG::AuxElement::ConstAccessor<char> PUsidebandMatchedAcc("matchedToPUsideband");
	if (useChargedPUsideband && !PUsidebandMatchedAcc(*pfo)) reject = true;
      }
      else{
	if(!useNeutralPFOs) reject = true;
      }
      return reject;
    }
  };


  std::vector<fastjet::PseudoJet> 
  PFlowsToPJs(const xAOD::IParticleContainer& ips, bool skipNegativeEnergy, bool useChargedPFOs, bool useNeutralPFOs, bool useChargedPV, bool useChargedPUsideband, bool isUFO) {

    PFlowRejecter rejecter(skipNegativeEnergy, useChargedPFOs, useNeutralPFOs, useChargedPV, useChargedPUsideband, isUFO);
    std::vector<fastjet::PseudoJet> vpj;
    int index = -1;

    // loop over the input iparticles, select and  convert to pseudojets

    for(const xAOD::IParticle* ip: ips) {
      ++index;
      if(rejecter(ip)){continue;}
    
      // Create a PSeudojet with the momentum of the selected IParticles.
      fastjet::PseudoJet psj(ip->p4());

      // user index is used to identify the xAOD object used for the PSeudoJet
      psj.set_user_index(index);

      vpj.push_back(psj);
    }
    return vpj;
  }

  std::vector<fastjet::PseudoJet> 
  ByVertexPFlowsToPJs(const xAOD::IParticleContainer& ips, const xAOD::VertexContainer* pvs, bool skipNegativeEnergy, bool useChargedPFOs, bool useNeutralPFOs, bool isUFO) {

    const static SG::AuxElement::Accessor<             unsigned  > copyIndex("ConstituentCopyIndex");     // For neutral PFOs
    const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPVs("MatchingPVs");             // For charged PFOs
    const static SG::AuxElement::Accessor< std::vector<unsigned> > matchedPUSBs("MatchingPUsidebands");   // For charged PFOs
    PFlowRejecter rejecter(skipNegativeEnergy, useChargedPFOs, useNeutralPFOs, false, false, isUFO);
    std::vector<fastjet::PseudoJet> vpj;
    int index = -1;

    // loop over the input iparticles, select and  convert to pseudojets
    for(const xAOD::IParticle* ip: ips) {

      const xAOD::FlowElement* pfo = static_cast<const xAOD::FlowElement*>(ip);
      ++index;
      if(rejecter(ip)){
        continue;
      }

      unsigned vertexIndex{0};
      if (pfo->isCharged())
      {
        // Charged PFOs - use the vertex matched to the track
        if (matchedPVs.isAvailable(*pfo) && matchedPVs(*pfo).size())
        {
          // A charged PFO can potentially match multiple vertices, depending on the matching criteria used
          // For now, just use the first match, to be further optimised later -- TODO
          // Also add the part for PU sidebands -- TODO
          vertexIndex = matchedPVs(*pfo).at(0);
        }
        else{
          continue;
        }
      }
      else
      {
        // Neutral PFOs - there is one neutral PFO corrected to point to each vertex of interest
        // As such, just get the vertex index that this neutral PFO corresponds to
        if (copyIndex.isAvailable(*pfo)){
          vertexIndex = copyIndex(*pfo);
        }
        else{
          continue;         
        }
      }
    
      // Create a Pseudojet with the momentum of the selected IParticles.
      fastjet::PseudoJet psj(ip->p4());


      // Get the specified vertex and build the VertexIndexedConstituentUserInfo
      for (const xAOD::Vertex* vertex : *pvs)
        if (vertex->index() == vertexIndex){
          // vertex indexed constituent info associated to the pseudojet
          psj.set_user_info(new jet::VertexIndexedConstituentUserInfo(vertex));

          // user index is used to identify the xAOD object used for the PSeudoJet
          psj.set_user_index(index);

          vpj.push_back(psj);
        }
    }
    return vpj;
  }

#endif

}

#endif
