/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ZDC_HIT_ANALYSIS_H
#define ZDC_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "ZdcIdentifier/ZdcID.h"
#include "ZDC_SimEvent/ZDC_SimFiberHit_Collection.h"
#include "CaloSimEvent/CaloCalibrationHitContainer.h"


 
class ZDCHitAnalysis : public AthHistogramAlgorithm {

 public:

   using AthHistogramAlgorithm::AthHistogramAlgorithm;
   ~ZDCHitAnalysis() = default;

   virtual StatusCode initialize() override;
   virtual StatusCode execute() override;

 private:

   /** Some variables**/
   using TH1_Array = std::array<std::array<TH1*, 5>,2>;
   TH1_Array m_h_zdc_photons{};
   TH1_Array m_h_zdc_calibTot{};
   TH1_Array m_h_zdc_calibEM{};
   TH1_Array m_h_zdc_calibNonEM{};
   
   std::vector<int>* m_zdc_fiber_side{nullptr};
   std::vector<int>* m_zdc_fiber_mod{nullptr};
   std::vector<int>* m_zdc_fiber_channel{nullptr};
   std::vector<int>* m_zdc_fiber_photons{nullptr};
   
   std::vector<int>* m_zdc_calib_side{nullptr};
   std::vector<int>* m_zdc_calib_mod{nullptr};
   std::vector<int>* m_zdc_calib_channel{nullptr};
   std::vector<float>* m_zdc_calib_Total{nullptr};
   std::vector<float>* m_zdc_calib_EM{nullptr};
   std::vector<float>* m_zdc_calib_NonEM{nullptr};

   TTree * m_tree{nullptr};
   Gaudi::Property<std::string> m_path{this, "HistPath","/ZDCHitAnalysis/"};
   Gaudi::Property<std::string> m_ntupleFileName{this, "NtupleFileName","/ZDCHitAnalysis/"}; 

   SG::ReadHandleKey<ZDC_SimFiberHit_Collection> m_readKey{this, "InputKey", "ZDC_SimFiberHit_Collection"};
   SG::ReadHandleKey<CaloCalibrationHitContainer> m_readCalibKey{this, "InputCalibKey", "ZDC_CalibrationHit"};
   ZdcID *m_ZdcID{nullptr};

};

#endif // ZDC_HIT_ANALYSIS_H
