// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinning.cxx
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief See header file.
 */

#include "FPGATrackSimGenScanMonitoring.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"


FPGATrackSimGenScanMonitoring::FPGATrackSimGenScanMonitoring(const std::string& algname, const std::string &name, const IInterface *ifc) :
  AthAlgTool(algname, name, ifc)
{
}

StatusCode FPGATrackSimGenScanMonitoring::initialize()
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

StatusCode FPGATrackSimGenScanMonitoring::registerHistograms( 
  unsigned nLayers, const FPGATrackSimGenScanBinningBase  *binning, double rin, double rout)
{
  m_nLayers = nLayers;
  m_binning = binning;
  m_rin=rin;
  m_rout = rout;

  // This is because if you change the binning class you can change what axis
  // ranges you need for the plotting
  double phiScale = m_binning->phiHistScale();
  double etaScale = m_binning->etaHistScale();
  double drScale = (rout-rin) / nLayers;
  ATH_MSG_INFO("Hist scales phi: " << phiScale << "  eta: " << etaScale << "  dr:" << drScale);
  
  allocateDataFlowCounters();

  // Truth Parameter distributions, bounds should cover 3x nominal range
  for (unsigned i = 0; i < FPGATrackSimGenScanBinningBase::NPars; i++) {
    ATH_CHECK(makeAndRegHist(
        m_truthpars_hists[i], ("truth" + m_binning->parNames(i)).c_str(),
        (";" + m_binning->parNames(i) + ";").c_str(), 200,
        2 * m_binning->m_parMin[i] - m_binning->m_parMax[i],
        2 * m_binning->m_parMax[i] - m_binning->m_parMin[i]));
  }

  // All Hit level histograms
  ATH_CHECK(makeAndRegHist(m_hitLyrsAllBins , "HitLyrsAllBins" , ";Hit Layers", 10, 0, 10));
  ATH_CHECK(makeAndRegHistVector(m_rZ_allhits, m_nLayers+1, NULL , 
            "RZ_allhits", "; Z [mm] ; R [mm] ", 500, -2000, 2000, 500, 0, 500));
  
  ATH_CHECK(makeAndRegHistVector(m_phiResidual, m_nLayers+1, NULL , 
            "phiResidual", "phi residual [mm]", 1000, -10, 10));
  ATH_CHECK(makeAndRegHistVector(m_etaResidual,m_nLayers+1, NULL ,  
            "etaResidual", "eta residual [mm]", 1000, -10, 10));

  ATH_CHECK(makeAndRegHistVector(m_phiTrueBinShift, m_nLayers+1, NULL , 
            "phiTrueBinShift", "phi TrueBinShift [mm]", 1000, -10, 10));
  ATH_CHECK(makeAndRegHistVector(m_etaTrueBinShift,m_nLayers+1, NULL ,  
            "etaTrueBinShift", "eta TrueBinShift [mm]", 1000, -10, 10));

  // Road statistics
  ATH_CHECK(makeAndRegHist(m_outputHitsPerBin, "OutputHitsPerBin",  ";Output Hits Per Bin", 100, 0, 200));
  ATH_CHECK(makeAndRegHist(m_outputHitsPerSlice, "OutputHitsPerSlice",  ";Output Hits Per Slice", 1000, 0, 2000));
  ATH_CHECK(makeAndRegHist(m_outputHitsPerRow, "OutputHitsPerRow",  ";Output Hits Per Row", 1000, 0, 2000));
  ATH_CHECK(makeAndRegHist(m_outputRoadsPerRow, "OutputRoadsPerRow",  ";Output Roads Per Row", 100, 0, 100));
  ATH_CHECK(makeAndRegHist(m_roadFilterFlow , "roadFilterFlow" , "road filter flow", 10, 0, 10));
  ATH_CHECK(makeAndRegHist(m_phiShift_road, "phiShift_road", ";Phi Shift", 2000, -phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_etaShift_road, "etaShift_road", ";Eta Shift", 2000, -etaScale, etaScale));
  ATH_CHECK(makeAndRegHist(m_phiShift2D_road, "phiShift2D_road", ";Phi Shift; R", 
                          400, -phiScale, phiScale, 100, 0, 400));
  ATH_CHECK(makeAndRegHist(m_etaShift2D_road, "etaShift2D_road", ";Phi Shift; R", 
                          400, -etaScale, etaScale, 100, 0, 400));

  ATH_CHECK(makeAndRegHist(m_hitsPerLayer_road , "HitsPerLayer_road" , "; Layer; Hits", m_nLayers, 0, m_nLayers, 20, 0, 20));


  // Data flow hists
  ATH_CHECK(makeAndRegHist(m_hitsLoadedPerLayer, "HitsLoadedPerLayer", "; Layer; Hits", m_nLayers, 0, m_nLayers));
  ATH_CHECK(makeAndRegHist(m_inputHits, "InputHits",  ";Input Hits", 200, 0, 100000));
  ATH_CHECK(makeAndRegHist(m_inputHitsPerSlice, "InputHitsPerSlice",  ";Input Hits Per Slice", 1000, 0, 10000));
  ATH_CHECK(makeAndRegHist(m_inputHitsPerRow, "InputHitsPerRow",  ";Input Hits Per Row", 1000, 0, 10000));
  ATH_CHECK(makeAndRegHist(m_hitsPerLayer, "HitsPerLayer", "Layer; Hits", m_nLayers, 0, m_nLayers));
  ATH_CHECK(makeAndRegHist(m_hitsPerLayer2D, "HitsPerLayer2D", "Layer; Hits",
                           m_nLayers, 0, m_nLayers, 20, 0,
                           m_isSingleParticle ? 20 : 10000));
  ATH_CHECK(makeAndRegHist(m_pairs, "pairs", ";pairs;", 100, 0, 500));
  ATH_CHECK(makeAndRegHist(m_pairinghits, "pairinghits", ";Pairing Hits;", 50, 0, 50));
  ATH_CHECK(makeAndRegHist(m_filteredpairs, "filteredpairs", "filteredpairs", 100, 0, 500));
  ATH_CHECK(makeAndRegHist(m_pairsets, "pairsets", ";pairsets;", 100, 0, 100));


  // Pair Filter Cuts
  ATH_CHECK(makeAndRegHist(m_phiOutExtrap, "PhiOutExtrap", ";Phi Out Extrap", 2000, -phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_phiInExtrap, "PhiInExtrap", ";Phi In Extrap", 2000, -phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_phiOutExtrapCurveLimit, "PhiOutExtrapCurveLimit", 
    ";Phi Out Extrap w/ Curve Limit", 2000, -phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_phiInExtrapCurveLimit, "PhiInExtrapCurveLimit", 
    ";Phi In Extrap w/ Curve Limit", 2000, -phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_etaOutExtrap, "EtaOutExtrap", ";Eta Out Extrap", 2000, -etaScale, etaScale));
  ATH_CHECK(makeAndRegHist(m_etaInExtrap, "EtaInExtrap", ";Eta In Extrap", 2000, -etaScale, etaScale));
  ATH_CHECK(makeAndRegHist(m_deltaPhi, "DeltaPhi", ";Delta Phi [rad]", 2000, -1.0*phiScale, phiScale));
  ATH_CHECK(makeAndRegHist(m_deltaEta, "DeltaEta", ";Delta Eta [rad]", 2000, -1.0*etaScale, etaScale));
  ATH_CHECK(makeAndRegHist(m_deltaPhiDR, "DeltaPhiDR", ";Delta Phi / Delta R", 2000, -0.1 * phiScale, 0.1 * phiScale));
  ATH_CHECK(makeAndRegHist(m_deltaEtaDR, "DeltaEtaDR", ";Delta Eta / Delta R", 2000, -0.1 * phiScale, 0.1 * phiScale));
  ATH_CHECK(makeAndRegHistVector(m_deltaPhiByLyr, m_nLayers, NULL, "DeltaPhiDR", ";Delta Phi / Delta R", 2000, -0.1 * phiScale, 0.1 * phiScale));
  ATH_CHECK(makeAndRegHistVector(m_deltaEtaByLyr, m_nLayers, NULL,  "DeltaEtaDR", ";Delta Eta / Delta R", 2000, -0.1 * phiScale, 0.1 * phiScale));

  // Pair Match Cuts
  ATH_CHECK(makeAndRegHistVector(
      m_pairSetMatchPhi, m_twoPairClasses.size(), &m_twoPairClasses,
      "PairSetMatchPhi", ";PairSet Match Phi [mm]", 500,
      -1*phiScale / drScale, phiScale / drScale));
  ATH_CHECK(makeAndRegHistVector(
      m_pairSetMatchEta, m_twoPairClasses.size(), &m_twoPairClasses,
      "PairSetMatchEta", ";PairSet Match Eta [mm]", 500,
      -1*etaScale / drScale, etaScale / drScale));
  ATH_CHECK(makeAndRegHistVector(
      m_deltaDeltaPhi, m_twoPairClasses.size(), &m_twoPairClasses,
      "DeltaDeltaPhi", ";DeltaDelta   Phi [mm]", 500, -1.0*phiScale / drScale,
      phiScale / drScale));
  ATH_CHECK(makeAndRegHistVector(
      m_deltaDeltaEta, m_twoPairClasses.size(), &m_twoPairClasses,
      "DeltaDeltaEta", ";DeltaDelta   Eta [mm]", 500, -1.0 * etaScale / drScale,
      1.0 * etaScale / drScale));
  ATH_CHECK(makeAndRegHistVector(m_phiCurvature, m_twoPairClasses.size(),
                               &m_twoPairClasses, "PhiCurvature",
                               ";Phi Curvature ", 500,
                               -0.1 * phiScale / drScale / drScale,
                               0.1 * phiScale / drScale / drScale));
  ATH_CHECK(makeAndRegHistVector(m_etaCurvature, m_twoPairClasses.size(),
                               &m_twoPairClasses, "EtaCurvature",
                               ";Eta Curvature ", 500,
                               -0.1 * etaScale / drScale / drScale,
                               0.1 * etaScale / drScale / drScale));
  ATH_CHECK(makeAndRegHistVector(m_deltaPhiCurvature, m_twoPairClasses.size(),
                               &m_twoPairClasses, "DeltaPhiCurvature",
                               ";Delta Phi Curvature ", 500,
                               -0.1 * phiScale / drScale / drScale,
                               0.1 * phiScale / drScale / drScale));
  ATH_CHECK(makeAndRegHistVector(m_deltaEtaCurvature, m_twoPairClasses.size(),
                               &m_twoPairClasses, "DeltaEtaCurvature",
                               ";Delta Eta Curvature ", 500,
                               -0.1 * etaScale / drScale / drScale,
                               0.1 * etaScale / drScale / drScale));
  ATH_CHECK(makeAndRegHistVector(m_phiOutExtrapCurved, m_twoPairClasses.size(),
                               &m_twoPairClasses, "PhiOutExtrapCurved",
                               ";Phi Out Extrap Curved", 2000, -phiScale,
                               phiScale));
  ATH_CHECK(makeAndRegHistVector(m_phiInExtrapCurved, m_twoPairClasses.size(),
                               &m_twoPairClasses, "PhiInExtrapCurved",
                               ";Phi In Extrap Curved ", 2000, -phiScale,
                               phiScale));
  ATH_CHECK(makeAndRegHist(m_pairMatchPhi2D, "PairSetMatchPhi2D",
                         ";PairSet Match Phi vs dRin [mm]", 100, 0, 100, 500,
                         -100.0 * phiScale, 100.0 * phiScale));
  ATH_CHECK(makeAndRegHist(m_pairMatchEta2D, "PairSetMatchEta2D",
                         ";PairSet Match Eta vs dRin [mm]", 100, 0, 100, 500,
                         -100.0 * etaScale, 100.0 * etaScale));

  ATH_CHECK(makeAndRegHist(m_unpairedHits, "UnpairedHits", "; Stage ; Unpaired Hits" , m_nLayers+1, 0, m_nLayers+1, 20, 0, 20));
  ATH_CHECK(makeAndRegHist(m_pairsetsIncr, "PairSetsIncr", "; Stage ; Pairsets" , m_nLayers+1, 0, m_nLayers+1, 20, 0, 20));
  ATH_CHECK(makeAndRegHist(m_pairsetsHits, "PairSetsHits", "; Stage ; Hits in Pairsets" , m_nLayers+1, 0, m_nLayers+1, 20, 0, 20));
  ATH_CHECK(makeAndRegHist(m_totalInputIncr, "TotalInputIncr", "; Stage ; Pairsets+Unpaired" , m_nLayers+1, 0, m_nLayers+1, 20, 0, 20));
  ATH_CHECK(makeAndRegHist(m_binStagesIncr, "BinStagesIncr", "; Stage ; Bins at this point" , m_nLayers+1, 0, m_nLayers+1, 20, 0, 20));

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimGenScanMonitoring::bookTree() {
  ATH_MSG_DEBUG("Booking Layers Study  Tree");
  m_bin_module_tree = new TTree("LayerStudy","LayerStudy");
  m_bin_module_tree->Branch("bin", &m_tree_bin);
  m_bin_module_tree->Branch("r", &m_tree_r);
  m_bin_module_tree->Branch("z", &m_tree_z);
  m_bin_module_tree->Branch("id", &m_tree_id);
  m_bin_module_tree->Branch("layer", &m_tree_layer);
  m_bin_module_tree->Branch("side", &m_tree_side);
  m_bin_module_tree->Branch("etamod",  &m_tree_etamod);
  m_bin_module_tree->Branch("phimod",  &m_tree_phimod);
  m_bin_module_tree->Branch("dettype", &m_tree_dettype);
  m_bin_module_tree->Branch("detzone",  &m_tree_detzone);

  ATH_CHECK(m_tHistSvc->regTree(m_dir + m_bin_module_tree->GetName(), m_bin_module_tree));
  return StatusCode::SUCCESS;
}
void FPGATrackSimGenScanMonitoring::ClearTreeVectors()
{
  m_tree_r.clear();
  m_tree_z.clear();
  m_tree_id.clear();
  m_tree_layer.clear();
  m_tree_side.clear();
  m_tree_etamod.clear();
  m_tree_phimod.clear();
  m_tree_dettype.clear();
  m_tree_detzone.clear();
}


void FPGATrackSimGenScanMonitoring::allocateDataFlowCounters() {

  m_hitsCntByLayer.resize(m_nLayers,0);
  
  m_inputhitsperslice.setsize(m_binning->sliceBins(), 0);
  m_inputhitsperrow.setsize(m_binning->sliceAndScanBins(), 0);

  m_outputhitsperslice.setsize(m_binning->sliceBins(), 0);
  m_outputhitsperrow.setsize(m_binning->sliceAndScanBins(),0);
  m_outputroadsperrow.setsize(m_binning->sliceAndScanBins(),0);
}

void FPGATrackSimGenScanMonitoring::resetDataFlowCounters() {
  for (auto& bin: m_hitsCntByLayer) { bin =0;}
  for (auto& bin: m_inputhitsperslice) { bin.data() =0;}
  for (auto& bin: m_inputhitsperrow) { bin.data() =0;}
  for (auto& bin: m_outputhitsperslice) { bin.data() =0;}
  for (auto& bin: m_outputhitsperrow) { bin.data() =0;}
  for (auto &bin : m_outputroadsperrow) {
    bin.data() = 0;
  }
}



// This is done at the end of event execution to store any graphs that were created
StatusCode FPGATrackSimGenScanMonitoring::registerGraphs() 
{
  // Make a vector of pointers to all the eventDispSet instances...
  std::vector<FPGATrackSimGenScanMonitoring::eventDispSet *> allsets(
      {&m_allHitsGraph, &m_roadGraph, &m_lostPairFilterGraph,
       &m_passPairFilterGraph, &m_lostPairSetFilterGraph,
       &m_passPairSetFilterGraph});

  // Then loop to register them all
  for (auto& set : allsets) {
    ATH_CHECK(set->registerGraphs(this));
  }

  return StatusCode::SUCCESS;
}


void FPGATrackSimGenScanMonitoring::fillBinLevelOutput(const FPGATrackSimGenScanBinningBase::IdxSet &idx,
                                  const FPGATrackSimGenScanTool::BinEntry &data)
{
  setBinPlotsActive(idx);

  m_outputHitsPerBin->Fill(data.hits.size());
  m_outputhitsperslice[m_binning->sliceIdx(idx)] += data.hits.size();
  m_outputhitsperrow[m_binning->sliceAndScanIdx(idx)] += data.hits.size();
  m_outputroadsperrow[m_binning->sliceAndScanIdx(idx)] += 1;

  m_roadGraph.addEvent(data.hits);

  if (m_binPlotsActive) {
    for (auto& hit : data.hits) {
      m_phiShift_road->Fill(hit.phiShift);
      m_etaShift_road->Fill(hit.etaShift);
      m_phiShift2D_road->Fill(hit.phiShift, hit.hitptr->getR());
      m_etaShift2D_road->Fill(hit.etaShift, hit.hitptr->getR());
    }


    // Module mapping and Layer definition studies
    // first sort hits by r+z radii
    std::vector<FPGATrackSimGenScanTool::StoredHit> sorted_hits = data.hits;
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

void FPGATrackSimGenScanMonitoring::fillHitsByLayer(                           
      const std::vector<std::vector<const FPGATrackSimGenScanTool::StoredHit *> > & hitsByLayer)
{
  for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
  {
    m_hitsPerLayer_road->Fill(lyr, hitsByLayer[lyr].size());
  }
}

void  FPGATrackSimGenScanMonitoring::fillHitLevelInput(const FPGATrackSimHit* hit) {
  m_rZ_allhits[m_nLayers]->Fill(hit->getZ(), hit->getR());       // all layer plot
  m_rZ_allhits[hit->getLayer()]->Fill(hit->getZ(), hit->getR()); // layer specific plot
  m_hitsCntByLayer[hit->getLayer()]++;
  if (m_truthIsValid)
    {
      m_etaResidual[m_nLayers]->Fill(m_binning->etaResidual(m_truthparset, hit));
      m_etaResidual[hit->getLayer()]->Fill(m_binning->etaResidual(m_truthparset, hit));
      m_phiResidual[m_nLayers]->Fill(m_binning->phiResidual(m_truthparset, hit));
      m_phiResidual[hit->getLayer()]->Fill(m_binning->phiResidual(m_truthparset, hit));

      if (m_truthbin != m_binning->m_invalidBin) {
        m_etaTrueBinShift[m_nLayers]->Fill(m_binning->etaShift(m_truthbin,hit)); 
        m_etaTrueBinShift[hit->getLayer()]->Fill(m_binning->etaShift(m_truthbin,hit)); 
        m_phiTrueBinShift[m_nLayers]->Fill(m_binning->phiShift(m_truthbin,hit)); 
        m_phiTrueBinShift[hit->getLayer()]->Fill(m_binning->phiShift(m_truthbin,hit)); 
      }
  }
}

void FPGATrackSimGenScanMonitoring::sliceCheck( const std::vector<unsigned>& sliceidx) {
  if (m_truthIsValid &&
      (m_binning->subVec(m_binning->slicePars(), m_truthbin) == sliceidx)) {
    ATH_MSG_DEBUG("truth bin missed slice");
  }
}

void FPGATrackSimGenScanMonitoring::pairFilterCheck(
    const FPGATrackSimGenScanTool::HitPairSet &pairs,
    const FPGATrackSimGenScanTool::HitPairSet &filteredpairs, 
    bool passedPairFilter) {

  m_roadFilterFlow->Fill(0);
  m_pairs->Fill(pairs.pairList.size());
  m_filteredpairs->Fill(filteredpairs.pairList.size());

  if (passedPairFilter) {
    // if passed mark progress in flow histogram and make passed Histogram
    m_roadFilterFlow->Fill(1);
    m_passPairFilterGraph.addEvent(filteredpairs.hitlist);
  } else {
    // If the true bin didn't pass print some debug and make a lost event
    // display
    if (m_binPlotsActive) {
      ATH_MSG_INFO("lost at pair filter : hit layers = "
                   << filteredpairs.lyrCnt() << " "
                   << std::bitset<16>(filteredpairs.hitLayers));
      m_lostPairFilterGraph.addEvent(pairs.hitlist);
    }
  }
}

void FPGATrackSimGenScanMonitoring::incrementInputPerScan(
    const FPGATrackSimGenScanBinningBase::IdxSet& idx,
    const std::pair<unsigned, unsigned>& rowRange, const FPGATrackSimHit* hit) {
  if (rowRange.second > rowRange.first) {
    m_hitsLoadedPerLayer->Fill(double(hit->getLayer()));
    m_inputhitsperrow[m_binning->sliceAndScanIdx(idx)]++;
  } else {
    // Nice debug check for if hit is in the truthbin for single track sample
    if (m_truthIsValid && 
       (m_binning->subVec(m_binning->scanPars(), m_truthbin) == (m_binning->subVec(m_binning->scanPars(), idx)))) {
        ATH_MSG_DEBUG("truth bin missed row ");
    }
  }
}

void FPGATrackSimGenScanMonitoring::fillInputSummary(
    const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits,
    const FPGATrackSimGenScanArray<int>& validSlice,
    const FPGATrackSimGenScanArray<int>& validSliceAndScan)
{
  m_inputHits->Fill(hits.size());
  m_allHitsGraph.addEvent(hits);

  for (FPGATrackSimGenScanArray<int>::Iterator& val : m_inputhitsperslice)
  {
    if (!validSlice[val.idx()]) continue;
    m_inputHitsPerSlice->Fill(val.data());
  }

  for (FPGATrackSimGenScanArray<int>::Iterator& val : m_inputhitsperrow)
  {
    if (!validSliceAndScan[val.idx()]) continue;
    m_inputHitsPerRow->Fill(val.data());
  }

  for (unsigned i = 0; i < m_nLayers; i++)
  {
    m_hitsPerLayer->Fill(i, m_hitsCntByLayer[i]);
    m_hitsPerLayer2D->Fill(i, m_hitsCntByLayer[i]);
  }
}

void FPGATrackSimGenScanMonitoring::fillOutputSummary(
    const FPGATrackSimGenScanArray<int>& validSlice,
    const FPGATrackSimGenScanArray<int>& validSliceAndScan)
{
  for (FPGATrackSimGenScanArray<int>::Iterator& val : m_outputhitsperslice)
  {
    if (!validSlice[val.idx()]) continue;
    m_outputHitsPerSlice->Fill(val.data());
  }
  for (FPGATrackSimGenScanArray<int>::Iterator& val : m_outputhitsperrow)
  {
    if (!validSliceAndScan[val.idx()]) continue;
    m_outputHitsPerRow->Fill(val.data());
  }
  for (FPGATrackSimGenScanArray<int>::Iterator& val : m_outputroadsperrow) {
    if (!validSliceAndScan[val.idx()]) continue;
    m_outputRoadsPerRow->Fill(val.data());
  }
}



void FPGATrackSimGenScanMonitoring::parseTruthInfo(
  std::vector<FPGATrackSimTruthTrack> const * truthtracks, bool isSingleParticle,
  const FPGATrackSimGenScanArray<int>& validBin)
{
  ATH_MSG_DEBUG("In parseTruthInfo ptr = " << truthtracks 
                << " size = " << (truthtracks ? truthtracks->size() : 0));
  m_isSingleParticle = isSingleParticle;
  m_truthtracks = truthtracks;

  if (m_truthtracks)
  {
    if (m_truthtracks->size() > 0)
    {
      m_truthpars = (*m_truthtracks)[0].getPars();
      m_truthpars[FPGATrackSimTrackPars::IHIP] = m_truthpars[FPGATrackSimTrackPars::IHIP] * 1000;
      m_truthbin = m_binning->parsToBin(m_truthpars);
      m_truthparset = m_binning->trackParsToParSet(m_truthpars);

      // histogram parameters
      for (unsigned i = 0; i < FPGATrackSimGenScanBinningBase::NPars; i++)
      {        
        // fill
        m_truthpars_hists[i]->Fill(m_truthparset[i]);
      }

      // a closure test
      FPGATrackSimTrackPars recovered = m_binning->parSetToTrackPars(m_truthparset);
      ATH_MSG_DEBUG("parset:" << m_truthparset << " " << m_truthpars
                              << " ?= " << recovered << " closure:"
                              << " " << recovered[FPGATrackSimTrackPars::IHIP] - m_truthpars[FPGATrackSimTrackPars::IHIP]
                              << " " << recovered[FPGATrackSimTrackPars::IPHI] - m_truthpars[FPGATrackSimTrackPars::IPHI]
                              << " " << recovered[FPGATrackSimTrackPars::ID0] - m_truthpars[FPGATrackSimTrackPars::ID0]
                              << " " << recovered[FPGATrackSimTrackPars::IETA] - m_truthpars[FPGATrackSimTrackPars::IETA]
                              << " " << recovered[FPGATrackSimTrackPars::IZ0] - m_truthpars[FPGATrackSimTrackPars::IZ0]);
    }

    // print if there are multiple tracks for debugging single track MC
    if (m_truthtracks->size() > 1)
    {
      for (unsigned i = 0; i < m_truthtracks->size(); i++)
      {
        ATH_MSG_INFO("Multiple truth" << i << " of "
                                      << m_truthtracks->size() << " " << (*m_truthtracks)[i].getPars());
      }
    }

    // find truth bin for later plotting selections
    ATH_MSG_DEBUG("truthbin " << m_truthtracks << " " << m_truthtracks->size() << " " << m_truthpars << " " << m_truthbin);
    if (!m_binning->inRange(m_truthparset))
    {
      ATH_MSG_INFO("Truth out of range");
    }
    else if (!validBin[m_truthbin])
    {
      ATH_MSG_INFO("Truth Bin not valid!" << m_truthbin << " : " << m_truthpars);
      std::vector<FPGATrackSimGenScanBinningBase::ParSet> parsets = m_binning->makeVariationSet(std::vector<unsigned>({0, 1, 2, 3, 4}), m_truthbin);
      for (FPGATrackSimGenScanBinningBase::ParSet& parset : parsets)
      {
        ATH_MSG_INFO("Truth Box " << m_binning->parSetToTrackPars(parset));
      }
    }

    m_truthIsValid = true;
  }
}

void FPGATrackSimGenScanMonitoring::fillPairingHits(
    std::vector<const FPGATrackSimGenScanTool::StoredHit *> const *lastlyr,
    std::vector<const FPGATrackSimGenScanTool::StoredHit *> const
        *lastlastlyr) {

  auto size_if_nonzero_ptr =
      [](std::vector<const FPGATrackSimGenScanTool::StoredHit *> const *ptr) {
        if (ptr) {
          return int(ptr->size());
        } else {
          return 0;
        }
      };

  m_pairinghits->Fill(size_if_nonzero_ptr(lastlyr) +
                      size_if_nonzero_ptr(lastlastlyr));
}

void FPGATrackSimGenScanMonitoring::fillPairFilterCuts(
    const FPGATrackSimGenScanTool::HitPair &pair) {
 // m_pairs->Fill(pairs.pairList.size());
  if (m_binPlotsActive) {
      int lyr = pair.first->hitptr->getLayer();
      m_deltaPhi->Fill(pair.dPhi());
      m_deltaEta->Fill(pair.dEta());
      m_deltaPhiDR->Fill(pair.dPhi() / pair.dR());
      m_deltaEtaDR->Fill(pair.dEta() / pair.dR());
      m_deltaPhiByLyr[lyr]->Fill(pair.dPhi());
      m_deltaEtaByLyr[lyr]->Fill(pair.dEta());
      m_phiInExtrap->Fill(pair.PhiInExtrap(m_rin));
      m_phiOutExtrap->Fill(pair.PhiOutExtrap(m_rout));
      m_etaInExtrap->Fill(pair.EtaInExtrap(m_rin));
      m_etaOutExtrap->Fill(pair.EtaOutExtrap(m_rout));
  }
}


// This classifies a pair of pairs into a category for plotting
// e.g. are they bewtween hits in the same layer, are there three layers,
// if there are three layers are they sequential,...
// This is to try to understand some of the structure of the
// pair matching distributions
unsigned FPGATrackSimGenScanMonitoring::pairpairCategory(
    const FPGATrackSimGenScanTool::HitPair &pair,
    const FPGATrackSimGenScanTool::HitPair &lastpair) const 
{
  int minlyr = std::min(lastpair.first->layer, lastpair.second->layer);
  minlyr = std::min(minlyr, pair.first->layer);
  minlyr = std::min(minlyr, pair.second->layer);

  unsigned hitbits =
      ((1 << lastpair.first->layer) | (1 << lastpair.second->layer) |
       (1 << pair.first->layer) | (1 << pair.second->layer));
  hitbits = hitbits >> minlyr;  // now bit list starts with lowest hit layer

  int hitlyrs = __builtin_popcount(hitbits);

  assert(hitbits & 0x1);

  unsigned retv = 0;
  if (hitlyrs == 2)  // must be same layers
  {
    retv = 1;
  } else if (hitlyrs == 3) {
    if (hitbits == 0b111)  // 3 bits in a row -> adjacent layers
    {
      if ((lastpair.first->layer == pair.second->layer) ||
          (pair.first->layer == lastpair.second->layer)) {
        retv = 2;  // sequential
      } else {
        retv = 3;  // one is a skip and the other isn't
      }
    } else if ((hitbits == 0b1101) || (hitbits == 0b1011))  // skips 1 layer
    {
      retv = 4;
    }
  } else {
    retv = 5;
  }

  ATH_MSG_VERBOSE("pairpairCat "
                << m_twoPairClasses[retv] << " " << lastpair.first->layer << " "
                << lastpair.second->layer << " " << pair.first->layer << " "
                << pair.second->layer << " : " << minlyr << " "
                << std::bitset<16>(hitbits));

  return retv;
}

void FPGATrackSimGenScanMonitoring::fillPairSetFilterCut(
    std::vector<TH1D *> &histset, double val,
    const FPGATrackSimGenScanTool::HitPair &pair,
    const FPGATrackSimGenScanTool::HitPair &lastpair, bool nminus1) {
  
  if (m_binPlotsActive) {
    unsigned ppcat = pairpairCategory(pair, lastpair);
    histset[0]->Fill(val); // 0 is "all"
    histset[ppcat]->Fill(val); // 0 is "all"
    if (nminus1)
      histset[m_nminus1_idx]->Fill(val);
  }

}

void FPGATrackSimGenScanMonitoring::pairSetFilterCheck(
    const FPGATrackSimGenScanTool::HitPairSet &filteredpairs,
    const std::vector<FPGATrackSimGenScanTool::HitPairSet> &pairsets,
    unsigned threshold) {
  m_pairsets->Fill(pairsets.size());

  for (const auto& pairset : pairsets) {
    // if over threshold add it to the output
    if (pairset.lyrCnt() >= threshold) {
      m_roadFilterFlow->Fill(3);
      m_passPairSetFilterGraph.addEvent(pairset.hitlist);
    } else {
      m_lostPairSetFilterGraph.addEvent(filteredpairs.hitlist);
    }
  }
}

void FPGATrackSimGenScanMonitoring::fillBuildGroupsWithPairs(
    const std::vector<FPGATrackSimGenScanTool::IntermediateState> &states,
    unsigned allowed_misses) {

  for (unsigned i = 0; i < states.size(); i++) {
    m_unpairedHits->Fill(i, states[i].unpairedHits.size());
    m_pairsetsIncr->Fill(i, states[i].pairsets.size());
    for (auto& pair : states[i].pairsets) {
      m_pairsetsHits->Fill(i, pair.hitlist.size());
    }
    unsigned totalInput = states[i].pairsets.size() +states[i].unpairedHits.size();
    m_totalInputIncr->Fill(i, totalInput);
    m_binStagesIncr->Fill(i, (i <= allowed_misses)||(totalInput>0));
  }
}




// Event Disply Implementation
////////////////////////////////////////////////////////////
TGraph* FPGATrackSimGenScanMonitoring::eventDispSet::initGraph(const std::string& name)
{
  TGraph *retv = new TGraph();
  retv->SetName(name.c_str());
  return retv;
}

void FPGATrackSimGenScanMonitoring::eventDispSet::AddPoint(TGraph *g, double x, double y)
{
  g->SetPoint(g->GetN(), x, y);
}

void FPGATrackSimGenScanMonitoring::eventDispSet::addEvent(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits)
{
  unsigned int count = m_rZ.size(); // both graph sets should have the same size

  // If over limit on number of event displays just return
  if (count > m_maxEvts) return;

  m_rZ.push_back(initGraph("EventDisp_RZ_"+m_name+std::to_string(count)));
  m_xY.push_back(initGraph("EventDisp_XY_"+m_name+std::to_string(count)));
  for (auto & hit: hits) 
  {
    AddPoint(m_rZ.back(), hit->getZ(), hit->getR());
    AddPoint(m_xY.back(), hit->getX(), hit->getY());
  }
}

void FPGATrackSimGenScanMonitoring::eventDispSet::addEvent(const std::vector<FPGATrackSimGenScanTool::StoredHit>& hits) 
{
  // If over limit on number of event displays just return
  if (m_rZ.size() > m_maxEvts) return;

  // convert vector of instances to vector of pointers to instances
  std::vector<const FPGATrackSimGenScanTool::StoredHit *> ptrvec;
  for (auto & hit: hits) 
  {
    ptrvec.push_back(&hit);
  }

  // then just call pointer-based implementation
  return addEvent(ptrvec);
}

void FPGATrackSimGenScanMonitoring::eventDispSet::addEvent(const std::vector<const FPGATrackSimGenScanTool::StoredHit*>& hits) 
{
  unsigned int count = m_rZ.size(); // all four graph sets should have the same size

  // If over limit on number of event displays just return
  if (count > m_maxEvts) return;

  // make the graphs to store the event display points
  m_rZ.push_back(initGraph("EventDisp_RZ_" + m_name + std::to_string(count)));
  m_xY.push_back(initGraph("EventDisp_XY_"+m_name+std::to_string(count)));
  m_rPhi.push_back(initGraph("EventDisp_RPhi_"+m_name+std::to_string(count)));
  m_rEta.push_back(initGraph("EventDisp_REta_"+m_name+std::to_string(count)));

  for (auto & hit : hits)
  {
    AddPoint(m_rZ.back(), hit->hitptr->getZ(), hit->hitptr->getR());
    AddPoint(m_xY.back(), hit->hitptr->getX(), hit->hitptr->getY());
    AddPoint(m_rEta.back(), hit->etaShift, hit->hitptr->getR());
    AddPoint(m_rPhi.back(), hit->phiShift, hit->hitptr->getR());
  }
}
            
StatusCode FPGATrackSimGenScanMonitoring::eventDispSet::registerGraphs(FPGATrackSimGenScanMonitoring* parent) {
  // Make a vector of all the vectors of graphs and then loop to register them all...
  for (auto &graphs :
       std::vector<std::vector<TGraph *> *>({&m_rZ, &m_xY, &m_rPhi, &m_rEta})) {
    for (auto& g : *graphs) { ATH_CHECK(parent->regGraph(g));}
  }
  return StatusCode::SUCCESS;
}
