/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * =====================================================================================
 *
 *       Filename:  TrigEgammaPrecisionPhotonCaloIsoHypoTool.cxx
 *
 *    Description:  Hypo tool for Calorimeter isolation applied HLT precision step for photon triggers
 *
 *        Created:  08/09/2022 11:19:55 AM
 *
 *         Author:  Fernando Monticelli (), Fernando.Monticelli@cern.ch
 *   Organization:  UNLP/IFLP/CONICET
 *
 * =====================================================================================
 */
#include <algorithm>
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "TrigEgammaPrecisionPhotonCaloIsoHypoTool.h"

namespace TCU = TrigCompositeUtils;

TrigEgammaPrecisionPhotonCaloIsoHypoTool::TrigEgammaPrecisionPhotonCaloIsoHypoTool( const std::string& type, 
        const std::string& name, 
        const IInterface* parent ) 
  : base_class( type, name, parent ),
    m_decisionId( HLT::Identifier::fromToolName( name ) ) {
}


StatusCode TrigEgammaPrecisionPhotonCaloIsoHypoTool::initialize()  
{
  ATH_MSG_DEBUG( "Initialization completed successfully"   );    
  ATH_MSG_DEBUG( "EtaBins        = " << m_etabin   );
  
   if ( m_etabin.empty() ) {
    ATH_MSG_ERROR(  " There are no cuts set (EtaBins property is an empty list)" );
    return StatusCode::FAILURE;
  }

  for (unsigned idx = 0; idx < s_nCones; ++idx) {
    if (s_coneSizes[idx] == m_TopoEtConeSize) {
      m_coneIdx = idx;
      break;
    }
  }
  if ( m_coneIdx < 0 && !m_acceptAll ) {
    ATH_MSG_ERROR("Invalid cone size: " << m_TopoEtConeSize);
    return StatusCode::FAILURE;
  }

  // Retrieving Luminosity info
  ATH_MSG_DEBUG( "Retrieving luminosityCondData..."  );
  ATH_CHECK( m_avgMuKey.initialize() );

  ATH_MSG_DEBUG( "Tool configured for chain/id: " << m_decisionId );

  if ( not m_monTool.name().empty() ) 
    CHECK( m_monTool.retrieve() );

  return StatusCode::SUCCESS;
}


bool TrigEgammaPrecisionPhotonCaloIsoHypoTool::decide( const ITrigEgammaPrecisionPhotonCaloIsoHypoTool::PhotonInfo& input ) const 
{
  auto mon_ET              = Monitored::Scalar( "Et_em", -1.0 );
  auto mon_etaBin          = Monitored::Scalar( "EtaBin", -1.0 );
  auto mon_Eta             = Monitored::Scalar( "Eta", -99. );
  auto mon_Phi             = Monitored::Scalar( "Phi", -99. );
  auto mon_mu              = Monitored::Scalar("mu",   -1.);
  auto mon_etcone20        = Monitored::Scalar("etcone20",   -99.);
  auto mon_topoetcone20    = Monitored::Scalar("topoetcone20",   -99.);
  auto mon_relEtCone20     = Monitored::Scalar("relEtCone20",   -99.);
  auto mon_relTopoEtCone20 = Monitored::Scalar("relTopoEtCone20",   -99.);

  auto mon_etcone30        = Monitored::Scalar("etcone30",   -99.);
  auto mon_topoetcone30    = Monitored::Scalar("topoetcone30",   -99.);
  auto mon_relEtCone30     = Monitored::Scalar("relEtCone30",   -99.);
  auto mon_relTopoEtCone30 = Monitored::Scalar("relTopoEtCone30",   -99.);

  auto mon_etcone40        = Monitored::Scalar("etcone40",   -99.);
  auto mon_topoetcone40    = Monitored::Scalar("topoetcone40",   -99.);
  auto mon_relEtCone40     = Monitored::Scalar("relEtCone40",   -99.);
  auto mon_relTopoEtCone40 = Monitored::Scalar("relTopoEtCone40",   -99.);

  auto PassedCuts          = Monitored::Scalar<int>( "CutCounter", -1 );  
  auto monitorIt           = Monitored::Group( m_monTool, 
                                        mon_etaBin, mon_Eta, mon_Phi, mon_mu, 
                                        mon_etcone20, mon_topoetcone20, mon_relEtCone20, mon_relTopoEtCone20,
                                        mon_etcone30, mon_topoetcone30, mon_relEtCone30, mon_relTopoEtCone30,
                                        mon_etcone40, mon_topoetcone40, mon_relEtCone40, mon_relTopoEtCone40,
										PassedCuts );

  // when leaving scope it will ship data to monTool
  PassedCuts = PassedCuts + 1; //got called (data in place)

  auto roiDescriptor = input.roi;

  if ( fabs( roiDescriptor->eta() ) > 2.6 ) {
      ATH_MSG_DEBUG( "REJECT The photon had eta coordinates beyond the EM fiducial volume : " 
                    << roiDescriptor->eta() << "; stop the chain now" );
      return false; // special case
  } 

  ATH_MSG_DEBUG( "; RoI ID = " << roiDescriptor->roiId() 
                << ": Eta = " << roiDescriptor->eta() 
                << ", Phi = " << roiDescriptor->phi() );


  const auto pClus = input.photon->caloCluster();
  
  const float absEta = fabs( pClus->eta() );
  const int cutIndex = findCutIndex( absEta );

  // eta range
  if ( !m_acceptAll && cutIndex == -1 ) {  // VD
    ATH_MSG_DEBUG( "Photon : " << absEta << " outside eta range " << m_etabin[m_etabin.size()-1] );
    return false;
  } else { 
    ATH_MSG_DEBUG( "eta bin used for cuts " << cutIndex << " AcceptAll = " << m_acceptAll );
  }
  mon_etaBin = m_etabin[cutIndex]; 
  PassedCuts = PassedCuts + 1; // passed eta cut
  
  const float photon_eT = pClus->et();
  mon_ET = photon_eT;

  // get average luminosity information to calculate LH
  float avg_mu = 0; 
  SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_avgMuKey);
  if(eventInfoDecor.isPresent()) {
    avg_mu = eventInfoDecor(0);
    ATH_MSG_DEBUG("Average mu " << avg_mu);
  }
  mon_mu = avg_mu;

  float ptCone[s_nCones]     = {999, 999, 999};
  float etCone[s_nCones]     = {999, 999, 999};
  float topoEtCone[s_nCones] = {999, 999, 999};

  float relEtCone[s_nCones]     = {999, 999, 999};
  float relTopoEtCone[s_nCones] = {999, 999, 999};

  for (int idx = 0; idx < s_nCones; ++idx) {
    input.photon->isolationValue(ptCone[idx],     s_ptConeIsoTypes[idx]);
    input.photon->isolationValue(etCone[idx],     s_etConeIsoTypes[idx]);
    input.photon->isolationValue(topoEtCone[idx], s_topoEtconeIsoTypes[idx]);

    ATH_MSG_DEBUG( " ptCone" << s_coneSizes[idx] << "     = " << ptCone[idx] );
    ATH_MSG_DEBUG( " etCone" << s_coneSizes[idx] << "     = " << etCone[idx] );
    ATH_MSG_DEBUG( " topoEtCone" << s_coneSizes[idx] << " = " << topoEtCone[idx] );
  }

  // Calculate relative isolations and assign monitoring variables
  for (int idx = 0; idx < s_nCones; ++idx) {
    relEtCone[idx]     = etCone[idx] / photon_eT;
    relTopoEtCone[idx] = topoEtCone[idx] / photon_eT;

    ATH_MSG_DEBUG("relEtCone" << s_coneSizes[idx] << " = " << relEtCone[idx]);
    ATH_MSG_DEBUG("relTopoEtCone" << s_coneSizes[idx] << " = " << relTopoEtCone[idx]);
  }

  // Fill monitoring variables (per-cone names preserved for downstream monitoring)
  mon_etcone20 = etCone[0];
  mon_topoetcone20 = topoEtCone[0];
  mon_relEtCone20 = relEtCone[0];
  mon_relTopoEtCone20 = relTopoEtCone[0];

  mon_etcone30 = etCone[1];
  mon_topoetcone30 = topoEtCone[1];
  mon_relEtCone30 = relEtCone[1];
  mon_relTopoEtCone30 = relTopoEtCone[1];

  mon_etcone40 = etCone[2];
  mon_topoetcone40 = topoEtCone[2];
  mon_relEtCone40 = relEtCone[2];
  mon_relTopoEtCone40 = relTopoEtCone[2];

  if ( m_acceptAll ) {
    return true;
  }

  const bool pass_relTopoEtCone = ( m_RelTopoEtConeCut > 900 || ( relTopoEtCone[m_coneIdx] - m_CutOffset/photon_eT < m_RelTopoEtConeCut ));
  ATH_MSG_DEBUG(
    "pass_relTopoEtCone for cone size " << m_TopoEtConeSize <<  " (m_coneIdx = " << m_coneIdx << ") =  "
    << relTopoEtCone[m_coneIdx] << " - " << m_CutOffset << '/' << photon_eT << "  < "
    << m_RelTopoEtConeCut << " = " << pass_relTopoEtCone
  );

  return pass_relTopoEtCone;
 
}

int TrigEgammaPrecisionPhotonCaloIsoHypoTool::findCutIndex( float eta ) const {
  const float absEta = std::abs(eta);
  
  auto binIterator = std::adjacent_find( m_etabin.begin(), m_etabin.end(), [=](float left, float right){ return left < absEta and absEta < right; }  );
  if ( binIterator == m_etabin.end() ) {
    return -1;
  }
  return  binIterator - m_etabin.begin();
}


StatusCode TrigEgammaPrecisionPhotonCaloIsoHypoTool::decide( std::vector<PhotonInfo>& input )  const {
  for ( auto& i: input ) {
    if ( TCU::passed ( m_decisionId.numeric(), i.previousDecisionIDs ) ) {
      if ( decide( i ) ) {
        TCU::addDecisionID( m_decisionId, i.decision );
      }
    }
  }
  return StatusCode::SUCCESS;
}
