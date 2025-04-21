/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Surfaces/SurfaceBounds.hpp"

using chType = sTgcIdHelper::sTgcChannelTypes;


namespace MuonGMR4 {

StatusCode NswGeoPlottingAlg::initialize() {
  ATH_CHECK(m_geoCtxKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(detStore()->retrieve(m_detMgr));
  ATH_MSG_INFO("Check Acts surface "<<m_testActsSurf);
  ATH_CHECK(initMm());
  ATH_CHECK(initStgc());
  return StatusCode::SUCCESS;
}
StatusCode NswGeoPlottingAlg::execute() {
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const ActsGeometryContext* gctx{nullptr};
  ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

  std::vector<const MmReadoutElement*> micromegas = m_detMgr->getAllMmReadoutElements();
  for (const MmReadoutElement* mm : micromegas) {
      for (int gasGap = 1; gasGap <= 4; ++ gasGap) {
          const IdentifierHash  hash = MmReadoutElement::createHash(gasGap, (mm->stationEta() > 0 ? 1 : 2) +
                                                                            10 * mm->multilayer());
          auto histo = m_mmActiveAreas[hash];
          const StripDesign& design{mm->stripLayer(hash).design()};
          const Acts::Surface& plane{mm->surface(mm->layerHash(hash))};
          const double halfY = 2.*design.longHalfHeight();
          const double halfX = 2.*design.halfWidth();
          for (double x = -halfX; x <= halfX; x+= 1.*Gaudi::Units::mm){
              for (double y = -halfY; y<= halfY; y+=1.*Gaudi::Units::mm) {
                  const Amg::Vector3D locPos{x,y,0};
                  if (!m_testActsSurf && !design.insideTrapezoid(locPos.block<2,1>(0,0))) {
                      continue;
                  } else if (m_testActsSurf && !plane.insideBounds(locPos.block<2,1>(0,0))) {
                     continue;                     
                  }
                  
                  const Amg::Vector3D globPos = plane.transform(gctx->context()) * locPos;
                  histo->Fill(globPos.x(), globPos.y());
              }
          }
      }
  }

  std::vector<const sTgcReadoutElement*> sTgcs = m_detMgr->getAllsTgcReadoutElements();
  for (const sTgcReadoutElement* sTgc : sTgcs) {
       for (int chanType : {chType::Strip, chType::Pad, chType::Wire}){
          for (int gasGap = 1; gasGap <= 4; ++ gasGap) {
              const IdentifierHash hash = sTgcReadoutElement::createHash(gasGap, chanType, (sTgc->stationEta() > 0 ? 1 : 2) +
                                                                                          10 * sTgc->multilayer());
              auto histo = m_stgcActiveAreas[hash];
              
              const StripDesign& design{ chanType == chType::Strip? sTgc->stripDesign(hash) :
                                         chanType == chType::Wire ? static_cast<const StripDesign&>(sTgc->wireDesign(hash)) 
                                                                  : static_cast<const StripDesign&>(sTgc->padDesign(hash))};
              const Acts::Surface& plane{sTgc->surface(sTgc->layerHash(hash))};
              const double halfY = 2.*design.longHalfHeight();
              const double halfX = 2.*design.halfWidth();
              for (double x = -halfX; x <= halfX; x+= 1.*Gaudi::Units::mm){
                  for (double y = -halfY; y<= halfY; y+=1.*Gaudi::Units::mm) {
                      const Amg::Vector3D locPos{x,y,0};
                      if (!m_testActsSurf && !design.insideTrapezoid(locPos.block<2,1>(0,0))) {
                          continue;
                      } else if (m_testActsSurf && !plane.insideBounds(locPos.block<2,1>(0,0))) {
                         continue;                     
                      }
                  
                      const Amg::Vector3D globPos = plane.transform(gctx->context()) * locPos;
                      histo->Fill(globPos.x(), globPos.y());
                  }
              }
          }
      }
  }


  

  return StatusCode::SUCCESS;
}
StatusCode NswGeoPlottingAlg::initStgc() {
    for (unsigned int ml =1 ; ml <= 2; ++ml) {
        for(unsigned int active =1 ; active <= 2; ++active){
            for (int chanType : {chType::Strip, chType::Pad, chType::Wire}){
                for (unsigned int gasGap =1; gasGap <= 4; ++gasGap) {
                    std::string histoName = "STGC_"+std::string(active == 1? "A" : "C") + "M" + 
                                            std::to_string(ml) + "G" + std::to_string(gasGap) + 
                                            + (chanType == chType::Strip? "S" :
                                               chanType == chType::Wire ? "W" : "P");
                
                    auto newHisto = std::make_unique<TH2I>(histoName.c_str(),
                                                                "ActiveNSW;x [mm]; y [mm]", 1000, -5001, 5001., 1000,
                                                                -5001., 5001.);
                    m_stgcActiveAreas[sTgcReadoutElement::createHash(gasGap, chanType, active + 10 * ml)] = newHisto.get();
                    ATH_CHECK(histSvc()->regHist("/GEOMODELTESTER/ActiveSurfaces/"+ histoName,std::move(newHisto)));
                }
            }
        }
    }
    return StatusCode::SUCCESS;
}
StatusCode NswGeoPlottingAlg::initMm() {
 
  for (unsigned int ml = 1; ml <= 2; ++ml) {
    for (unsigned int active = 1; active <= 2; ++ active) {
        for (unsigned int gasGap = 1; gasGap <= 4; ++gasGap) {
          std::string histoName = "MM_"+std::string(active == 1? "A" : "C") + "M" + 
                                  std::to_string(ml) + "G" + std::to_string(gasGap);
          auto newHisto = std::make_unique<TH2I>(histoName.c_str(),
                                                                "ActiveNSW;x [mm]; y [mm]", 1000, -5001, 5001., 1000,
                                                                -5001., 5001.);
          m_mmActiveAreas[MmReadoutElement::createHash(gasGap, active + 10 * ml)] = newHisto.get();
          ATH_CHECK(histSvc()->regHist("/GEOMODELTESTER/ActiveSurfaces/"+ histoName,std::move(newHisto)));
        }
    } 
  }
  return StatusCode::SUCCESS;
}

}
