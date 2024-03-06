/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H
#define TRIGT1CALOMONITORING_EFEXSIMMONITORALGORITHM_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
//
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexEMRoI.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/eFexTauRoI.h"

#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/eFexTower.h"
#include "FourMomUtils/P4Helpers.h"

#include "LArRecConditions/LArBadChannelCont.h"

class EfexSimMonitorAlgorithm : public AthMonitorAlgorithm {
public:EfexSimMonitorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator );
  virtual ~EfexSimMonitorAlgorithm()=default;
  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:

  StringProperty m_packageName{this,"PackageName","EfexSimMonitor","group name for histograming"};

  // these maps hold the binlabels (in form of LBN:FirstEventNum) to use for each lb
  mutable std::map<int,std::string> m_firstEvents_DataTowers ATLAS_THREAD_SAFE;
  mutable std::map<int,std::string> m_firstEvents_EmulatedTowers ATLAS_THREAD_SAFE;
  mutable std::mutex m_firstEventsMutex;

    // container keys including this, steering parameter, default value and help description
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmContainerKey{this,"eFexEMRoIContainer","L1_eEMRoI","SG key of the data eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauContainerKey{this,"eFexTauRoIContainer","L1_eTauRoI","SG key of the data eFex Tau RoI container"};
  SG::ReadHandleKey<xAOD::eFexEMRoIContainer> m_eFexEmSimContainerKey{this,"eFexEMRoISimContainer","L1_eEMRoISim","SG key of the simulated eFex Em RoI container"};
  SG::ReadHandleKey<xAOD::eFexTauRoIContainer> m_eFexTauSimContainerKey{this,"eFexTauSimRoIContainer","L1_eTauRoISim","SG key of the simulated eFex Tau RoI container"};

  // SG::ReadDecorHandleKey<xAOD::EventInfo> m_decorKey;
  SG::ReadHandleKey<xAOD::eFexTowerContainer> m_eFexTowerContainerKey{this,"eFexTowerContainer","L1_eFexDataTowers","SG key of the primary eFex tower container, which should be populated if fex readout occurring"};

  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArMaskedChannelKey", "LArMaskedSC", "Key of the OTF-Masked SC" };


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
            for(auto tob : *tobs) {
                sortedTobs.emplace_back(SortableTob{tob->word0(),tob->eta(),tob->phi()});
            }
            std::sort(sortedTobs.begin(),sortedTobs.end(),[](const SortableTob& lhs, const SortableTob& rhs) { return lhs.word0<rhs.word0; });
            for(auto& tob : sortedTobs) {
                etas.push_back(tob.eta);
                phis.push_back(tob.phi);
                word0s.push_back(tob.word0);
            }
        }
    }


  template <typename T> unsigned int fillHistos(const SG::ReadHandleKey<T>& key1, const SG::ReadHandleKey<T>& key2, const std::string& eventType, const EventContext& ctx, const std::string& signa = "" ) const {
      SG::ReadHandle<T> tobs1{key1, ctx};
      SG::ReadHandle<T> tobs2{key2, ctx};

      std::set<uint32_t> word0s2;
      if(tobs2.isValid()) {
          for(auto tob : *tobs2) {
              word0s2.insert(tob->word0());
          }
      }

      auto signature = Monitored::Scalar<std::string>("Signature",signa);
      auto evtType = Monitored::Scalar<std::string>("EventType",eventType);
      auto tobMismatched = Monitored::Scalar<float>("tobMismatched",0.0);

      // for each collection record if TOB is matched or not
      unsigned int nUnmatched = 0;
      if(tobs1.isValid()) {
          for(auto tob : *tobs1) {
              tobMismatched=100;
              if(word0s2.find(tob->word0()) == word0s2.end()) {
                  nUnmatched++;
              } else {
                  tobMismatched=0;
              }
              fill("mismatches",signature,evtType,tobMismatched);
//              if(tobMismatched && this->msgLevel(MSG::DEBUG)) {
//                  std::cout << "evtNumber " << GetEventInfo(ctx)->eventNumber() << " " << tobs1.key() << " " << (groupSuffix=="2" ? "L1_eFexDataTowers" : "L1_eFexEmulatedTowers") << " mismatched: 0x" << std::hex << tob->word0() << std::dec << " (" << tob->eta() << "," << tob->phi() << ")" << std::endl;
//                  for(auto tower : *towers) {
//                      if (std::abs(tower->eta() - tob->eta()) < 0.2 && std::abs(P4Helpers::deltaPhi(tower->phi(),tob->phi()))<0.2) {
//                          std::cout << tower->eta() << " " << tower->phi() << " : ";
//                          for(auto& c : tower->et_count()) std::cout << c << ",";
//                          std::cout << std::endl;
//                      }
//                  }
//              }
          }
          if(tobs2.isValid() && tobs1->size() < tobs2->size()) {
              tobMismatched=100;
              for(unsigned int i=0;i<(tobs2->size()-tobs1->size());i++) {
                  nUnmatched++;
                  fill("mismatches",signature,tobMismatched,evtType);
              }
          }
      }
      return nUnmatched;

  }


};
#endif
