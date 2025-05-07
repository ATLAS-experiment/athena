/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPJETALG_H
#define TRIGL0GEPPERF_GEPJETALG_H

/*
  This algorithm creates jets from CaloClusters, and writes them out
   as xAOD::Jets. The origin of the clusters maybe via standard ATLS
   code, or by Gep clustering. The jet strategy is
   carried out by helper objects.
   The strategy used is chosen according to string set at configure time. *
*/



#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "xAODJet/JetContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"

//Athena::Units::GeV
#include "AthenaKernel/Units.h"

#include <string>


class GepJetAlg: public ::AthReentrantAlgorithm {
 public:
  
  GepJetAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ) const override;    
  

 private:

  Gaudi::Property<std::string> m_jetAlgName{this, "jetAlgName", "",
      "Gep jet alg idenfifier"};
  
  SG::ReadHandleKey< xAOD::CaloClusterContainer> m_caloClustersKey {
    this, "caloClustersKey", "", "key to read in a CaloCluster constainer"};

  SG::ReadHandleKey<xAOD::jFexSRJetRoIContainer> m_jFexSRJetsKey {
    this, "jFexSRJetRoIs", "L1_jFexSRJetRoISim", "key to read a L1 jet container"};
  
  SG::WriteHandleKey<xAOD::JetContainer> m_outputGepJetsKey{
    this, "outputJetsKey", "",
    "key for xAOD:Jet wrappers for GepJets"};

  // WTAConeMaker parameters
  Gaudi::Property<float> m_WTAConstEtCut{this, "WTAConstEtCut", 2.0,
    "Minimum Et for a tower to be considered as a constituent"};

  Gaudi::Property<float> m_WTASeedEtCut{this, "WTASeedEtCut", 5.0,
      "Minimum Et for a tower to be considered as a seed"};

  Gaudi::Property<float> m_WTAJet_dR2{this, "WTAJet_dR2", 0.16,
      "Jet area for merging constituents"};

  Gaudi::Property<unsigned int> m_WTAMaxConstN{this, "WTAMaxConstN", 205,
      "Maximum number of constituents per jet"};

  Gaudi::Property<unsigned int> m_WTAMaxSeedSortingN{this, "WTAMaxSeedSortingN", 50,
      "Maximum number of seeds to sort"};

  Gaudi::Property<unsigned int> m_WTABlockN{this, "WTABlockN", 4,
      "Number of blocks to divide the input towers into for parallel processing. Options: 1, 4"};

  Gaudi::Property<std::string> m_WTASeedCleaningName{this, "WTASeedCleaningName", "TwoPass",
      "Seed cleaning algorithm to use. Options: Baseline, TwoPass"};

};

#endif //> !TRIGL0GEPPERF_GEPJETALG_H
