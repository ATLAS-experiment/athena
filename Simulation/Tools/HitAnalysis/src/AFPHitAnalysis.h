/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AFP_HIT_ANALYSIS_H
#define AFP_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "AFP_SimEv/AFP_SIDSimHitCollection.h"


class AFPHitAnalysis : public AthHistogramAlgorithm {

 public:
   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   ~AFPHitAnalysis() = default;

   virtual StatusCode initialize() override final;
   virtual StatusCode execute() override final;

 private:

   /** Some histograms**/
   TH1*   m_h_hitID{nullptr};
   TH1*   m_h_pdgID{nullptr};
   TH1*   m_h_trackID{nullptr};
   TH1*   m_h_kine{nullptr};
   TH1*   m_h_edep{nullptr};
   TH1*   m_h_stepX{nullptr};
   TH1*   m_h_stepY{nullptr};
   TH1*   m_h_stepZ{nullptr};
   TH1*   m_h_time{nullptr};
   TH1*   m_h_stationID{nullptr};
   TH1*   m_h_detID{nullptr};
   TH1*   m_h_pixelRow{nullptr};
   TH1*   m_h_pixelCol{nullptr};

   std::vector<float>*   m_hitID{nullptr};
   std::vector<float>*   m_pdgID{nullptr};
   std::vector<float>*   m_trackID{nullptr};
   std::vector<float>*   m_kine{nullptr};
   std::vector<float>*   m_edep{nullptr};
   std::vector<float>*   m_stepX{nullptr};
   std::vector<float>*   m_stepY{nullptr};
   std::vector<float>*   m_stepZ{nullptr};
   std::vector<float>*   m_time{nullptr};
   std::vector<int>*   m_stationID{nullptr};
   std::vector<int>*   m_detID{nullptr};
   std::vector<int>*   m_pixelRow{nullptr};
   std::vector<int>*   m_pixelCol{nullptr};
   
   TTree * m_tree{nullptr};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName", "/AFPHitAnalysis/" };
   Gaudi::Property<std::string> m_path{this, "HistPath", "/AFPHitAnalysis/"}; 
   SG::ReadHandleKey<AFP_SIDSimHitCollection> m_readKey{this, "InpuKey", "AFP_SIDSimHitCollection"};

  

};

#endif // AFP_HIT_ANALYSIS_H
