/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetPerfPlot_HitResidual.cxx
 * @author shaun roe
 **/

#include "InDetPerfPlot_HitResidual.h"
#include "AthContainers/ConstAccessor.h"

using namespace TMath;

InDetPerfPlot_HitResidual::InDetPerfPlot_HitResidual(InDetPlotBase* pParent, const std::string& sDir)  : InDetPlotBase(
    pParent, sDir) {
//
}

void
InDetPerfPlot_HitResidual::initializePlots() {
  // const bool prependDirectory(false);
  // x residuals
  book(m_residualx.at(L0PIXBARR).at(BARREL), "residualx_l0pix_barrel");
  book(m_residualx_1hit.at(L0PIXBARR).at(BARREL), "residualx_l0pix_barrel_1hit");
  book(m_residualx_2ormorehits.at(L0PIXBARR).at(BARREL), "residualx_l0pix_barrel_2ormorehits");
  //
  book(m_residualx.at(PIXEL).at(BARREL), "residualx_pixel_barrel");
  book(m_residualx_1hit.at(PIXEL).at(BARREL), "residualx_pixel_barrel_1hit");
  book(m_residualx_2ormorehits.at(PIXEL).at(BARREL), "residualx_pixel_barrel_2ormorehits");
  //
  book(m_residualx.at(SCT).at(BARREL), "residualx_sct_barrel");
  book(m_residualx_1hit.at(SCT).at(BARREL), "residualx_sct_barrel_1hit");
  book(m_residualx_2ormorehits.at(SCT).at(BARREL), "residualx_sct_barrel_2ormorehits");
  //
  book(m_residualx.at(TRT).at(BARREL), "residualx_trt_barrel");
  // ..now endcaps
  book(m_residualx.at(PIXEL).at(ENDCAP), "residualx_pixel_endcap");
  book(m_residualx_1hit.at(PIXEL).at(ENDCAP), "residualx_pixel_endcap_1hit");
  book(m_residualx_2ormorehits.at(PIXEL).at(ENDCAP), "residualx_pixel_endcap_2ormorehits");
  //
  book(m_residualx.at(SCT).at(ENDCAP), "residualx_sct_endcap");
  book(m_residualx_1hit.at(SCT).at(ENDCAP), "residualx_sct_endcap_1hit");
  book(m_residualx_2ormorehits.at(SCT).at(ENDCAP), "residualx_sct_endcap_2ormorehits");
  //
  book(m_residualx.at(TRT).at(ENDCAP), "residualx_trt_endcap");
  //

  // y residuals
  book(m_residualy.at(L0PIXBARR).at(BARREL), "residualy_l0pix_barrel");
  book(m_residualy_1hit.at(L0PIXBARR).at(BARREL), "residualy_l0pix_barrel_1hit");
  book(m_residualy_2ormorehits.at(L0PIXBARR).at(BARREL), "residualy_l0pix_barrel_2ormorehits");
  //
  book(m_residualy.at(PIXEL).at(BARREL), "residualy_pixel_barrel");
  book(m_residualy_1hit.at(PIXEL).at(BARREL), "residualy_pixel_barrel_1hit");
  book(m_residualy_2ormorehits.at(PIXEL).at(BARREL), "residualy_pixel_barrel_2ormorehits");
  //
  // SCT and TRT do not have y-residuals/pulls
  // ..now endcaps
  book(m_residualy.at(PIXEL).at(ENDCAP), "residualy_pixel_endcap");
  book(m_residualy_1hit.at(PIXEL).at(ENDCAP), "residualy_pixel_endcap_1hit");
  book(m_residualy_2ormorehits.at(PIXEL).at(ENDCAP), "residualy_pixel_endcap_2ormorehits");
  //
  // SCT and TRT do not have y-residuals/pulls
  // pulls
  // barrel
  book(m_pullx.at(L0PIXBARR).at(BARREL), "pullx_l0pix_barrel");
  book(m_pullx.at(PIXEL).at(BARREL), "pullx_pixel_barrel");
  book(m_pullx.at(SCT).at(BARREL), "pullx_sct_barrel");
  book(m_pullx.at(TRT).at(BARREL), "pullx_trt_barrel");
  //
  book(m_pullx.at(PIXEL).at(ENDCAP), "pullx_pixel_endcap");
  book(m_pullx.at(SCT).at(ENDCAP), "pullx_sct_endcap");
  book(m_pullx.at(TRT).at(ENDCAP), "pullx_trt_endcap");
  //
  // barrel
  book(m_pully.at(L0PIXBARR).at(BARREL), "pully_l0pix_barrel");
  book(m_pully.at(PIXEL).at(BARREL), "pully_pixel_barrel");
  //
  // SCT and TRT do not have y-residuals/pulls
  book(m_pully.at(PIXEL).at(ENDCAP), "pully_pixel_endcap");
  //
  ////SCT and TRT do not have y-residuals/pulls
  // introduce cluster width histograms
  book(m_etaWidth.at(PIXEL).at(BARREL), "clusterEtaWidth_pixel_barrel");
  book(m_etaWidth.at(PIXEL).at(ENDCAP), "clusterEtaWidth_pixel_endcap");
  book(m_phiWidth.at(PIXEL).at(BARREL), "clusterPhiWidth_pixel_barrel");
  book(m_phiWidth.at(PIXEL).at(ENDCAP), "clusterPhiWidth_pixel_endcap");
  //
  book(m_phiWidth.at(SCT).at(BARREL), "clusterPhiWidth_sct_barrel");
  book(m_phiWidth.at(SCT).at(ENDCAP), "clusterPhiWidth_sct_endcap");

  book(m_phiWidthEta.at(PIXEL), "clusterPhiWidth_eta_pixel");
  book(m_etaWidthEta.at(PIXEL), "clusterEtaWidth_eta_pixel");
  //
  book(m_phiWidthEta.at(SCT), "clusterPhiWidth_eta_sct");

  // additional histograms booked if high detail level is specified
  // all additional histograms for residuals, pull and cluster size vs eta
  if(m_iDetailLevel > 200) {
    book(m_residualx_eta.at(L0PIXBARR).at(BARREL), "residualx_l0pix_barrel_eta");
    book(m_residualx_eta.at(PIXEL).at(BARREL), "residualx_pixel_barrel_eta");
    book(m_residualx_eta.at(SCT).at(BARREL), "residualx_sct_barrel_eta");
    book(m_residualx_eta.at(PIXEL).at(ENDCAP), "residualx_pixel_endcap_eta");
    book(m_residualx_eta.at(SCT).at(ENDCAP), "residualx_sct_endcap_eta");
    book(m_residualy_eta.at(L0PIXBARR).at(BARREL), "residualy_l0pix_barrel_eta");
    book(m_residualy_eta.at(PIXEL).at(BARREL), "residualy_pixel_barrel_eta");
    book(m_residualy_eta.at(PIXEL).at(ENDCAP), "residualy_pixel_endcap_eta");
    book(m_pullx_eta.at(L0PIXBARR).at(BARREL), "pullx_l0pix_barrel_eta");
    book(m_pullx_eta.at(PIXEL).at(BARREL), "pullx_pixel_barrel_eta");
    book(m_pullx_eta.at(SCT).at(BARREL), "pullx_sct_barrel_eta");
    book(m_pullx_eta.at(PIXEL).at(ENDCAP), "pullx_pixel_endcap_eta");
    book(m_pullx_eta.at(SCT).at(ENDCAP), "pullx_sct_endcap_eta");
    book(m_pully_eta.at(L0PIXBARR).at(BARREL), "pully_l0pix_barrel_eta");
    book(m_pully_eta.at(PIXEL).at(BARREL), "pully_pixel_barrel_eta");
    book(m_pully_eta.at(PIXEL).at(ENDCAP), "pully_pixel_endcap_eta");
    book(m_phiWidth_eta.at(L0PIXBARR).at(BARREL), "clusterPhiWidth_l0pix_barrel_eta");
    book(m_phiWidth_eta.at(PIXEL).at(BARREL), "clusterPhiWidth_pixel_barrel_eta");
    book(m_phiWidth_eta.at(SCT).at(BARREL), "clusterPhiWidth_sct_barrel_eta");
    book(m_phiWidth_eta.at(PIXEL).at(ENDCAP), "clusterPhiWidth_pixel_endcap_eta");
    book(m_phiWidth_eta.at(SCT).at(ENDCAP), "clusterPhiWidth_sct_endcap_eta");
    book(m_etaWidth_eta.at(L0PIXBARR).at(BARREL), "clusterEtaWidth_l0pix_barrel_eta");
    book(m_etaWidth_eta.at(PIXEL).at(BARREL), "clusterEtaWidth_pixel_barrel_eta");
    book(m_etaWidth_eta.at(PIXEL).at(ENDCAP), "clusterEtaWidth_pixel_endcap_eta");
  }
}


void
InDetPerfPlot_HitResidual::fill(const xAOD::TrackParticle& trkprt, float weight) {
  static const SG::ConstAccessor<std::vector<int> >
    measurement_regionAcc("measurement_region");
  const static bool hitDetailsAvailable = measurement_regionAcc.isAvailable(trkprt);

  if (!hitDetailsAvailable) {
    if (m_warnCount++ < 10) {
      ATH_MSG_WARNING("The hit res plots dont see any data (note:only 10 warnings issued)");
    }
  } else {
    static const SG::ConstAccessor< std::vector<int> > measurement_detAcc("measurement_det");
    const std::vector<int>& result_det = measurement_detAcc(trkprt);

    if (!result_det.empty()) {
      static const SG::ConstAccessor< std::vector<int> > measurement_typeAcc("measurement_type");
      static const SG::ConstAccessor< std::vector<int> > measurement_regionAcc("measurement_region");
      static const SG::ConstAccessor< std::vector<float> > hitResiduals_residualLocXAcc("hitResiduals_residualLocX");
      static const SG::ConstAccessor< std::vector<float> > hitResiduals_pullLocXAcc("hitResiduals_pullLocX");
      static const SG::ConstAccessor< std::vector<float> > hitResiduals_residualLocYAcc("hitResiduals_residualLocY");
      static const SG::ConstAccessor< std::vector<float> > hitResiduals_pullLocYAcc("hitResiduals_pullLocY");
      static const SG::ConstAccessor< std::vector<int> > hitResiduals_phiWidthAcc("hitResiduals_phiWidth");
      static const SG::ConstAccessor< std::vector<int> > hitResiduals_etaWidthAcc("hitResiduals_etaWidth");

      const std::vector<int>& result_measureType = measurement_typeAcc(trkprt);
      const std::vector<int>& result_region = measurement_regionAcc(trkprt);
      const std::vector<float>& result_residualLocX = hitResiduals_residualLocXAcc(trkprt);
      const std::vector<float>& result_pullLocX = hitResiduals_pullLocXAcc(trkprt);
      const std::vector<float>& result_residualLocY = hitResiduals_residualLocYAcc(trkprt);
      const std::vector<float>& result_pullLocY = hitResiduals_pullLocYAcc(trkprt);
      const std::vector<int>& result_phiWidth = hitResiduals_phiWidthAcc(trkprt);
      const std::vector<int>& result_etaWidth = hitResiduals_etaWidthAcc(trkprt);

      const float eta = trkprt.eta();

      // NP: this should be fine... resiudal filled with -1 if not hit
      if (result_det.size() != result_residualLocX.size()) {
        ATH_MSG_WARNING("Vectors of results are not matched in size!");
      }
      const auto resultSize = result_region.size();
      for (unsigned int idx = 0; idx < resultSize; ++idx) {
        const int measureType = result_measureType[idx];
        if (measureType != 4) {
          continue; // NP: Only use unbiased hits for the hit residuals ;)
        }
        const int det = result_det[idx];
        const int region = result_region[idx];
        const int width = result_phiWidth[idx];
        const int etaWidth = result_etaWidth[idx];
        const float residualLocX = result_residualLocX[idx];
        const float pullLocX = result_pullLocX[idx];
        const float residualLocY = result_residualLocY[idx];
        const float pullLocY = result_pullLocY[idx];
        if ((det == INVALID_DETECTOR)or(region == INVALID_REGION)) {
          continue;
        }
        if ((width > 0) or (det ==TRT)){//TRT does not have defined cluster width 
          // introduce cluster width histograms
          fillHisto(m_phiWidth.at(det).at(region), width, weight);
          fillHisto(m_etaWidth.at(det).at(region), etaWidth, weight);

          // cluster width eta profiles
          fillHisto(m_phiWidthEta.at(det), eta, width, weight);
          fillHisto(m_etaWidthEta.at(det), eta, etaWidth, weight);

          fillHisto(m_residualx.at(det).at(region), residualLocX, weight);

          if(m_iDetailLevel > 200) {
            fillHisto(m_phiWidth_eta.at(det).at(region), eta, width, weight);
            fillHisto(m_etaWidth_eta.at(det).at(region), eta, etaWidth, weight);
            fillHisto(m_residualx_eta.at(det).at(region), eta, residualLocX, weight);
          }

          const bool hasYCoordinate = (det != SCT)and(det != TRT); // SCT & TRT do not have LocY
          fillHisto(m_pullx.at(det).at(region), pullLocX, weight);
          if(m_iDetailLevel > 200)
            fillHisto(m_pullx_eta.at(det).at(region), eta, pullLocX, weight);

          // SCT & TRT do not have LocY
          if (hasYCoordinate) {
            fillHisto(m_residualy.at(det).at(region), residualLocY, weight);
            fillHisto(m_pully.at(det).at(region), pullLocY, weight);
            if(m_iDetailLevel > 200) {
              fillHisto(m_residualy_eta.at(det).at(region), eta, residualLocY, weight);
              fillHisto(m_pully_eta.at(det).at(region), eta, pullLocY, weight);
            }
          }
          if ((det == TRT) or (width < 0)) {
            continue;
          }
          if (width == 1) {
            fillHisto(m_residualx_1hit.at(det).at(region), residualLocX, weight);
            if (hasYCoordinate) {
              fillHisto(m_residualy_1hit.at(det).at(region), residualLocY, weight);
            }
          } else {
            fillHisto(m_residualx_2ormorehits.at(det).at(region), residualLocX, weight);
            if (hasYCoordinate) {
              fillHisto(m_residualy_2ormorehits.at(det).at(region), residualLocY, weight);
            }
          }
        }
      }
    }
  }
}
