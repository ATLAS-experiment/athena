/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWGeoPlottingAlg.h"

#include <cmath>

#include "GaudiKernel/SystemOfUnits.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "StoreGate/ReadCondHandle.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2D.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/Surface.h"

namespace {
std::string to_string(const Amg::Vector3D& v) {
  std::stringstream sstr{};
  sstr << "[x,y,z]=(" << v.x() << "," << v.y() << "," << v.z()
       << ") [theta/eta/phi]=(" << (v.theta() / Gaudi::Units::degree) << ","
       << v.eta() << "," << v.phi() << ")";
  return sstr.str();
}

}  // namespace

NSWGeoPlottingAlg::NSWGeoPlottingAlg(const std::string& name,
                                     ISvcLocator* pSvcLocator)
    : AthHistogramAlgorithm(name, pSvcLocator) {}
StatusCode NSWGeoPlottingAlg::finalize() {
  std::unique_ptr<TFile> out_file =
      std::make_unique<TFile>(m_outFile.value().c_str(), "RECREATE");
  if (!out_file || !out_file->IsOpen() || out_file->IsZombie()) {

    ATH_MSG_FATAL("Failed to create the output file " << m_outFile);
    return StatusCode::FAILURE;
  }
  out_file->mkdir("SinglePads");
  out_file->mkdir("ActiveSurfaces");
  const MmIdHelper& mm_helper = m_idHelperSvc->mmIdHelper();
  const sTgcIdHelper& st_helper = m_idHelperSvc->stgcIdHelper();
  for (auto& id_graph : m_nswPads) {
    std::unique_ptr<TGraph>& graph = id_graph.second;
    const Identifier& id = id_graph.first;
    bool is_mm = m_idHelperSvc->isMM(id);
    const int stEta = m_idHelperSvc->stationEta(id);
    const int ml = is_mm ? mm_helper.multilayer(id) : st_helper.multilayer(id);
    const int lay = is_mm ? mm_helper.gasGap(id) : st_helper.gasGap(id);
    const std::string ch_name =
        (is_mm ? mm_helper.stationNameString(m_idHelperSvc->stationName(id))
               : st_helper.stationNameString(m_idHelperSvc->stationName(id))) +
        std::to_string(std::abs(stEta)) + (stEta > 0 ? "A" : "C") +
        std::to_string(m_idHelperSvc->stationPhi(id)) + "W" +
        std::to_string(ml) + "L" + std::to_string(lay);
    TDirectory* dir = out_file->GetDirectory("SinglePads");
    dir->WriteObject(graph.get(), ch_name.c_str());
    graph.reset();
    const int signed_lay = layerId(id);
    std::unique_ptr<TGraph>& lay_graph = m_nswLayers[signed_lay];
    if (!lay_graph)
      continue;
    std::string lay_name = std::string{is_mm ? "MMG" : "STG"} + "W" +
                           std::to_string(ml) + (stEta > 0 ? "A" : "C") +
                           std::to_string(lay);
    out_file->WriteObject(lay_graph.get(), lay_name.c_str());
    lay_graph.reset();
    std::unique_ptr<TH1>& active_area = m_nswActiveAreas[signed_lay];
    if (!active_area)
      continue;
    dir = out_file->GetDirectory("ActiveSurfaces");

    dir->WriteObject(active_area.get(), lay_name.c_str());
    active_area.reset();
  }
  return StatusCode::SUCCESS;
}
StatusCode NSWGeoPlottingAlg::initialize() {
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(initMicroMega());
  ATH_CHECK(initSTgcs());
  return StatusCode::SUCCESS;
}
StatusCode NSWGeoPlottingAlg::execute() {
  if (m_alg_run)
    return StatusCode::SUCCESS;
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_DetectorManagerKey,
                                                         ctx};
  if (!detMgr.isValid()) {
    ATH_MSG_FATAL("Failed to retrieve the detector manager ");
    return StatusCode::FAILURE;
  }

  for (auto& id_graph : m_nswPads) {
    const Identifier& id = id_graph.first;
    std::unique_ptr<TGraph>& pad_graph = id_graph.second;
    const bool is_mm = m_idHelperSvc->isMM(id);
    const int signed_lay = layerId(id);
    std::unique_ptr<TGraph>& wheel_graph = m_nswLayers[signed_lay];

    const MuonGM::MMReadoutElement* mm_roe =
        is_mm ? detMgr->getMMReadoutElement(id) : nullptr;
    const MuonGM::sTgcReadoutElement* st_roe =
        is_mm ? nullptr : detMgr->getsTgcReadoutElement(id);

    const MuonGM::MuonChannelDesign* design =
        is_mm ? mm_roe->getDesign(id) : st_roe->getDesign(id);

    const Trk::TrkDetElementBase* roe =
        (mm_roe ? static_cast<const Trk::TrkDetElementBase*>(mm_roe)
                : static_cast<const Trk::TrkDetElementBase*>(st_roe));
    auto fill_graphs = [&](const Amg::Vector2D& locPos, int strip) {
      if ((is_mm || std::abs(locPos.y()) < 1.e-3) &&
          design->channelNumber(locPos) != strip) {
        ATH_MSG_ALWAYS(" Backmapping of the strip number did not work for "
                       << m_idHelperSvc->toString(id) << " local pos: "
                       << locPos.x() << " " << locPos.y() << " "
                       << " " << design->channelNumber(locPos) << " vs. "
                       << strip);
        return false;
      }

      Amg::Vector3D globPos{Amg::Vector3D::Zero()};
      roe->surface(id).localToGlobal(locPos, Amg::Vector3D::Zero(), globPos);
      if (pad_graph)
        pad_graph->SetPoint(pad_graph->GetN(), globPos.x(), globPos.y());
      if (wheel_graph)
        wheel_graph->SetPoint(wheel_graph->GetN(), globPos.x(), globPos.y());

      return true;
    };
    const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
    auto global_points = [&](const Identifier& id, Amg::Vector3D& left,
                             Amg::Vector3D& center, Amg::Vector3D& right) {
      Amg::Vector2D l_cen{Amg::Vector2D::Zero()}, l_left{Amg::Vector2D::Zero()},
          l_right{Amg::Vector2D::Zero()};
      const MuonGM::MuonChannelDesign* design = nullptr;
      if (mm_roe)
        design = mm_roe->getDesign(id);
      else if (st_roe)
        design = st_roe->getDesign(id);
      const int chan = id_helper.channel(id);
      design->leftEdge(chan, l_left);
      design->center(chan, l_cen);
      design->rightEdge(chan, l_right);

      roe->surface(id).localToGlobal(l_left, Amg::Vector3D::Zero(), left);
      roe->surface(id).localToGlobal(l_cen, Amg::Vector3D::Zero(), center);
      roe->surface(id).localToGlobal(l_right, Amg::Vector3D::Zero(), right);
    };

    const int n_strips =
        (is_mm ? mm_roe->numberOfStrips(id) : st_roe->numberOfStrips(id));
    for (int strip = design->numberOfMissingBottomStrips() + 1;
         strip <= n_strips; strip += 1) {
      {
        Amg::Vector2D locPos{Amg::Vector2D::Zero()};
        if (design->leftEdge(strip, locPos) && !fill_graphs(locPos, strip))
          ATH_MSG_DEBUG("left edge -- channel " << m_idHelperSvc->toString(id)
                                                << " does not have strip "
                                                << strip << "... Why?");
      }
      {
        Amg::Vector2D locPos{Amg::Vector2D::Zero()};
        if (design->center(strip, locPos) && !fill_graphs(locPos, strip))
          ATH_MSG_DEBUG("center -- channel " << m_idHelperSvc->toString(id)
                                             << " does not have strip " << strip
                                             << "... Why?");
      }
      {
        Amg::Vector2D locPos{Amg::Vector2D::Zero()};
        if (design->rightEdge(strip, locPos) && !fill_graphs(locPos, strip))
          ATH_MSG_DEBUG("right edge -- channel " << m_idHelperSvc->toString(id)
                                                 << " does not have strip "
                                                 << strip << "... Why?");
      }
    }
    auto uv_intersects = [&]() {
      /// Calculate the intersections in a dedicated loop
      if (design->hasStereoAngle() || m_idHelperSvc->issTgc(id))
        return StatusCode::SUCCESS;
      const int gap = id_helper.gasGap(id);
      const int ml = id_helper.multilayer(id);
      if ((ml == 1 && gap == 2) || (ml == 2 && gap == 4))
        return StatusCode::SUCCESS;

      for (int strip = design->numberOfMissingBottomStrips() + 1;
           strip <= n_strips; strip += 1) {
        const int u_gap = ml == 1 ? 3 : 1;
        const int v_gap = ml == 1 ? 4 : 2;
        const Identifier x_id = id_helper.channelID(id, ml, gap, strip);
        const Identifier u_id = id_helper.channelID(id, ml, u_gap, strip);
        const Identifier v_id = id_helper.channelID(id, ml, v_gap, strip);
        Amg::Vector3D x_center{Amg::Vector3D::Zero()},
            u_center{Amg::Vector3D::Zero()}, v_center{Amg::Vector3D::Zero()};
        Amg::Vector3D x_left{Amg::Vector3D::Zero()},
            u_left{Amg::Vector3D::Zero()}, v_left{Amg::Vector3D::Zero()};
        Amg::Vector3D x_right{Amg::Vector3D::Zero()},
            u_right{Amg::Vector3D::Zero()}, v_right{Amg::Vector3D::Zero()};

        global_points(x_id, x_left, x_center, x_right);
        global_points(u_id, u_left, u_center, u_right);
        global_points(v_id, v_left, v_center, v_right);

        const Amg::Vector3D x_dir = (x_right - x_left).unit();
        const Amg::Vector3D v_dir = (v_left - v_right).unit();
        const Amg::Vector3D u_dir = (u_left - u_right).unit();

        std::optional<double> uv_isect =
            Amg::intersect<3>(v_center, v_dir, u_center, u_dir);

        if (!uv_isect) {
          ATH_MSG_ERROR("Failed to intersect the uv strips for identifiers "
                        << std::endl
                        << " *** " << m_idHelperSvc->toString(u_id) << " "
                        << to_string(u_dir) << std::endl
                        << " *** " << m_idHelperSvc->toString(v_id) << " "
                        << to_string(v_dir));
          return StatusCode::FAILURE;
        }
        const Amg::Vector3D uv_ipoint = u_center + (*uv_isect) * u_dir;
        const Amg::Vector2D cen_diff = (uv_ipoint - x_center).block<2, 1>(0, 0);
        if (cen_diff.dot(cen_diff) > std::numeric_limits<float>::epsilon()) {
          ATH_MSG_ERROR("Expect that the uv strips "
                        << std::endl
                        << " *** " << m_idHelperSvc->toString(u_id) << " "
                        << to_string(u_dir) << std::endl
                        << " *** " << m_idHelperSvc->toString(v_id) << " "
                        << to_string(v_dir)
                        << " intersect at the center of the corresponding x "
                           "strip. But they don't. "
                        << std::endl
                        << to_string(uv_ipoint) << std::endl
                        << " vs." << std::endl
                        << to_string(x_center));
        }
        ATH_MSG_DEBUG("Intersection of uv is in " << to_string(uv_ipoint) << " "
                                                  << to_string(x_center));

        std::optional<double> ux_isect =
            Amg::intersect<3>(u_center, u_dir, x_center, x_dir);
        if (!ux_isect) {
          ATH_MSG_ERROR("Failed to intersect the ux strips for identifiers "
                        << std::endl
                        << " *** " << m_idHelperSvc->toString(v_id) << " "
                        << to_string(u_dir) << std::endl
                        << " *** " << m_idHelperSvc->toString(x_id) << " "
                        << to_string(x_dir));
          return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG("Intersection of xu is in "
                      << to_string(x_center + (*ux_isect) * x_dir) << " "
                      << to_string(x_center));

        std::optional<double> vx_isect =
            Amg::intersect<3>(v_center, v_dir, x_center, x_dir);
        if (!ux_isect) {
          ATH_MSG_ERROR("Failed to intersect the vx strips for identifiers "
                        << std::endl
                        << " *** " << m_idHelperSvc->toString(v_id) << " "
                        << to_string(v_dir) << std::endl
                        << " *** " << m_idHelperSvc->toString(x_id) << " "
                        << to_string(x_dir));
          return StatusCode::FAILURE;
        }
        ATH_MSG_DEBUG("Intersection of vu is in "
                      << to_string(x_center + (*vx_isect) * x_dir) << " "
                      << to_string(x_center));
      }
      return StatusCode::SUCCESS;
    };
    if (false)
      ATH_CHECK(uv_intersects());

    std::unique_ptr<TH1>& surface_histo = m_nswActiveAreas[signed_lay];
    if (!surface_histo) {
      ATH_MSG_WARNING("No surface histo has been made for "
                      << m_idHelperSvc->toString(id) << " " << signed_lay);
      continue;
    }
    const Amg::Vector3D& surf_cent = roe->center(id);
    double d = std::max(std::max(design->xSize(), design->maxYSize()),
                        design->minYSize());
    for (double x = surf_cent.x() - d; x <= surf_cent.x() + d; x += 1) {
      for (double y = surf_cent.y() - d; y <= surf_cent.y() + d; y += 1) {
        const Amg::Vector3D glob_pos{x, y, surf_cent.z()};
        Amg::Vector2D lpos{Amg::Vector2D::Zero()};
        if (!roe->surface(id).globalToLocal(glob_pos, glob_pos, lpos))
          continue;
        if ((is_mm && mm_roe->insideActiveBounds(id, lpos, 10., 10.)) ||
            (!is_mm && st_roe->surface(id).insideBounds(lpos, 10., 10.))

        ) {
          surface_histo->Fill(x, y);
        }
      }
    }
  }
  m_alg_run = true;
  return StatusCode::SUCCESS;
}
StatusCode NSWGeoPlottingAlg::initMicroMega() {
  const MuonGM::MuonDetectorManager* detMgr{nullptr};
  ATH_CHECK(detStore()->retrieve(detMgr));
  const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
  for (const std::string station : {"MML", "MMS"}) {
      for (int phi = id_helper.stationPhiMin();
           phi <= id_helper.stationPhiMax(); ++phi) {
        for (int eta = -2; eta <= 2; ++eta) {
          if (eta == 0)
            continue;
          for (int ml = id_helper.multilayerMin();
               ml <= id_helper.multilayerMax(); ++ml) {
            bool is_valid{false};
            const Identifier station_id =
                id_helper.elementID(station, eta, phi, is_valid);
            if (!is_valid)
              continue;
            const Identifier module_id = id_helper.multilayerID(station_id, ml);
            if (!detMgr->getMMReadoutElement(module_id))
              continue;
            for (int i_layer = 1; i_layer <= 4; ++i_layer) {
              const Identifier id =
                  id_helper.channelID(module_id, ml, i_layer, 10);
              m_nswPads[id] = std::make_unique<TGraph>();
              /// Add a graph showing the complete NSW
              int signed_layer = layerId(id);
              if (!m_nswLayers[signed_layer]) {
                m_nswLayers[signed_layer] = std::make_unique<TGraph>();
              }
              if (!m_nswActiveAreas[signed_layer]) {
                m_nswActiveAreas[signed_layer] = std::make_unique<TH2D>(
                    std::to_string(m_nswActiveAreas.size()).c_str(),
                    "ActiveNSW;x [mm]; y [mm]", 1000, -5001, 5001., 1000,
                    -5001., 5001.);
              }
            }
          }
        }
      }    
  }
  return StatusCode::SUCCESS;
}
int NSWGeoPlottingAlg::layerId(const Identifier& id) const {
  int eta = m_idHelperSvc->stationEta(id);
  if (m_idHelperSvc->issTgc(id)) {
    const sTgcIdHelper& id_helper = m_idHelperSvc->stgcIdHelper();
    int ml = id_helper.multilayer(id);
    int lay = id_helper.gasGap(id);
    int type = id_helper.channelType(id);
    return (8 * (type == sTgcIdHelper::sTgcChannelTypes::Strip) + 4 * (ml - 1) +
            (lay - 1)) *
           (eta > 0 ? 1 : -1);
  }
  if (m_idHelperSvc->isMM(id)) {
    const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
    int ml = id_helper.multilayer(id);
    int lay = id_helper.gasGap(id);
    return (16 + 4 * (ml - 1) + (lay - 1)) * (eta > 0 ? 1 : -1);
  }
  return -666;
}
StatusCode NSWGeoPlottingAlg::initSTgcs() {
  const MuonGM::MuonDetectorManager* detMgr{nullptr};
  ATH_CHECK(detStore()->retrieve(detMgr));
  const sTgcIdHelper& id_helper = m_idHelperSvc->stgcIdHelper();
  for (const std::string station : {"STS", "STL"}) {
    for (int eta = id_helper.stationEtaMin(); eta <= id_helper.stationEtaMax();
         ++eta) {
      if (eta == 0)
        continue;
      for (int phi = id_helper.stationPhiMin();
           phi <= id_helper.stationPhiMax(); ++phi) {
        for (int ml = id_helper.multilayerMin();
             ml <= id_helper.multilayerMax(); ++ml) {
          bool is_valid{false};
          Identifier station_id =
              id_helper.elementID(station, eta, phi, is_valid);
          if (!is_valid)
            continue;
          const Identifier module_id = id_helper.multilayerID(station_id, ml);
          if (!detMgr->getsTgcReadoutElement(module_id))
            continue;
          for (int lay = 1; lay <= 4; ++lay) {
            const Identifier strip_id = id_helper.channelID(
                module_id, ml, lay, sTgcIdHelper::sTgcChannelTypes::Strip, 10);
            const Identifier wire_id = id_helper.channelID(
                module_id, ml, lay, sTgcIdHelper::sTgcChannelTypes::Wire, 10);
            for (const Identifier& id : {strip_id, wire_id}) {
              m_nswPads[id] = std::make_unique<TGraph>();
              int signed_layer = layerId(id);
              if (!m_nswLayers[signed_layer]) {
                m_nswLayers[signed_layer] = std::make_unique<TGraph>();
              }
              if (!m_nswActiveAreas[signed_layer]) {
                m_nswActiveAreas[signed_layer] = std::make_unique<TH2D>(
                    std::to_string(m_nswActiveAreas.size()).c_str(),
                    "ActiveNSW;x [mm]; y [mm]", 1000, -5001, 5001., 1000,
                    -5001., 5001.);
              }
            }
          }
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}
