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
#include "CaloEvent/CaloCellContainer.h"

#include "LArRecConditions/LArBadChannelCont.h"

class JfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
    public:
        JfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
        virtual ~JfexSimMonitorAlgorithm()=default;
        virtual StatusCode initialize() override;
        virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

    private:
        
        // container keys for jfex input data
        SG::ReadHandleKey<xAOD::jFexTowerContainer> m_jFexTowerKey{this, "jFexTowerContainer","L1_jFexDataTowers","SG key of the input jFex Tower container"};
        SG::ReadHandleKey<CaloCellContainer> m_scellKey{this, "CaloCellContainerReadKey","SCell","Read handle key for the supercells"};

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

      template <typename T> bool compareRoI(const std::string& label, const std::string& evenType,
                                                                   const SG::ReadHandleKey<T>& tobs1Key,
                                                                   const SG::ReadHandleKey<T>& tobs2Key,
                                                                   const EventContext& ctx, bool simReadyFlag=false, size_t maxTobs=0) const;

        // map hold the binlabels (in form of LBN:FirstEventNum) to use for each lb
        mutable std::map<int,std::string> m_firstEvents ATLAS_THREAD_SAFE;
        mutable std::mutex m_firstEventsMutex;


        struct SortableTob {
            SortableTob(unsigned int w, float e, float p) : word0(w),eta(e),phi(p) { }
            unsigned int word0;
            float eta,phi;
        };
        template <typename T> void fillVectors(const SG::ReadHandleKey<T>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const {
            etas.clear();phis.clear();word0s.clear();
            SG::ReadHandle<T> tobs{key, ctx};
            if(tobs.isValid()) {
                etas.reserve(tobs->size());
                phis.reserve(tobs->size());
                word0s.reserve(tobs->size());
                std::vector<SortableTob> sortedTobs;
                sortedTobs.reserve(tobs->size());
                for(const auto tob : *tobs) {
                    sortedTobs.emplace_back(SortableTob{tob->tobWord(),tob->eta(),tob->phi()});
                }
                std::sort(sortedTobs.begin(),sortedTobs.end(),[](const SortableTob& lhs, const SortableTob& rhs) { return lhs.word0<rhs.word0; });
                for(const auto& tob : sortedTobs) {
                    etas.push_back(tob.eta);
                    phis.push_back(tob.phi);
                    word0s.push_back(tob.word0);
                }
            }
        }

};

// specializations for global tob types (no eta and phi values)
template <> void JfexSimMonitorAlgorithm::fillVectors(const SG::ReadHandleKey<xAOD::jFexMETRoIContainer>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const;
template <> void JfexSimMonitorAlgorithm::fillVectors(const SG::ReadHandleKey<xAOD::jFexSumETRoIContainer>& key, const EventContext& ctx, std::vector<float>& etas, std::vector<float>& phis, std::vector<unsigned int>& word0s) const;

#endif
