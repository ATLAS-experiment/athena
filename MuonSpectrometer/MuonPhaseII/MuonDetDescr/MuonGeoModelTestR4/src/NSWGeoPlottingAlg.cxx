/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWGeoPlottingAlg.h"

#include <cmath>

#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "StoreGate/ReadHandle.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2I.h"


namespace MuonGMR4 {
NswGeoPlottingAlg::NswGeoPlottingAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator) {}


StatusCode NswGeoPlottingAlg::initialize() {
  ATH_CHECK(m_geoCtxKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_detMgr));
  ATH_CHECK(initMicroMega());
  return StatusCode::SUCCESS;
}
StatusCode NswGeoPlottingAlg::execute() {
  if (m_alg_run)
    return StatusCode::SUCCESS;
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<ActsGeometryContext> gctxHandle{m_geoCtxKey, ctx};
  ATH_CHECK(gctxHandle.isPresent());
  
  std::vector<const MmReadoutElement*> micromegas = m_detMgr->getAllMmReadoutElements();
  for (const MmReadoutElement* mm : micromegas) {
      for (int gasGap = 1; gasGap <= 4; ++ gasGap) {
          const IdentifierHash  hash = MmReadoutElement::createHash(gasGap, (mm->stationEta() > 0 ? 1 : 2) +
                                                                            10 * mm->multilayer());
          auto& histo = m_nswActiveAreas[hash];
          const StripDesign& design{mm->stripLayer(hash).design()};
          const double halfY = 2.*design.longHalfHeight();
          const double halfX = 2.*design.halfWidth();
          for (double x = -halfX; x <= halfX; x+= 0.25*Gaudi::Units::mm){
              for (double y = -halfY; y<= halfY; y+=0.25*Gaudi::Units::mm) {
                  Amg::Vector3D locPos{x,y,0};
                  if (!design.insideTrapezoid(locPos.block<2,1>(0,0))) continue;
                  const Amg::Vector3D globPos = mm->localToGlobalTrans(*gctxHandle, mm->layerHash(hash)) * locPos;
                  histo->Fill(globPos.x(), globPos.y());
              }
          }

      }
  }
  m_alg_run = true;
  return StatusCode::SUCCESS;
}
StatusCode NswGeoPlottingAlg::initMicroMega() {
 
  for (unsigned int ml = 1; ml <= 2; ++ml) {
    for (unsigned int active = 1; active <= 2; ++ active) {
        for (unsigned int gasGap = 1; gasGap <= 4; ++gasGap) {
          std::string histoName = "NSW_"+std::string(active == 1? "A" : "C") + "M" + 
                                  std::to_string(ml) + "G" + std::to_string(gasGap);
          auto newHisto = std::make_unique<TH2I>(histoName.c_str(),
                                                                "ActiveNSW;x [mm]; y [mm]", 1000, -5001, 5001., 1000,
                                                                -5001., 5001.);
          m_nswActiveAreas[MmReadoutElement::createHash(gasGap, active + 10 * ml)] = newHisto.get();
          ATH_CHECK(histSvc()->regHist("/GEOMODELTESTER/ActiveSurfaces/"+ histoName,std::move(newHisto)));
        }
    } 
  }
  return StatusCode::SUCCESS;
}

}
