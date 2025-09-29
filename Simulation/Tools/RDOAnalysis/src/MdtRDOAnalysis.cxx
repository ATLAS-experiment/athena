/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "MdtRDOAnalysis.h"
#include "StoreGate/ReadHandle.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>
#include <format>

namespace MuonVal{
  using namespace Muon::MuonStationIndex;

  MdtRDOAnalysis::HistoSet::HistoSet(const ServiceHandle<ITHistSvc>& histSvc,
                                     const std::string& basePath,
                                     const StIdx_t stIndex):
      stIdx{stIndex} {

      auto createHisto = [stIndex, &histSvc, &basePath](const std::string& hName,
                                                        const std::string& hTitle,
                                                        const unsigned nBins,
                                                         const float xLow, 
                                                         const float xHigh){
          TH1* h = new TH1F(hName.c_str(), hTitle.c_str(), nBins, xLow, xHigh);
          histSvc->regHist(std::format("/{}/RDO/Station_{}/{}",
                                        basePath, 
                                        stIndex != StIdx_t::StUnknown ? stName(stIndex) : "Inclusive", 
                                        hName), h).ignore();
          return h;
      };

      h_subID = createHisto("subDetId", "Subdetector ID", 100, 0 , 150);
      h_mrodID = createHisto("mrodID", "MROD ID", 100, 0, 150);
      h_csmID = createHisto("csmID", "CSM ID", 100, 0, 10);
      h_tdcID = createHisto("tdcID", "TDC ID", 100, 0, 50);
      h_chanID = createHisto("channelID", "Channel ID", 100, 0, 50);
      h_coarse = createHisto("coarseTime", "Drift time (coarse)", 100, 0, 100);
      h_fine = createHisto("fineTime", "Drift time (fine)", 100, 0, 50);
      h_width = createHisto("width", "Width", 100, 0, 500);
   }

  StatusCode MdtRDOAnalysis::initialize() {
      ATH_MSG_DEBUG( "Initializing MdtRDOAnalysis" );

      // This will check that the properties were initialized
      // properly by job configuration.
      ATH_CHECK(m_inputKey.initialize());
      ATH_CHECK(m_idHelperSvc.retrieve());   
  
      for (int s = static_cast<int>(StIdx_t::StUnknown); s < static_cast<int>(StIdx_t::StIndexMax); ++s) {
         auto stIdx = static_cast<StIdx_t>(s);
         m_histos.emplace_back(histSvc(), m_path, stIdx);
      }

      return StatusCode::SUCCESS;
    }

    StatusCode MdtRDOAnalysis::execute() {
        const EventContext& ctx{Gaudi::Hive::currentContext()};
        const MdtCsmContainer* csmCont{nullptr};
        ATH_CHECK(SG::get(csmCont, m_inputKey, ctx));
 
        for (const MdtCsm* csm : *csmCont) {
            std::vector<unsigned> fillMe{};
            const StIdx_t stIdx = m_idHelperSvc->stationIndex(csm->identify());
            for (unsigned  h =0 ; h < m_histos.size() ; ++h) {
                if (m_histos[h].stIdx == StIdx_t::StUnknown || 
                    m_histos[h].stIdx == stIdx) {
                    fillMe.push_back(h);
                }
                const uint16_t subID = csm->SubDetId();
                const uint16_t mrodID = csm->MrodId();
                const uint16_t csmID = csm->CsmId();
                for (unsigned h : fillMe) {
                    m_histos[h].h_subID->Fill(subID);
                    m_histos[h].h_mrodID->Fill(mrodID);
                    m_histos[h].h_csmID->Fill(csmID);
                }
                for (const MdtAmtHit* hit : *csm) {
                    const uint16_t tdcID = hit->tdcId();
                    const uint16_t chanID = hit->channelId();
                    const uint16_t coarseTime = hit->coarse();
                    const uint16_t fineTime = hit->fine();
                    const uint16_t widthComb = hit->width();
                    for (auto h : fillMe) {
                        m_histos[h].h_tdcID->Fill(tdcID);
                        m_histos[h].h_chanID->Fill(chanID);
                        m_histos[h].h_coarse->Fill(coarseTime);
                        m_histos[h].h_fine->Fill(fineTime);
                        m_histos[h].h_width->Fill(widthComb);
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}
