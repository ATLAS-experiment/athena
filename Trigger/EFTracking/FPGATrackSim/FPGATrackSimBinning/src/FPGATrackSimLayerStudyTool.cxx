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
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include <algorithm>
#include <bit>
#include <numbers>

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

  ATH_CHECK(bookTrees());

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
        (";" + m_binnedhits->getBinTool().binDesc()->parNames(i) + ";").c_str(), 2000,
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
  ATH_CHECK(makeAndRegHist(m_binsFilled, "binsFilled", "; Bins Filled per Event", 200, 0, 200));


  // All Hit level histograms
  ATH_CHECK(makeAndRegHistVector(m_rZ_allhits, nLyrs + 1, NULL, "RZ_allhits",
                                 "; Z [mm] ; R [mm] ", 500, -2000, 2000, 500, 0, 500));

  ATH_CHECK(makeAndRegHistVector(m_phiResidual, nLyrs + 1, NULL, "phiResidual",
                                 "phi residual [mm]", 1000, -10, 10));
  ATH_CHECK(makeAndRegHistVector(m_etaResidual, nLyrs + 1, NULL, "etaResidual",
                                 "eta residual [mm]", 1000, -10, 10));

  ATH_CHECK(makeAndRegHistVector(m_phiResidual_v_r, m_N_ptplot, NULL, "phiResidual_v_r",
                                  "; R[mm]; phi residual [mm]", 1200, 0, 1200, 1000, -100, 100));
  ATH_CHECK(makeAndRegHistVector(m_etaResidual_v_r, m_N_ptplot, NULL, "etaResidual_v_r",
                                  "; R[mm]; eta residual [mm]", 1200, 0, 1200, 1000, -100, 100));

  ATH_CHECK(makeAndRegHistVector(m_phiScale_v_r, m_N_ptplot, NULL, "phiScale_v_r",
                                    "; R[mm]; phi scale", 1200, 0, 1200, 1000, -2, 2.0));
  ATH_CHECK(makeAndRegHistVector(m_etaScale_v_r, m_N_ptplot, NULL, "etaScale_v_r",
                                    "; R[mm]; eta scale", 1200, 0, 1200, 1000, -2.0, 2.0));

  ATH_CHECK(makeAndRegHistVector(m_phiTrueBinShift, nLyrs + 1, NULL,
                                 "phiTrueBinShift", "phi TrueBinShift [mm]", 1000, -10, 10));
  ATH_CHECK(makeAndRegHistVector(m_etaTrueBinShift, nLyrs + 1, NULL,
                                 "etaTrueBinShift", "eta TrueBinShift [mm]",1000, -10, 10));

  // Road statistics
  ATH_CHECK(makeAndRegHist(m_phiShift_road, "phiShift_road", ";Phi Shift", 2000, -m_phiScale, m_phiScale));
  ATH_CHECK(makeAndRegHist(m_etaShift_road, "etaShift_road", ";Eta Shift", 2000, -m_etaScale, m_etaScale));
  ATH_CHECK(makeAndRegHist(m_phiShift2D_road, "phiShift2D_road", ";Phi Shift; R", 400, -m_phiScale, m_phiScale, 100, 0, 400));
  ATH_CHECK(makeAndRegHist(m_etaShift2D_road, "etaShift2D_road", ";Phi Shift; R", 400, -m_etaScale, m_etaScale, 100, 0, 400));

  // Efficiency monitoring
  ATH_CHECK(makeAndRegHistVector(m_ptDist, m_distPlotClasses.size(), &m_distPlotClasses,
                                 "ptDist", "pT [GeV]",400, 0, 100));
  ATH_CHECK(makeAndRegHistVector(m_etaDist, m_distPlotClasses.size(), &m_distPlotClasses,
                                 "etaDist", "#eta",1000, -5, 5));
  ATH_CHECK(makeAndRegHistVector(m_phiDist, m_distPlotClasses.size(), &m_distPlotClasses,
                                 "phiDist", "#phi",640, 0, 2*std::numbers::pi));
  ATH_CHECK(makeAndRegHistVector(m_d0Dist, m_distPlotClasses.size(), &m_distPlotClasses,
                                 "d0Dist", "d_{0} [mm]",120, -3.0, 3.0));
  ATH_CHECK(makeAndRegHistVector(m_z0Dist, m_distPlotClasses.size(), &m_distPlotClasses,
                                 "z0Dist", "z_{0} [mm]",400, -200.0, 200.0));

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimLayerStudyTool::bookTrees() {
  ATH_MSG_DEBUG("Booking Layer Study Tree: " << m_layerStudyTreeName
                 << " and Truth Tree: " << m_truthTreeName);

  // Create the LayerStudy tree
  m_bin_tree = new TTree(m_layerStudyTreeName.value().c_str(),
                         m_layerStudyTreeName.value().c_str());
  m_bin_tree->Branch("bin",     &m_bin_tree_bin);
  m_bin_tree->Branch("r",       &m_bin_tree_r);
  m_bin_tree->Branch("z",       &m_bin_tree_z);
  m_bin_tree->Branch("id",      &m_bin_tree_id);
  m_bin_tree->Branch("hash",    &m_bin_tree_hash);
  m_bin_tree->Branch("layer",   &m_bin_tree_layer);
  m_bin_tree->Branch("side",    &m_bin_tree_side);
  m_bin_tree->Branch("etamod",  &m_bin_tree_etamod);
  m_bin_tree->Branch("phimod",  &m_bin_tree_phimod);
  m_bin_tree->Branch("dettype", &m_bin_tree_dettype);
  m_bin_tree->Branch("detzone", &m_bin_tree_detzone);

  // Create the Truth tree
  m_truth_tree = new TTree(m_truthTreeName.value().c_str(),
                           m_truthTreeName.value().c_str());
  m_truth_tree->Branch("stdpars", &m_truth_tree_phi);
  m_truth_tree->Branch("stdpars", &m_truth_tree_qOverPt);
  m_truth_tree->Branch("stdpars", &m_truth_tree_eta);
  m_truth_tree->Branch("stdpars", &m_truth_tree_d0);
  m_truth_tree->Branch("stdpars", &m_truth_tree_z0);
  m_truth_tree->Branch("parset",  &m_truth_tree_parset);

  // Register with THistSvc — convert Gaudi::Property to std::string before concatenation
  ATH_CHECK(m_tHistSvc->regTree(m_dir.value() + m_layerStudyTreeName.value(), m_bin_tree));
  ATH_CHECK(m_tHistSvc->regTree(m_dir.value() + m_truthTreeName.value(),      m_truth_tree));

  return StatusCode::SUCCESS;
}

void FPGATrackSimLayerStudyTool::ClearTreeVectors()
{
  m_bin_tree_r.clear();
  m_bin_tree_z.clear();
  m_bin_tree_id.clear();
  m_bin_tree_hash.clear();
  m_bin_tree_layer.clear();
  m_bin_tree_side.clear();
  m_bin_tree_etamod.clear();
  m_bin_tree_phimod.clear();
  m_bin_tree_dettype.clear();
  m_bin_tree_detzone.clear();

  m_truth_tree_parset.clear();
}

void FPGATrackSimLayerStudyTool::fillBinLevelOutput ATLAS_NOT_THREAD_SAFE(const FPGATrackSimBinUtil::IdxSet &idx,
                                  const FPGATrackSimBinnedHits::BinEntry &data)
{
  setBinPlotsActive(idx);

  // fill all truth 
  m_ptDist[0]->Fill(std::abs(1/m_truthpars.qOverPt));
  m_etaDist[0]->Fill(1/m_truthpars.eta);
  m_phiDist[0]->Fill(1/m_truthpars.phi);
  m_d0Dist[0]->Fill(1/m_truthpars.d0);
  m_z0Dist[0]->Fill(1/m_truthpars.z0);

  if (m_binPlotsActive) {
    for (auto& hit : data.hits) {
      m_phiShift_road->Fill(hit.phiShift);
      m_etaShift_road->Fill(hit.etaShift);
      m_phiShift2D_road->Fill(hit.phiShift, hit.hitptr->getR());
      m_etaShift2D_road->Fill(hit.etaShift, hit.hitptr->getR());
    }

    // fill param monitoring
    for (int i = 0; i < 2; i++) {
      // i=0 all, i=1 no missed layers, i=2 is one missed layer
      if ((i == 0)|| (data.lyrCnt() >= m_binnedhits->getNLayers() - (i - 1))) {
          // all layer hit
          m_ptDist[i]->Fill(std::abs(1 / m_truthpars.qOverPt));
          m_etaDist[i]->Fill(1 / m_truthpars.eta);
          m_phiDist[i]->Fill(1 / m_truthpars.phi);
          m_d0Dist[i]->Fill(1 / m_truthpars.d0);
          m_z0Dist[i]->Fill(1 / m_truthpars.z0);
        }
    }

    // Module mapping and Layer definition studies
    // first sort hits by r+z radii
    std::vector<FPGATrackSimBinUtil::StoredHit> sorted_hits = data.hits;
    std::sort(sorted_hits.begin(), sorted_hits.end(),
              [](const auto &hit1, const auto &hit2) {
                return hit1.rzrad() < hit2.rzrad();
              });

    // Fill tree
    m_bin_tree_bin = std::vector<unsigned>(idx);
    ClearTreeVectors();
    for (auto &hit : sorted_hits) {

      m_bin_tree_r.push_back(hit.hitptr->getR());
      m_bin_tree_z.push_back(hit.hitptr->getZ());
      m_bin_tree_id.push_back(hit.hitptr->getIdentifier());
      m_bin_tree_hash.push_back(hit.hitptr->getIdentifierHash());
      m_bin_tree_layer.push_back(hit.hitptr->getLayerDisk());
      m_bin_tree_side.push_back(hit.hitptr->getSide());
      m_bin_tree_etamod.push_back(hit.hitptr->getEtaModule());
      m_bin_tree_phimod.push_back(hit.hitptr->getPhiModule());
      m_bin_tree_dettype.push_back((int)hit.hitptr->getDetType());
      m_bin_tree_detzone.push_back((int)hit.hitptr->getDetectorZone());
    }
    m_bin_tree->Fill();
    m_binsFilledCnt++;
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

  m_binsFilled->Fill(m_binsFilledCnt);
  m_binsFilledCnt=0;

}

void FPGATrackSimLayerStudyTool::fillHitLevelInput(const FPGATrackSimHit *hit) {
  const IFPGATrackSimBinDesc* bindesc = m_binnedhits->getBinTool().binDesc();
  m_rZ_allhits[m_binnedhits->getNLayers()]->Fill(hit->getZ(), hit->getR());       // all layer plot
  if (m_truthIsValid)
    {
      m_etaResidual[m_binnedhits->getNLayers()]->Fill(bindesc->etaResidual(m_truthparset, hit));
      m_phiResidual[m_binnedhits->getNLayers()]->Fill(bindesc->phiResidual(m_truthparset, hit));

      int ptbin =
          floor((m_truthpars[FPGATrackSimTrackPars::IHIP] + 1.0) / 2.0 * m_N_ptplot);
      ptbin = std::min(std::max(ptbin,0),4);
      m_etaResidual_v_r[ptbin]->Fill(hit->getR(), bindesc->etaResidual(m_truthparset, hit));
      m_phiResidual_v_r[ptbin]->Fill(hit->getR(), bindesc->phiResidual(m_truthparset, hit));

      double expectedshift = FPGATrackSimBinUtil::GeomHelpers::dPhiHitTrkFromPars(hit->getR(), m_truthpars);
      double phiscale = -1.0*(hit->getGPhi()-m_truthpars[FPGATrackSimTrackPars::IPHI])/expectedshift;
      m_phiScale_v_r[ptbin]->Fill(hit->getR(), phiscale);


      if (m_truthbin.back() != FPGATrackSimBinUtil::invalidBin) {
        FPGATrackSimBinUtil::ParSet binCenter = m_binnedhits->getBinTool().lastStep()->binCenter(m_truthbin.back());
        m_etaTrueBinShift[m_binnedhits->getNLayers()]->Fill(bindesc->etaResidual(binCenter, hit));
        m_phiTrueBinShift[m_binnedhits->getNLayers()]->Fill(bindesc->phiResidual(binCenter, hit));
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
  m_truthbin.clear();
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

  m_truth_tree_phi = m_truthpars[FPGATrackSimTrackPars::IPHI];
  m_truth_tree_qOverPt = m_truthpars[FPGATrackSimTrackPars::IHIP];
  m_truth_tree_d0 = m_truthpars[FPGATrackSimTrackPars::ID0];
  m_truth_tree_z0 = m_truthpars[FPGATrackSimTrackPars::IZ0];
  m_truth_tree_eta = m_truthpars[FPGATrackSimTrackPars::IETA];
  m_truth_tree_parset = std::vector<double>(m_truthparset);
  m_truth_tree->Fill();

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
    ATH_MSG_INFO("Truth out of range because truth parset = " << m_truthparset << " wrt min = " << m_binnedhits->getBinTool().parMin() << ", max = " << m_binnedhits->getBinTool().parMax());
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


void FPGATrackSimLayerStudyTool::setBinPlotsActive(const FPGATrackSimBinUtil::IdxSet &idx)
{
  m_binPlotsActive = (!m_isSingleParticle);

  // this finds the parameters at all 2^5 corners of the bin and then finds the min and max of those
  std::vector<FPGATrackSimBinUtil::IdxSet> idxsets = FPGATrackSimBinUtil::makeVariationSet(std::vector<unsigned>({0,1,2,3,4}),idx);
  const FPGATrackSimBinTool &bintool = m_binnedhits->getBinTool();
  const IFPGATrackSimBinDesc* bindesc = bintool.binDesc();

  // get window in std parameters for bin
  FPGATrackSimTrackPars minpars = bindesc->parSetToTrackPars(bintool.lastStep()->binCenter(idx));   
  FPGATrackSimTrackPars maxpars = bindesc->parSetToTrackPars(bintool.lastStep()->binCenter(idx));   
  for (FPGATrackSimBinUtil::IdxSet & idxset : idxsets) {
    FPGATrackSimTrackPars trackpars = bindesc->parSetToTrackPars(bintool.lastStep()->binLowEdge(idxset));      
    for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
      minpars[par] = std::min(minpars[par],trackpars[par]);
      maxpars[par] = std::max(maxpars[par],trackpars[par]);
    }
  }

  // check if truth track is in bin within padding
  FPGATrackSimTrackPars padding;
  padding[FPGATrackSimTrackPars::ID0] = m_d0pad;
  padding[FPGATrackSimTrackPars::IZ0] = m_z0pad;
  padding[FPGATrackSimTrackPars::IETA] = m_etapad;
  padding[FPGATrackSimTrackPars::IPHI] = m_phipad;
  padding[FPGATrackSimTrackPars::IHIP] = m_qptpad;
  bool inRange = true;
  for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
      inRange = inRange &&  (m_truthpars[par] > minpars[par]-padding[par]);
      inRange = inRange &&  (m_truthpars[par] < maxpars[par]+padding[par]);
  }
  //m_binPlotsActive |= inRange;
  m_binPlotsActive = m_binPlotsActive||(m_truthbin.back()==idx)||(m_plotAllBins) ;


}
