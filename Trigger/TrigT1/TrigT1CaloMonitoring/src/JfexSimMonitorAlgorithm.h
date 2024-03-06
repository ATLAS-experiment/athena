/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_JFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_JFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"

#include "xAODTrigL1Calo/jFexTowerContainer.h"

#include "LArRecConditions/LArBadChannelCont.h"

class JfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
    public:
        JfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        virtual ~JfexSimMonitorAlgorithm()=default;
        virtual StatusCode initialize() override;
        virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

    private:

        StringProperty m_Grouphist   {this,"Grouphist"       ,"JfexSimMonitor","group name for histograming"};
        
        ToolHandle<GenericMonitoringTool> m_monTool{this,"jFEXMonTool","","Monitoring tool"};
        void  genError(const std::string& location, const std::string& title) const;
        
        
        // container keys for jfex input data
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_jFexTowerKey{this, "jFexTowerContainer","L1_jFexDataTowers","SG key of the input jFex Tower container"};

        // container keys for Data tobs
        SG::ReadHandleKey< xAOD::jFexSRJetRoIContainer > m_data_key_jJ   {this,"jFexSRJetRoIContainer","L1_jFexSRJetRoI","SG key of the jFex SR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexLRJetRoIContainer > m_data_key_jLJ  {this,"jFexLRJetRoIContainer","L1_jFexLRJetRoI","SG key of the jFex LR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexTauRoIContainer   > m_data_key_jTau {this,"jFexTauRoIContainer"  ,"L1_jFexTauRoI"  ,"SG key of the jFex Tau Roi container"   };
        SG::ReadHandleKey< xAOD::jFexFwdElRoIContainer > m_data_key_jEM  {this,"jFexFwdElRoIContainer","L1_jFexFwdElRoI","SG key of the jFex EM Roi container"    };
        SG::ReadHandleKey< xAOD::jFexMETRoIContainer   > m_data_key_jXE  {this,"jFexMETRoIContainer"  ,"L1_jFexMETRoI"  ,"SG key of the jFex MET Roi container"   };
        SG::ReadHandleKey< xAOD::jFexSumETRoIContainer > m_data_key_jTE  {this,"jFexSumETRoIContainer","L1_jFexSumETRoI","SG key of the jFex SumEt Roi container" };

        // container keys for Simulation tobs
        SG::ReadHandleKey< xAOD::jFexSRJetRoIContainer > m_simu_key_jJ   {this,"jFexSRJetRoISimContainer","L1_jFexSRJetRoISim","SG key of the Sim jFex SR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexLRJetRoIContainer > m_simu_key_jLJ  {this,"jFexLRJetRoISimContainer","L1_jFexLRJetRoISim","SG key of the Sim jFex LR Jet Roi container"};
        SG::ReadHandleKey< xAOD::jFexTauRoIContainer   > m_simu_key_jTau {this,"jFexTauRoISimContainer"  ,"L1_jFexTauRoISim"  ,"SG key of the Sim jFex Tau Roi container"   };
        SG::ReadHandleKey< xAOD::jFexFwdElRoIContainer > m_simu_key_jEM  {this,"jFexFwdElRoISimContainer","L1_jFexFwdElRoISim","SG key of the Sim jFex EM Roi container"    };
        SG::ReadHandleKey< xAOD::jFexMETRoIContainer   > m_simu_key_jXE  {this,"jFexMETRoISimContainer"  ,"L1_jFexMETRoISim"  ,"SG key of the Sim jFex MET Roi container"   };
        SG::ReadHandleKey< xAOD::jFexSumETRoIContainer > m_simu_key_jTE  {this,"jFexSumETRoISimContainer","L1_jFexSumETRoISim","SG key of the Sim jFex SumEt Roi container" };

    SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArMaskedChannelKey", "LArMaskedSC", "Key of the OTF-Masked SC" };
        
        template <typename T> std::vector<std::array<float,5> > tobMatching(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<float,5> > & unmatched, const std::string& evtType, const std::string& signa="" ) const;
        void fillHist(const std::string & pkg, const std::string & item, const std::string & input, const std::string & LB, const bool fillError, std::vector< std::array<float,5> > & elem ) const;
        
        template <typename T> std::vector<std::array<int,3> >  tobMatchingGlobals(const SG::ReadHandleKey<T>& tobs1Key, const SG::ReadHandleKey<T>& tobs2Key, const EventContext& ctx, std::vector< std::array<int,3> > & unmatched, const std::string& evtType, const std::string& signa="" ) const;
        void fillHistGlobals(const std::string & pkg, const std::string & item, const std::string & input, const std::string & LB, const bool fillError, std::vector< std::array<int,3> > & elem ) const;
};       
#endif
