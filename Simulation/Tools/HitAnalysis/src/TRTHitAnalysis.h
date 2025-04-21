/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITANALYSIS_TRTHITANALYSIS_H
#define HITANALYSIS_TRTHITANALYSIS_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"

class TH1;
class TH2;
class TTree;

namespace InDetDD {
  class TRT_DetectorManager;
}

class TRTHitAnalysis : public AthAlgorithm {

 public:

   TRTHitAnalysis(const std::string& name, ISvcLocator* pSvcLocator);
   ~TRTHitAnalysis() = default;

   virtual StatusCode initialize() override;
   virtual StatusCode execute() override;

 private:

   /** Some variables**/
   TH1* m_h_TRT_x;
   TH1* m_h_TRT_y;
   TH1* m_h_TRT_z;
   TH1* m_h_TRT_r;
   TH2* m_h_TRT_xy;
   TH2* m_h_TRT_zr;
   TH1* m_h_TRT_time_photons;
   TH1* m_h_TRT_time_nonphotons;
   TH1* m_h_TRT_edep_photons;
   TH1* m_h_TRT_edep_nonphotons;
   TH1* m_h_TRT_kine_photons;
   TH1* m_h_TRT_kine_nonphotons;
   TH1* m_h_TRT_barcode;

   std::vector<float>* m_TRT_x;
   std::vector<float>* m_TRT_y;
   std::vector<float>* m_TRT_z;
   std::vector<float>* m_TRT_r;
   std::vector<float>* m_TRT_time_photons;
   std::vector<float>* m_TRT_time_nonphotons;
   std::vector<float>* m_TRT_edep_photons;
   std::vector<float>* m_TRT_edep_nonphotons;
   std::vector<float>* m_TRT_kine_photons;
   std::vector<float>* m_TRT_kine_nonphotons;
   std::vector<float>* m_TRT_barcode;
   
   TTree * m_tree;
   std::string m_path;
   std::string m_ntupleFileName; 
   ServiceHandle<ITHistSvc> m_thistSvc;
   const InDetDD::TRT_DetectorManager* m_detMgr{nullptr};
};

#endif // TRT_HIT_ANALYSIS_H
