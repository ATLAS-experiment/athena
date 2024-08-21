/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_HitEfficiency.cxx
 * @author nora pettersson
 **/

#include "InDetPerfPlot_HitEfficiency.h"
#include "InDetPhysHitDecoratorAlg.h"
#include "xAODTracking/TrackParticle.h"
#include "AthContainers/ConstAccessor.h"
#include "TProfile.h"
#include "TEfficiency.h"
#include <vector>
#include <cmath> // for std::fabs




InDetPerfPlot_HitEfficiency::InDetPerfPlot_HitEfficiency(InDetPlotBase* pParent, const std::string& sDir, bool isITk)  :
  InDetPlotBase(pParent, sDir), m_isITk{isITk}, m_HitEfficiencyVsEta{}, m_debug{false} {
  //
}

void
InDetPerfPlot_HitEfficiency::initializePlots() {
  // const bool prependDirectory(false);
  // eff plots for L0PIXBARR, PIXEL, SCT, TRT
  // Barrel
  book(m_HitEfficiencyVsEta[L0PIXBARR][BARREL], "eff_hit_vs_eta_l0pix_barrel");
  book(m_HitEfficiencyVsEta[PIXEL][BARREL], "eff_hit_vs_eta_pix_barrel");
  book(m_HitEfficiencyVsEta[SCT][BARREL], "eff_hit_vs_eta_sct_barrel");
  if(!m_isITk) book(m_HitEfficiencyVsEta[TRT][BARREL], "eff_hit_vs_eta_trt_barrel");

  book(m_HitEfficiencyVsEta[PIXEL][ENDCAP], "eff_hit_vs_eta_pix_endcap");
  book(m_HitEfficiencyVsEta[SCT][ENDCAP], "eff_hit_vs_eta_sct_endcap");
  if(!m_isITk) book(m_HitEfficiencyVsEta[TRT][ENDCAP], "eff_hit_vs_eta_trt_endcap");
}

void
InDetPerfPlot_HitEfficiency::fill(const xAOD::TrackParticle& trkprt, float weight) {
  if (m_debug) {
    ATH_MSG_INFO("Filling HitEfficiency");
  }

  static const SG::ConstAccessor< std::vector<int> > measurement_regionAcc("measurement_region");
  const bool hitDetailsAvailable = measurement_regionAcc.isAvailable(trkprt);
  if (!hitDetailsAvailable) {
    if (m_warnCount++ < 10) {
      ATH_MSG_WARNING("The HitEff plots dont see any data (note: only 10 warnings issued)");
    }
  } else {
    static const SG::ConstAccessor< std::vector<int> > measurement_detAcc("measurement_det");
    const std::vector<int>& result_det = measurement_detAcc(trkprt);
    if (!result_det.empty()) {
      static const SG::ConstAccessor< std::vector<int> > measurement_typeAcc("measurement_type");
      const std::vector<int>& result_measureType = measurement_typeAcc(trkprt);
      const std::vector<int>& result_region = measurement_regionAcc(trkprt);
      // NP: this should be fine... residual filled with -1 if not hit

      for (unsigned int idx = 0; idx < result_region.size(); ++idx) {
        const int measureType = result_measureType[idx];
        const bool isHit((measureType == 0)or(measureType == 4));
        const int det = result_det[idx]; // LAYER TYPE L0PIXBARR / PIXEL / ...
        const int region = result_region[idx]; // BARREL OR ENDCAP
        float eta = std::fabs(trkprt.eta());
        
        fillHisto(m_HitEfficiencyVsEta[det][region], eta, isHit, weight);
      }
    }
  }
}
