// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinning.cxx
 * @author Elliot Lipeles, Ben Rosser
 * @date Sept 10th, 2024
 * @brief See header file.
 */

#include "FPGATrackSimBinning/FPGATrackSimLayerStudyTool.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FPGATrackSimBinning/FPGATrackSimBinnedHits.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include <bit>

FPGATrackSimLayerStudyTool::FPGATrackSimLayerStudyTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  AthAlgTool(algname, name, ifc)
{
}

StatusCode FPGATrackSimLayerStudyTool::initialize()
{
  // Dump the configuration to make sure it propagated through right
  auto props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }

  ATH_CHECK(m_tHistSvc.retrieve());

  ATH_CHECK(bookTree());

  return StatusCode::SUCCESS;

}

StatusCode FPGATrackSimLayerStudyTool::registerHistograms(const FPGATrackSimBinnedHits* binnedhits, bool skipTruth)
{
  m_binnedhits = binnedhits;
  const FPGATrackSimBinTool &bintool = m_binnedhits->getBinTool();
  const IFPGATrackSimBinDesc* bindesc = bintool.binDesc();
  int nLyrs = m_binnedhits->getNLayers();

  // *Technically* this is a bit fragile. Since we are testing if the event selection service is set to
  // the "skip truth" type rather than testing if it's set to the SingleMuon, SingleElectron, or SinglePion types.
  // At the moment though we either set skip truth or we are running single particle, so it could be changed later.
  m_isSingleParticle = not skipTruth;

  // This is because if you change the binning class you can change what axis
  // ranges you need for the plotting
  ATH_MSG_INFO("Hist scales phi: " << m_phiScale << "  eta: " << m_etaScale << "  dr:" << m_drScale);

  // Truth Parameter distributions, bounds should cover 3x nominal range
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    ATH_CHECK(makeAndRegHist(
        m_truthpars_hists[i], ("truth" + bindesc->parNames(i)).c_str(),
        (";" + m_binnedhits->getBinTool().binDesc()->parNames(i) + ";").c_str(), 200,
        -2 * m_binnedhits->getBinTool().parRange(i),
        2 * m_binnedhits->getBinTool().parRange(i)));
  }

  // Data flow hists. We may not need these for the layer study.
  ATH_CHECK(makeAndRegHist(m_inputHits, "InputHits", ";Input Hits", 200, 0, 100000));
  ATH_CHECK(makeAndRegHistVector(m_hitsPerStepBin, bintool.steps().size(),
                                 &bintool.stepNames(), "hitsPerStep",
                                 "; Hits per bin in step", 20, 0, m_isSingleParticle ? 50 : 10000));
  ATH_CHECK(makeAndRegHist(m_hitsPerLayer, "hitsPerLayer", "; Layer ; Hits ", nLyrs, 0, nLyrs));
  ATH_CHECK(makeAndRegHist(m_hitsPerLayer2D, "hitsPerLayer2D", "; Layer ; Hits ", nLyrs, 0, nLyrs, 20, 0, m_isSingleParticle ? 20 : 10000));

  // Road statistics
  ATH_CHECK(makeAndRegHist(m_phiShift_road, "phiShift_road", ";Phi Shift", 2000, -m_phiScale, m_phiScale));
  ATH_CHECK(makeAndRegHist(m_etaShift_road, "etaShift_road", ";Eta Shift", 2000, -m_etaScale, m_etaScale));
  ATH_CHECK(makeAndRegHist(m_phiShift2D_road, "phiShift2D_road", ";Phi Shift; R", 400, -m_phiScale, m_phiScale, 100, 0, 400));
  ATH_CHECK(makeAndRegHist(m_etaShift2D_road, "etaShift2D_road", ";Phi Shift; R", 400, -m_etaScale, m_etaScale, 100, 0, 400));


  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimLayerStudyTool::bookTree() {
  ATH_MSG_DEBUG("Booking Layers Study  Tree");
  m_bin_module_tree = new TTree("LayerStudy","LayerStudy");
  m_bin_module_tree->Branch("bin", &m_tree_bin);
  m_bin_module_tree->Branch("r", &m_tree_r);
  m_bin_module_tree->Branch("z", &m_tree_z);
  m_bin_module_tree->Branch("id", &m_tree_id);
  m_bin_module_tree->Branch("hash", &m_tree_hash);
  m_bin_module_tree->Branch("layer", &m_tree_layer);
  m_bin_module_tree->Branch("side", &m_tree_side);
  m_bin_module_tree->Branch("etamod",  &m_tree_etamod);
  m_bin_module_tree->Branch("phimod",  &m_tree_phimod);
  m_bin_module_tree->Branch("dettype", &m_tree_dettype);
  m_bin_module_tree->Branch("detzone",  &m_tree_detzone);

  ATH_CHECK(m_tHistSvc->regTree(m_dir + m_bin_module_tree->GetName(), m_bin_module_tree));
  return StatusCode::SUCCESS;
}
void FPGATrackSimLayerStudyTool::ClearTreeVectors()
{
  m_tree_r.clear();
  m_tree_z.clear();
  m_tree_id.clear();
  m_tree_hash.clear();
  m_tree_layer.clear();
  m_tree_side.clear();
  m_tree_etamod.clear();
  m_tree_phimod.clear();
  m_tree_dettype.clear();
  m_tree_detzone.clear();
}

void FPGATrackSimLayerStudyTool::fillBinLevelOutput ATLAS_NOT_THREAD_SAFE(const FPGATrackSimBinUtil::IdxSet &idx,
                                  const FPGATrackSimBinnedHits::BinEntry &data)
{
  setBinPlotsActive(idx);

  if (m_binPlotsActive) {
    for (auto& hit : data.hits) {
      m_phiShift_road->Fill(hit.phiShift);
      m_etaShift_road->Fill(hit.etaShift);
      m_phiShift2D_road->Fill(hit.phiShift, hit.hitptr->getR());
      m_etaShift2D_road->Fill(hit.etaShift, hit.hitptr->getR());
    }

    // Module mapping and Layer definition studies
    // first sort hits by r+z radii
    std::vector<FPGATrackSimBinUtil::StoredHit> sorted_hits = data.hits;
    std::sort(sorted_hits.begin(), sorted_hits.end(),
              [](const auto &hit1, const auto &hit2) {
                return hit1.rzrad() < hit2.rzrad();
              });

    // Fill tree
    m_tree_bin = std::vector<unsigned>(idx);
    ClearTreeVectors();
    for (auto &hit : sorted_hits) {
      m_tree_r.push_back(hit.hitptr->getR());
      m_tree_z.push_back(hit.hitptr->getZ());
      m_tree_id.push_back(hit.hitptr->getIdentifier());
      m_tree_hash.push_back(hit.hitptr->getIdentifierHash());
      m_tree_layer.push_back(hit.hitptr->getLayerDisk());
      m_tree_side.push_back(hit.hitptr->getSide());
      m_tree_etamod.push_back(hit.hitptr->getEtaModule());
      m_tree_phimod.push_back(hit.hitptr->getPhiModule());
      m_tree_dettype.push_back((int)hit.hitptr->getDetType());
      m_tree_detzone.push_back((int)hit.hitptr->getDetectorZone());
    }
    m_bin_module_tree->Fill();
  }
}

void FPGATrackSimLayerStudyTool::fillBinningSummary ATLAS_NOT_THREAD_SAFE(
    const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits)
{
  m_inputHits->Fill(hits.size());

  for (auto &step : m_binnedhits->getBinTool().steps()) {
    for (auto bin : m_binnedhits->binnedHits()[step->stepNum()])
      m_hitsPerStepBin[step->stepNum()]->Fill(bin.data().hitCnt);
  }

  for (auto bin :m_binnedhits->binnedHits()[m_binnedhits->getBinTool().lastStep()->stepNum()]) {
    for (unsigned lyr = 0; lyr < m_binnedhits->getNLayers(); lyr++) {
      unsigned cnt = bin.data().hitsInLyr(lyr);
      m_hitsPerLayer->Fill(lyr, cnt);
      m_hitsPerLayer2D->Fill(lyr, cnt);
    }
  }
}


void FPGATrackSimLayerStudyTool::parseTruthInfo ATLAS_NOT_THREAD_SAFE(std::vector<FPGATrackSimTruthTrack> const & truthtracks) {
  ATH_MSG_DEBUG("In parseTruthInfo, truthtracks size = " << truthtracks.size());
  m_truthIsValid = false;

  const IFPGATrackSimBinDesc* bindesc = m_binnedhits->getBinTool().binDesc();

  if (truthtracks.size() == 0) return;

  // Convert to binning parameters and find truth bin for each step
  m_truthpars = (truthtracks)[0].getPars();
  m_truthpars[FPGATrackSimTrackPars::IHIP] =
      m_truthpars[FPGATrackSimTrackPars::IHIP] * 1000;
  m_truthparset = bindesc->trackParsToParSet(m_truthpars);
  for (auto &step : m_binnedhits->getBinTool().steps()) {
    m_truthbin.push_back(step->binIdx(m_truthparset));
  }

  // histogram parameters
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    m_truthpars_hists[i]->Fill(m_truthparset[i]);
  }

  // a closure test
  FPGATrackSimTrackPars recovered = bindesc->parSetToTrackPars(m_truthparset);
  ATH_MSG_DEBUG("parset:" << m_truthparset << " " << m_truthpars
                          << " ?= " << recovered << " closure:"
                          << " " << recovered[FPGATrackSimTrackPars::IHIP] - m_truthpars[FPGATrackSimTrackPars::IHIP]
                          << " " << recovered[FPGATrackSimTrackPars::IPHI] - m_truthpars[FPGATrackSimTrackPars::IPHI]
                          << " " << recovered[FPGATrackSimTrackPars::ID0] - m_truthpars[FPGATrackSimTrackPars::ID0]
                          << " " << recovered[FPGATrackSimTrackPars::IETA] - m_truthpars[FPGATrackSimTrackPars::IETA]
                          << " " << recovered[FPGATrackSimTrackPars::IZ0] - m_truthpars[FPGATrackSimTrackPars::IZ0]);

  // print if there are multiple tracks for debugging single track MC
  if (truthtracks.size() > 1) {
    for (unsigned i = 0; i < truthtracks.size(); i++) {
      ATH_MSG_INFO("Multiple truth" << i << " of " << truthtracks.size()
                                    << " " << (truthtracks)[i].getPars());
    }
  }

  // find truth bin for later plotting selections
  ATH_MSG_DEBUG("truthbin " << truthtracks.size()
                            << " " << m_truthpars << " " << m_truthbin);

  // Check if the truth track falls in the binning range
  if (!m_binnedhits->getBinTool().inRange(m_truthparset)) {
    ATH_MSG_INFO("Truth out of range");
    return;
  }

  // Check that truth track falls in an actual bin
  // this should alway pass, except for weird events
  m_truthIsValid = true;
  for (auto &step : m_binnedhits->getBinTool().steps()) {
    if (!step->validBinsFull()[m_truthbin[step->stepNum()]]) {
      ATH_MSG_INFO("Truth Bin not valid! Step " << step->stepName() << " "
                                                << m_truthbin[step->stepNum()]
                                                << " : " << m_truthpars);
      std::vector<FPGATrackSimBinUtil::IdxSet> idxsets =
          FPGATrackSimBinUtil::makeVariationSet(
              std::vector<unsigned>({0, 1, 2, 3, 4}),
              m_truthbin[step->stepNum()]);
      for (FPGATrackSimBinUtil::IdxSet &idxset : idxsets) {
        ATH_MSG_INFO("Truth Box "
                     << bindesc->parSetToTrackPars(step->binLowEdge(idxset)));
      }
      m_truthIsValid = false;
    }
  }
}
