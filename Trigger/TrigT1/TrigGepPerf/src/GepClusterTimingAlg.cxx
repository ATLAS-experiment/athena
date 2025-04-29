/*
 *   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "./GepClusterTimingAlg.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

GepClusterTimingAlg::GepClusterTimingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator){
}


GepClusterTimingAlg::~GepClusterTimingAlg() {}


StatusCode GepClusterTimingAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");

  // Initialize data access keys
  CHECK(m_inCaloClustersKey.initialize());
  CHECK(m_outCaloClustersKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepClusterTimingAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode GepClusterTimingAlg::execute(const EventContext& ctx) const{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  // This is a filtering Algorithm: it writes out a CaloClusterCollection obtained
  // filed by clusters selected from an input CaloClusterCollection, and writes
  // out the selected clusters to s view contsainer (ie no copying)

  // read in clusters
  auto h_inCaloClusters = SG::makeHandle(m_inCaloClustersKey, ctx);
  CHECK(h_inCaloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << (h_inCaloClusters->size()) << " clusters");

  
  auto h_outCaloClusters = SG::makeHandle(m_outCaloClustersKey, ctx);

  CHECK(h_outCaloClusters.record(std::make_unique<ConstDataVector<xAOD::CaloClusterContainer>>(SG::VIEW_ELEMENTS)));
 
 
  const static SG::AuxElement::ConstAccessor<float> acc_larq("AVG_LAR_Q");
  const static SG::AuxElement::ConstAccessor<float> acc_clambda("CENTER_LAMBDA");

  // select clusters
  for ( const auto cluster : *h_inCaloClusters) {
        
    float time = cluster->time(); 
    float quality = acc_larq(*cluster)/65535; 
    float lambda_center = acc_clambda(*cluster); 
    
    float timeCut = quality > m_qualityCut ? m_timeCutLargeQ : m_timeCutSmallQ;
    if(lambda_center < m_lambdaCalDivide && std::abs(cluster->eta()) < m_etaCut) {
      if( abs(time) > timeCut )continue; 
    }

    // record cluster if selected
    h_outCaloClusters->push_back(cluster);

  }

  ATH_MSG_DEBUG("Read In " << h_inCaloClusters->size() << " Selected " << h_outCaloClusters->size());
  
  return StatusCode::SUCCESS;
}



