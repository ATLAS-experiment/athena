/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/EfficiencyResponseHistos.h"

#include "JetMonitoring/ToolHandleHistoHelper.h"
#include "JetUtils/JetDistances.h"
#include "AthenaKernel/Units.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
#include <cmath>


using Athena::Units::GeV;


EfficiencyResponseHistos::EfficiencyResponseHistos(const std::string &t) : JetHistoBase(t) 
                                                           , m_histoDef(this)
{
  declareProperty("HistoDef", m_histoDef, "The list of HistoDefinitionTool defining the histos to be used in this tool"); 
  declareProperty("RefContainer", m_refContainerName);
}



StatusCode EfficiencyResponseHistos::initialize() {
  CHECK( m_histoDef.retrieve() );
  return StatusCode::SUCCESS;
}

int EfficiencyResponseHistos::buildHistos(){
  ATH_MSG_INFO(" buildHistos num of histos : "<< m_histoDef.size() );

  ToolHandleHistoHelper::HistoRetriever hbuilder(m_histoDef);

  // Histos are defined in jobOptions !
  // For each histo, ask hbuilder if a corresponding definition exists in the jobOption list.
  //  -> if so a valid histo is returned (and booked)
  //  -> else NULL is returned
  m_eff1 = bookHisto( hbuilder.build<TProfile>("erhEfficiencyR1") );
  m_eff2 = bookHisto( hbuilder.build<TProfile>("erhEfficiencyR2") );
  m_eff3 = bookHisto( hbuilder.build<TProfile>("erhEfficiencyR3") );

  m_etres = bookHisto( hbuilder.build<TH1F>("erhResponse") );
  m_etres_eta =bookHisto( hbuilder.build<TProfile>("erhResponseVsEta") );
  m_etres_pt =bookHisto( hbuilder.build<TProfile>("erhResponseVsPt") );

  m_etres_noShift = bookHisto( hbuilder.build<TH1F>("erhResponse_noShift") );
  m_etres_noShift_eta =bookHisto( hbuilder.build<TProfile>("erhResponseVsEta_noShift") );
  m_etres_noShift_pt =bookHisto( hbuilder.build<TProfile>("erhResponseVsPt_noShift") );

  m_deltaRclosest = bookHisto( hbuilder.build<TH1F>("erhDeltaR") );


  return 0;
}


int EfficiencyResponseHistos::fillHistosFromContainer(const xAOD::JetContainer &cont, float weight){

  const xAOD::JetContainer* refContainer = nullptr;
  CHECK( evtStore()->retrieve( refContainer, m_refContainerName), 1 );
  /// use a list to be a bit more efficient.
  std::list<const xAOD::Jet*> listJets(cont.begin(), cont.end());

  for ( const xAOD::Jet* refjet : *refContainer ){
    double dr2min = 500000;

    if (listJets.empty() ) break;
    // find the min match
    std::list<const xAOD::Jet*>::iterator it=listJets.begin();
    std::list<const xAOD::Jet*>::iterator itmin=listJets.end();
    for( ; it != listJets.end(); ++it) {
      double dr2 = jet::JetDistances::deltaR2(*(*it),*refjet);
      if(dr2 < dr2min) { dr2min = dr2; itmin = it ;}
    }
    if (itmin == listJets.end()) break;
    const xAOD::Jet* matched = *itmin;
    listJets.erase(itmin);
    
    double dr = std::sqrt(dr2min);
    double refPt = refjet->pt() / GeV;

    m_eff1->Fill(refPt, dr<0.1 ?  weight : 0 ); // 0 weight if not matching close enough
    m_eff2->Fill(refPt, dr<0.2 ?  weight : 0 ); // 0 weight if not matching close enough
    m_eff3->Fill(refPt, dr<0.3 ?  weight : 0 ); // 0 weight if not matching close enough
    
    m_deltaRclosest->Fill( dr, weight );

    if( dr < 0.3) {
      double relDiff = -999;
      double response = -999;
      if (refPt > 0.){
	relDiff = ( matched->pt() / GeV - refPt )/refPt;
	response = (matched->pt() / GeV)/refPt;
      }
      m_etres->Fill( relDiff, weight );
      m_etres_eta->Fill( refjet->eta(), relDiff, weight);
      m_etres_pt->Fill( refPt, relDiff, weight);

      m_etres_noShift->Fill( response, weight );
      m_etres_noShift_eta->Fill( refjet->eta(), response, weight);
      m_etres_noShift_pt->Fill( refPt, response, weight);
    }

  }

  
  return 0;
}
