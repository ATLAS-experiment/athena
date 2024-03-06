/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_GFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_GFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"

#include "xAODTrigL1Calo/gFexTowerContainer.h"

#include "LArRecConditions/LArBadChannelCont.h"

class GfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
  public:
    GfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
    virtual ~GfexSimMonitorAlgorithm()=default;
    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

  private:

    bool compareJetRoI(const std::string& label, const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs1Key, const SG::ReadHandleKey<xAOD::gFexJetRoIContainer>& tobs2Key, const EventContext& ctx) const;

    bool compareGlobalRoI(const std::string& label, const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs1Key, const SG::ReadHandleKey<xAOD::gFexGlobalRoIContainer>& tobs2Key, const EventContext& ctx) const;


    // container keys for Data tobs

    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_data_gFexRho                {this,"gFexRhoOutputContainer","L1_gFexRhoRoI","SG key of the gFEX EDM gFexRho container"};
    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_data_gFexBlock              {this,"gFexSRJetOutputContainer","L1_gFexSRJetRoI","SG key of the  gFEX EDM gFexBlock container"};
    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_data_gFexJet                {this,"gFexLRJetOutputContainer","L1_gFexLRJetRoI","SG key of the gFEX EDM gFexJet container"};

    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gScalarEJwoj           {this,"gScalarEJwojOutputContainer","L1_gScalarEJwoj","SG key  gFEX EDM Scalar MET and SumET (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gMETComponentsJwoj     {this,"gMETComponentsJwojOutputContainer","L1_gMETComponentsJwoj","SG key  gFEX EDM total MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gMHTComponentsJwoj     {this,"gMHTComponentsJwojOutputContainer","L1_gMHTComponentsJwoj","SG key  gFEX EDM hard MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gMSTComponentsJwoj     {this,"gMSTComponentsJwojOutputContainer","L1_gMSTComponentsJwoj","SG key  gFEX EDM soft MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gMETComponentsNoiseCut {this,"gMETComponentsNoiseCutOutputContainer","L1_gMETComponentsNoiseCut","SG key  gFEX EDM total MET components (NoiseCut) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gMETComponentsRms      {this,"gMETComponentsRmsOutputContainer","L1_gMETComponentsRms","SG key  gFEX EDM total MET components (RMS) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gScalarENoiseCut       {this,"gScalarENoiseCutOutputContainer","L1_gScalarENoiseCut","SG key  gFEX EDM Scalar MET and SumET (NoiseCut) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_data_gScalarERms            {this,"gScalarERmsOutputContainer","L1_gScalarERms","SG key  gFEX EDM Scalar MET and SumET (RMS) container"};



    SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArMaskedChannelKey", "LArMaskedSC", "Key of the OTF-Masked SC" };


    // container keys for Simulation tobsi
//        SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_key_gGlob  {this,"gFexGlobalRoISimContainer","L1_gFexGlobalRoISim","SG key of the Sim gFex Global Roi container"    };
//        SG::ReadHandleKey< xAOD::gFexJetRoIContainer   > m_simu_key_gJ  {this,"gFexJetRoISimContainer"  ,"L1_gFexJetRoISim"  ,"SG key of the Sim gFex Jet Roi container"   };


    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_simu_gFexRho                {this,"gFexRhoOutputSimContainer","L1_gFexRhoRoISim","SG key of the Sim gFEX EDM gFexRho container"};
    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_simu_gFexBlock              {this,"gFexSRJetOutputSimContainer","L1_gFexSRJetRoISim","SG key of the  Sim gFEX EDM gFexBlock container"};
    SG::ReadHandleKey< xAOD::gFexJetRoIContainer    > m_simu_gFexJet                {this,"gFexLRJetOutputSimContainer","L1_gFexLRJetRoISim","SG key of the Sim gFEX EDM gFexJet container"};

    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gScalarEJwoj           {this,"gScalarEJwojOutputSimContainer","L1_gScalarEJwojSim","SG key Sim gFEX EDM Scalar MET and SumET (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gMETComponentsJwoj     {this,"gMETComponentsJwojOutputSimContainer","L1_gMETComponentsJwojSim","SG key Sim  gFEX EDM total MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gMHTComponentsJwoj     {this,"gMHTComponentsJwojOutputSimContainer","L1_gMHTComponentsJwojSim","SG key Sim gFEX EDM hard MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gMSTComponentsJwoj     {this,"gMSTComponentsJwojOutputSimContainer","L1_gMSTComponentsJwojSim","SG key Sim gFEX EDM soft MET components (JwoJ) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gMETComponentsNoiseCut {this,"gMETComponentsNoiseCutOutputSimContainer","L1_gMETComponentsNoiseCutSim","SG key Sim gFEX EDM total MET components (NoiseCut) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gMETComponentsRms      {this,"gMETComponentsRmsOutputSimContainer","L1_gMETComponentsRmsSim","SG key Sim gFEX EDM total MET components (RMS) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gScalarENoiseCut       {this,"gScalarENoiseCutOutputSimContainer","L1_gScalarENoiseCutSim","SG key Sim gFEX EDM Scalar MET and SumET (NoiseCut) container"};
    SG::ReadHandleKey< xAOD::gFexGlobalRoIContainer > m_simu_gScalarERms            {this,"gScalarERmsOutputSimContainer","L1_gScalarERmsSim","SG key Sim  gFEX EDM Scalar MET and SumET (RMS) container"};

};
#endif

