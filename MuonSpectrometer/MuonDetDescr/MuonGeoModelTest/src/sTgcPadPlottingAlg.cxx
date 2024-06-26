/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "sTgcPadPlottingAlg.h"

#include <cmath>
#include <sstream>
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "StoreGate/ReadCondHandle.h"

#include <TFile.h>
#include <TGraph.h>
#include <TH2F.h>

using padCorners = MuonGM::MuonPadDesign::padCorners;
using CornerArray = MuonGM::MuonPadDesign::CornerArray;

sTgcPadPlottingAlg::sTgcPadPlottingAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator) {}

StatusCode sTgcPadPlottingAlg::finalize() {
  std::unique_ptr<TFile> out_file =
      std::make_unique<TFile>(m_outFile.value().c_str(), "RECREATE");
  if (!out_file || !out_file->IsOpen() || out_file->IsZombie()) {

    ATH_MSG_FATAL("Failed to create the output file " << m_outFile);
    return StatusCode::FAILURE;
  }
  out_file->mkdir("SinglePads");  
  TDirectory* dir = out_file->GetDirectory("SinglePads");
  for (auto& [id, grObj] : m_nswPads) {
    std::stringstream ch_name{};
    dir->WriteObject(grObj.get(), padName(id).c_str());
  }
  m_nswPads.clear();
  out_file->mkdir("PadScanning");  
  dir = out_file->GetDirectory("PadScanning");
  for (auto& [id, grObj] : m_nswPadDist) {
    std::stringstream ch_name{};
    dir->WriteObject(grObj.get(), padName(id).c_str());
  }
  m_nswPads.clear();
  
  return StatusCode::SUCCESS;
}
std::string sTgcPadPlottingAlg::padName(const Identifier& id) const {
  const sTgcIdHelper& st_helper = m_idHelperSvc->stgcIdHelper();
  std::stringstream ch_name{};
  const int stEta = m_idHelperSvc->stationEta(id);
  ch_name << m_idHelperSvc->stationNameString(id);
  ch_name << std::abs(stEta)<< (stEta > 0 ? "A" : "C");
  ch_name << m_idHelperSvc->stationPhi(id);
  ch_name <<"W" << st_helper.multilayer(id);
  ch_name <<"L" << st_helper.gasGap(id);
  ch_name <<"PAD"<<st_helper.padEta(id)<<"P"<<st_helper.padPhi(id);
  return ch_name.str();
}

StatusCode sTgcPadPlottingAlg::initialize() {
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(initSTgcs());
  return StatusCode::SUCCESS;
}

StatusCode sTgcPadPlottingAlg::execute() {
  if (m_alg_run)
    return StatusCode::SUCCESS;
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_DetectorManagerKey,
                                                         ctx};
  if (!detMgr.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve the detector manager ");
    return StatusCode::FAILURE;
  }
  for (auto & [id, grObj] : m_nswPads) {
    const MuonGM::sTgcReadoutElement* roElement = detMgr->getsTgcReadoutElement(id);
    std::array<Amg::Vector2D, 4> corners{};
    if (!roElement->padCorners(id, corners)) {
        ATH_MSG_WARNING("Corners could not be extracted for pad "<<m_idHelperSvc->toString(id));
        continue;
    }
    for (const Amg::Vector2D& corner : corners) {
       grObj->SetPoint(grObj->GetN(), corner.x(), corner.y());
    }
  }
  for (auto& [id, padObj]: m_nswPadDist) {
       const MuonGM::sTgcReadoutElement* re = detMgr->getsTgcReadoutElement(id);
       const MuonGM::MuonPadDesign* design = re->getPadDesign(id);
       const int ch = m_idHelperSvc->stgcIdHelper().channel(id); 
       for (int binX = 1 ; binX <= padObj->GetNbinsX(); ++binX) {
          for (int binY = 1; binY <= padObj->GetNbinsY(); ++binY) {
              const Amg::Vector2D pos{padObj->GetXaxis()->GetBinCenter(binX),
                                      padObj->GetYaxis()->GetBinCenter(binY)};
              const Amg::Vector2D distVec = design->distanceToPad(pos, ch);
              padObj->SetBinContent(binX, binY, std::hypot(distVec.x(), distVec.y()));
          }
      }
  }
  

  return StatusCode::SUCCESS;
}

StatusCode sTgcPadPlottingAlg::initSTgcs() {
  const MuonGM::MuonDetectorManager* detMgr{nullptr};
  ATH_CHECK(detStore()->retrieve(detMgr));
  const sTgcIdHelper& id_helper = m_idHelperSvc->stgcIdHelper();
  for (const std::string station : {"STS", "STL"}) {
    for (int eta = id_helper.stationEtaMin(); eta <= id_helper.stationEtaMax(); ++eta) {
      if (eta == 0) continue;
      for (int phi = id_helper.stationPhiMin(); phi <= id_helper.stationPhiMax(); ++phi) {
        for (int ml = id_helper.multilayerMin(); ml <= id_helper.multilayerMax(); ++ml) {
          bool is_valid{false};
          Identifier station_id = id_helper.elementID(station, eta, phi, is_valid);
          if (!is_valid) continue;
          const Identifier module_id = id_helper.multilayerID(station_id, ml);
          const MuonGM::sTgcReadoutElement* re = detMgr->getsTgcReadoutElement(module_id); 
          if (!re) continue;
          
          for (int lay = 1; lay <= 4; ++lay) {
              for (int padPhi = id_helper.padPhiMin(); padPhi <= id_helper.padPhiMax(); ++padPhi){
                for (int padEta = id_helper.padEtaMin(); padEta <= id_helper.padEtaMax(); ++padEta) {
                    bool isValid{false};
                    const Identifier padId = id_helper.padID(module_id, ml, lay, sTgcIdHelper::sTgcChannelTypes::Pad, padEta, padPhi, isValid);
                    if (!isValid) continue;
                    m_nswPads[padId] = std::make_unique<TGraph>();                    
                    const MuonGM::MuonPadDesign* design = re->getPadDesign(padId);
                    CornerArray padEdges{};
                    design->channelCorners(id_helper.channel(padId), padEdges);
                    const double padMinX = std::min(padEdges[padCorners::botLeft].x(), padEdges[padCorners::topLeft].x());
                    const double padMaxX = std::max(padEdges[padCorners::botRight].x(), padEdges[padCorners::topRight].x());
                    const double padMinY = padEdges[padCorners::botLeft].y();
                    const double padMaxY = padEdges[padCorners::topLeft].y();
                    const double dX = padMaxX - padMinX;
                    const double dY = padMaxY - padMinY;
                    std::unique_ptr<TH2> padH = std::make_unique<TH2D>(padName(padId).c_str(), 
                                                                       "dummy; pad x[mm];pad y[mm];pad distance [mm]", 
                                                                       800, padMinX - 0.25*dX, padMaxX + 0.25*dX,
                                                                       800, padMinY - 0.25*dY, padMaxY + 0.25*dY);
                    ATH_MSG_DEBUG("Dimensions "<<padName(padId)<<" "<<padMinX<<" "<<padMaxX<<" -- "<<padMinY<<" "<<padMaxY);
                    m_nswPadDist[padId] = std::move(padH);
                }
              }
          }
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}
