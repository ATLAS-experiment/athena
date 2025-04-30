/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/Combinators.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODEgamma/Photon.h"
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

  // Now we try to retrieve the ElectronPhotonSelectorTools that we will use to apply the photon Identification. This is a *must*


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

  bool pass = false;

  auto mon_ET              = Monitored::Scalar( "Et_em", -1.0 );
  auto mon_etaBin          = Monitored::Scalar( "EtaBin", -1.0 );
  auto mon_Eta             = Monitored::Scalar( "Eta", -99. );
  auto mon_Phi             = Monitored::Scalar( "Phi", -99. );
  auto mon_mu              = Monitored::Scalar("mu",   -1.);
  auto mon_etcone20        = Monitored::Scalar("etcone20",   -99.);
  auto mon_topoetcone20    = Monitored::Scalar("topoetcone20",   -99.);
  auto mon_reletcone20     = Monitored::Scalar("reletcone20",   -99.);
  auto mon_reltopoetcone20 = Monitored::Scalar("reltopoetcone20",   -99.);

  auto mon_etcone30        = Monitored::Scalar("etcone30",   -99.);
  auto mon_topoetcone30    = Monitored::Scalar("topoetcone30",   -99.);
  auto mon_reletcone30     = Monitored::Scalar("reletcone30",   -99.);
  auto mon_reltopoetcone30 = Monitored::Scalar("reltopoetcone30",   -99.);

  auto mon_etcone40        = Monitored::Scalar("etcone40",   -99.);
  auto mon_topoetcone40    = Monitored::Scalar("topoetcone40",   -99.);
  auto mon_reletcone40     = Monitored::Scalar("reletcone40",   -99.);
  auto mon_reltopoetcone40 = Monitored::Scalar("reltopoetcone40",   -99.);

  auto PassedCuts          = Monitored::Scalar<int>( "CutCounter", -1 );  
  auto monitorIt           = Monitored::Group( m_monTool, 
                                        mon_etaBin, mon_Eta, mon_Phi, mon_mu, 
                                        mon_etcone20, mon_topoetcone20, mon_reletcone20, mon_reltopoetcone20, 
                                        mon_etcone30, mon_topoetcone30, mon_reletcone30, mon_reltopoetcone30, 
                                        mon_etcone40, mon_topoetcone40, mon_reletcone40, mon_reltopoetcone40, 
										PassedCuts );

  // when leaving scope it will ship data to monTool
  PassedCuts = PassedCuts + 1; //got called (data in place)

  float ET(0);

  auto roiDescriptor = input.roi;

  if ( fabs( roiDescriptor->eta() ) > 2.6 ) {
      ATH_MSG_DEBUG( "REJECT The photon had eta coordinates beyond the EM fiducial volume : " 
                    << roiDescriptor->eta() << "; stop the chain now" );
      pass=false; // special case       
      return pass;
  } 

  ATH_MSG_DEBUG( "; RoI ID = " << roiDescriptor->roiId() 
                << ": Eta = " << roiDescriptor->eta() 
                << ", Phi = " << roiDescriptor->phi() );


  auto pClus = input.photon->caloCluster();
  
  float absEta = fabs( pClus->eta() );
  const int cutIndex = findCutIndex( absEta );
  
  ET  = pClus->et();
  // eta = pClus->eta();
  // phi = pClus->phi();

  // eta range
  if ( !m_acceptAll && cutIndex == -1 ) {  // VD
    ATH_MSG_DEBUG( "Photon : " << absEta << " outside eta range " << m_etabin[m_etabin.size()-1] );
    return pass;
  } else { 
    ATH_MSG_DEBUG( "eta bin used for cuts " << cutIndex << " AcceptAll = " << m_acceptAll );
  }
  mon_etaBin = m_etabin[cutIndex]; 
  PassedCuts = PassedCuts + 1; // passed eta cut
  
  mon_ET = ET; 

  // get average luminosity information to calculate LH
  float avg_mu = 0; 
  SG::ReadDecorHandle<xAOD::EventInfo,float> eventInfoDecor(m_avgMuKey);
  if(eventInfoDecor.isPresent()) {
    avg_mu = eventInfoDecor(0);
    ATH_MSG_DEBUG("Average mu " << avg_mu);
  }
  mon_mu = avg_mu;

  float ptcone20(999), ptcone30(999), ptcone40(999), 
		etcone20(999), etcone30(999), etcone40(999), 
		topoetcone20(999), topoetcone30(999), topoetcone40(999), 
		reletcone20(999), reletcone30(999), reletcone40(999), 
		reltopoetcone20(999), reltopoetcone30(999), reltopoetcone40(999);

    
  // isolation variables
  input.photon->isolationValue(ptcone20, xAOD::Iso::ptcone20);

  input.photon->isolationValue(ptcone30, xAOD::Iso::ptcone30);

  input.photon->isolationValue(ptcone40, xAOD::Iso::ptcone40);

  input.photon->isolationValue(etcone20, xAOD::Iso::etcone20);

  input.photon->isolationValue(etcone30, xAOD::Iso::etcone30);

  input.photon->isolationValue(etcone40, xAOD::Iso::etcone40);

  input.photon->isolationValue(topoetcone20, xAOD::Iso::topoetcone20);

  input.photon->isolationValue(topoetcone30, xAOD::Iso::topoetcone30);

  input.photon->isolationValue(topoetcone40, xAOD::Iso::topoetcone40);

  ATH_MSG_DEBUG( " ptcone20     = " << ptcone20 ) ;
  ATH_MSG_DEBUG( " ptcone30     = " << ptcone30 ) ;
  ATH_MSG_DEBUG( " ptcone40     = " << ptcone40 ) ;
  ATH_MSG_DEBUG( " etcone20     = " << etcone20 ) ;
  ATH_MSG_DEBUG( " etcone30     = " << etcone30 ) ;
  ATH_MSG_DEBUG( " etcone40     = " << etcone40 ) ;
  ATH_MSG_DEBUG( " topoetcone20 = " << topoetcone20 ) ;
  ATH_MSG_DEBUG( " topoetcone30 = " << topoetcone30 ) ;
  ATH_MSG_DEBUG( " topoetcone40 = " << topoetcone40 ) ;

  // Monitor showershapes                      
  float photon_eT = input.photon->caloCluster()->et();
  mon_etcone20 = etcone20;
  reletcone20 = etcone20/photon_eT;
  ATH_MSG_DEBUG("reletcone20 = " <<reletcone20  );
  mon_reletcone20 = reletcone20;

  mon_topoetcone20 = topoetcone20;
  reltopoetcone20 = topoetcone20/photon_eT;
  ATH_MSG_DEBUG("reltopoetcone20 = " <<reltopoetcone20  );
  mon_reltopoetcone20 = reltopoetcone20;

  mon_etcone30 = etcone30;
  reletcone30 = etcone30/photon_eT;
  ATH_MSG_DEBUG("reletcone30 = " <<reletcone30  );
  mon_reletcone30 = reletcone30;

  mon_topoetcone30 = topoetcone30;
  reltopoetcone30 = topoetcone30/photon_eT;
  ATH_MSG_DEBUG("reltopoetcone30 = " <<reltopoetcone30  );
  mon_reltopoetcone30 = reltopoetcone30;

  mon_etcone40 = etcone40;
  reletcone40 = etcone40/photon_eT;
  ATH_MSG_DEBUG("reletcone40 = " <<reletcone40  );
  mon_reletcone40 = reletcone40;

  mon_topoetcone40 = topoetcone40;
  reltopoetcone40 = topoetcone40/photon_eT;
  ATH_MSG_DEBUG("reltopoetcone40 = " <<reltopoetcone40  );
  mon_reltopoetcone40 = reltopoetcone40;

  // Place here all etcone variables to apply the cuts within the loop on cone sizes
  std::vector<float> reltopoetcone;
  reltopoetcone.push_back(reltopoetcone20);
  reltopoetcone.push_back(reltopoetcone30);
  reltopoetcone.push_back(reltopoetcone40);

  std::vector<float> reletcone;
  reletcone.push_back(etcone20);
  reletcone.push_back(etcone30);
  reletcone.push_back(etcone40);


  bool pass_reletcone = true;    // If cut is not succeeded, this will be "AND"ed with a False result
  bool pass_reltopoetcone = true;

  // Loop over three indices 0,1 and 2, each referring to cones 20 30 and 40 and checking whether it passes or not
  for (unsigned int conesize=0; conesize<3; conesize++){
	  ATH_MSG_DEBUG("m_RelEtConeCut[" << conesize << "] = " << m_RelEtConeCut[conesize] );
	  ATH_MSG_DEBUG("m_RelTopoEtConeCut[" << conesize << "] = " << m_RelTopoEtConeCut[conesize] );
	  ATH_MSG_DEBUG("m_CutOffset[" << conesize << "] = " << m_CutOffset[conesize] );

      // Check if need to apply isolation
      // First check logic. if cut is very big, then no isolation cut is defined
      // Applies to both reletcone[20,30,40] and reltopoetcone[20,30,40]
	  if (m_RelEtConeCut[conesize] > 900){ // I guess we want to deprecate this?
		  ATH_MSG_DEBUG(" not applying etcone[" << conesize << "] isolation.");
	  }
	  if (m_RelTopoEtConeCut[conesize] > 900){ // I guess we want to deprecate this?
		  ATH_MSG_DEBUG(" not applying topoetcone[" << conesize << "] isolation.");
	  }
	  bool pass_this_reletcone     = ( m_RelEtConeCut[conesize] > 900 || ( reletcone[conesize] - m_CutOffset[conesize]/photon_eT < m_RelEtConeCut[conesize]));
	  bool pass_this_reltopoetcone = ( m_RelTopoEtConeCut[conesize] > 900 || ( reltopoetcone[conesize] - m_CutOffset[conesize]/photon_eT  < m_RelTopoEtConeCut[conesize]));

	  ATH_MSG_DEBUG(" pass_reletcone[" << conesize << "] =  "  << reletcone[conesize] << " - " << m_CutOffset[conesize] << "/" << photon_eT << "  < " << m_RelEtConeCut[conesize]  << " = " << pass_reletcone);
	  ATH_MSG_DEBUG(" pass_reltopoetcone[" << conesize << "] =  "  << reltopoetcone[conesize] << " - " << m_CutOffset[conesize] << "/" << photon_eT << "  < " << m_RelEtConeCut[conesize] << " = " << pass_reltopoetcone);

	  pass_reletcone     = pass_reletcone     && pass_this_reletcone     ; 
	  pass_reltopoetcone = pass_reltopoetcone && pass_this_reltopoetcone ; 
  }
  // Reach this point successfully  
  pass = m_acceptAll || (pass_reletcone && pass_reltopoetcone);
  ATH_MSG_DEBUG( "AcceptAll          = " << m_acceptAll );
  ATH_MSG_DEBUG( "pass_reletcone     = " << pass_reletcone );
  ATH_MSG_DEBUG( "pass_reltopoetcone = " << pass_reltopoetcone );
  ATH_MSG_DEBUG( "pass               = " << pass );

  return pass;
 
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
