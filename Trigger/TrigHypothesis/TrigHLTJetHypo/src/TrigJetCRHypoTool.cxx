/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TrigJetCRHypoTool.cxx
// PACKAGE:  Trigger/TrigHypothesis/TrigHLTJetHypo
//
//
// ********************************************************************

#include "TrigJetCRHypoTool.h"
#include "GaudiKernel/StatusCode.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/xAODJetAsIJetFactory.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/lineSplitter.h"
#include "./DebugInfoCollector.h"
#include "./xAODJetCollector.h"

#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "CxxUtils/fpcompare.h"
#include "CxxUtils/phihelper.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloClusterContainer.h"
using namespace TrigCompositeUtils;

TrigJetCRHypoTool::TrigJetCRHypoTool(const std::string& type,
				     const std::string& name,
				     const IInterface* parent) :
  AthAlgTool(type, name, parent),
  m_decisionId(HLT::Identifier::fromToolName(name)){

}



StatusCode TrigJetCRHypoTool::initialize(){
  ATH_MSG_VERBOSE( "In TrigJetCRHypoTool, initialize()." ); 

  if (m_visitDebug){

    DebugInfoCollector collector(name());
    ATH_CHECK(m_helper->getDescription(collector));
    auto s = collector.toString();
  
    for(const auto& l : lineSplitter(s)){
      ATH_MSG_INFO(l);
    }
  }

  return StatusCode::SUCCESS;
}

bool TrigJetCRHypoTool::decide_on_single_jet( JetInfo& input ) const {

  auto jet = input.jet;
  auto trackContainer = input.allTracks;
  auto cellContainer = input.cells;

  //Checking jet logRatio requirements
  double jetEMF = jet->getAttribute<float>("EMFrac");
  double jetRatio;

  if (CxxUtils::fpcompare::greater(jetEMF,0.)){
    if(CxxUtils::fpcompare::greater_equal(jetEMF,1.)){
      ATH_MSG_DEBUG( "Fails logR cut, EMF>=1" );
      return false;
    }
    else jetRatio = log10(double(1./jetEMF - 1.));

    if(  jetRatio < m_jetlogRCut  ) return false;
  }

  // Loop over all tracks above m_trackPtCut and reject the jet if the closest track is at dR(jet, track)< m_deltaR
  auto jetPhi= jet->phi();
  auto jetEta= jet->eta();

  for ( const xAOD::TrackParticle_v1* track : *trackContainer) {

    if(track->pt() < m_trackPtCut ) continue;

    double phi  = track->phi0();
    double eta  = track->eta();

    double dR = xAOD::P4Helpers::deltaR( eta, phi, jetEta, jetPhi );
    if (dR<m_deltaR)   return false;
  }


  if(m_doBIBrm==1){
    int countCaloCell=0;
    double countCell_layer[4] = {0,0,0,0};

    for(CaloCellContainer::const_iterator celliter = cellContainer->begin(); celliter != cellContainer->end(); ++celliter){
      //LoF cell selection in tile:
      if((*celliter)->caloDDE()->is_tile() && (*celliter)->energy() > m_minCellEt){
     	  double d_phi = xAOD::P4Helpers::deltaPhi( (*celliter)->phi(), jetPhi );
	      double d_R = xAOD::P4Helpers::deltaR( (*celliter)->eta(), (*celliter)->phi(), jetEta, jetPhi );
	      //select cells in a horizontal line, not being part of the jet and timing consistent with BIB
	      if(std::abs(d_phi) < 0.2 && d_R > 0.4){
          //-early
          float t = (*celliter)->time();
          if(t < m_celltime){
            // check cell's time consitency with BIB
            float x = (*celliter)->x();
            float y = (*celliter)->y();
            float z = (*celliter)->z();
            float r = sqrt(x*x + y*y);

	          if((std::abs(t - (z-sqrt(z*z + r*r))/CLHEP::c_light) < m_dBIBtime) || (std::abs(t - (-z-sqrt(z*z + r*r))/CLHEP::c_light) < m_dBIBtime)){
    	        // for selected cells, store in which layer they are
	            if(r<2200){ countCell_layer[0]++;}
	            else if(r>=2200 && r<2600){ countCell_layer[1]++;}
	            else if(r>=2600 && r<3100){ countCell_layer[2]++;}
	            else if(r>=3100){ countCell_layer[3]++;}
	          }
	        }
	      }
      }
    }

    // get max number of selected cells in a layer
    for(int i=0; i<4; i++) 
      if(countCaloCell<countCell_layer[i]) countCaloCell=countCell_layer[i];

    //apply cut on number of cells here
    if (countCaloCell>=m_countCaloCell) return false;
    
    return true;
  }

  return  true;
}

StatusCode TrigJetCRHypoTool::decide( std::vector<JetInfo>& input )  const{
  for ( JetInfo& j: input ) {
    if ( passed ( m_decisionId.numeric(), j.previousDecisionIDs ) ) {
      if ( decide_on_single_jet( j ) ) {
	         addDecisionID( m_decisionId, j.decision );
           return StatusCode::SUCCESS;

      }
    }
  }

  return StatusCode::SUCCESS;
}


