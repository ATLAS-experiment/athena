// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanTool.cxx
 * @author Elliot Lipeles
 * @date Sept 6th, 2024
 * @brief See header file.
 */

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimGenScanTool.h"
#include "FPGATrackSimGenScanMonitoring.h"

#include <sstream>
#include <cmath>
#include <algorithm>


#include <nlohmann/json.hpp>

#include "TH1.h"



///////////////////////////////////////////////////////////////////////////////
// Debug Print Tools
template<class T>
static inline std::string to_string(const std::vector<T>& v)
{
  std::ostringstream oss;
  oss << "[";
  if (!v.empty())
  {
    std::copy(v.begin(), v.end() - 1, std::ostream_iterator<T>(oss, ", "));
    oss << v.back();
  }
  oss << "]";
  return oss.str();
}


///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGenScanTool::FPGATrackSimGenScanTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
  declareInterface<IFPGATrackSimRoadFinderTool>(this);
}


StatusCode FPGATrackSimGenScanTool::initialize()
{
  // Dump the configuration to make sure it propagated through right
  const std::vector<Gaudi::Details::PropertyBase*> props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }

  // Retrieve info
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  ATH_MSG_INFO("Map specifies :" << m_binnedhits->getNLayers());
  ATH_CHECK(m_binnedhits.retrieve());
  ATH_CHECK(m_monitoring.retrieve());
  ATH_MSG_INFO("Monitoring Dir :" << m_monitoring->dir());

  // Setup layer configuration if not already set from layerMap
  if (m_binnedhits->getNLayers()==0)
    m_binnedhits->setNLayers(m_FPGATrackSimMapping->PlaneMap_1st(getSubRegion())->getNLogiLayers());

  // This is the layers they get paired with previous layers
  for (unsigned lyr = 0; lyr < m_binnedhits->getNLayers(); ++lyr) m_pairingLayers.push_back(lyr);
  if (m_reversePairDir) {
    std::reverse(m_pairingLayers.begin(),m_pairingLayers.end());
  }
  ATH_MSG_INFO("Pairing Layers: " << m_pairingLayers);
  
  // Check inputs
  bool ok = false;
  if (m_pairFilterDeltaPhiCut.size() != m_binnedhits->getNLayers() - 1)
    ATH_MSG_FATAL("initialize() pairFilterDeltaPhiCut must have size nLayers-1=" << m_binnedhits->getNLayers() - 1 << " found " << m_pairFilterDeltaPhiCut.size());
  else if (m_pairFilterDeltaEtaCut.size() != m_binnedhits->getNLayers() - 1)
    ATH_MSG_FATAL("initialize() pairFilterDeltaEtaCut must have size nLayers-1=" << m_binnedhits->getNLayers() - 1 << " found " << m_pairFilterDeltaEtaCut.size());
  else if (m_pairFilterPhiExtrapCut.size() != 2)
    ATH_MSG_FATAL("initialize() pairFilterPhiExtrapCut must have size 2 found " << m_pairFilterPhiExtrapCut.size());
  else if (m_pairFilterEtaExtrapCut.size() != 2)
    ATH_MSG_FATAL("initialize() pairFilterEtaExtrapCut must have size 2 found " << m_pairFilterEtaExtrapCut.size());
  else if (m_pairSetPhiExtrapCurvedCut.size() != 2)
    ATH_MSG_FATAL("initialize() PairSetPhiExtrapCurvedCut must have size 2found " << m_pairSetPhiExtrapCurvedCut.size());
  else if ((m_rin < 0.0) || (m_rout < 0.0))
    ATH_MSG_FATAL("Radii not set");
  else
    ok = true;
  if (!ok)
    return StatusCode::FAILURE;

  
  // register histograms
  ATH_CHECK(m_monitoring->registerHistograms(m_binnedhits.get()));

  // write out the firmware LUTs
  m_binnedhits->getBinTool().writeLUTs();

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimGenScanTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits,
                                             std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads)
{
  ATH_MSG_DEBUG("In getRoads, Processing Event# " << ++m_evtsProcessed << " hit size = " << hits.size());

  roads.clear();
  m_roads.clear();
  m_monitoring->resetDataFlowCounters();
  
  // Currently assume that if less than 100 hits its a single track MC
  m_monitoring->parseTruthInfo(getTruthTracks(),(hits.size() < 100));

  // do the binning...
  ATH_CHECK(m_binnedhits->fill(hits));
  m_monitoring->fillBinningSummary(hits);

  // scan over image building pairs for bins over threshold
  for (FPGATrackSimBinArray<BinEntry>::ConstIterator &bin : m_binnedhits->lastStepBinnedHits())
  {
    // apply threshold
    if (bin.data().lyrCnt() < m_threshold) continue;
    ATH_MSG_DEBUG("Bin passes threshold " << bin.data().lyrCnt() << " "
                                          << bin.idx());  

    // Monitor contents of bins passing threshold
    m_monitoring->fillBinLevelOutput(bin.idx(), bin.data());
    if (m_binningOnly) continue;
      
    // pass hits for bin to filterRoad and get back pairs of hits grouped into pairsets
    std::vector<HitPairSet> pairsets;
    if (m_binFilter=="PairThenGroup") {
      ATH_CHECK(pairThenGroupFilter(bin.data(), pairsets));
    } else if (m_binFilter=="IncrementalBuild") {
      ATH_CHECK(incrementalBuildFilter(bin.data(), pairsets));
    } else {
      ATH_MSG_FATAL("Unknown bin filter" << m_binFilter);
    }
    ATH_MSG_DEBUG("grouped PairSets " << pairsets.size());

    // convert the group pairsets to FPGATrackSimRoads
    for (const FPGATrackSimGenScanTool::HitPairSet& pairset: pairsets)
      {      
        addRoad(pairset.hitlist, bin.idx());
        ATH_MSG_DEBUG("Output road size=" <<pairset.hitlist.size());

        // debug statement if more than one road found in bin
        // not necessarily a problem
        if (pairsets.size() >1) {
          std::string s = "";
          for (const FPGATrackSimBinUtil::StoredHit* const hit : pairset.hitlist)
          {
            s += "(" + std::to_string(hit->layer) + "," + std::to_string(hit->hitptr->getR()) + "), ";
          }        
          ATH_MSG_DEBUG("Duplicate Group " << s);
        }
      }  
  }

  // copy roads to output vector
  roads.reserve(m_roads.size());  
  for (auto & r : m_roads) roads.push_back(std::move(r));
  ATH_MSG_DEBUG("Roads = " << roads.size());

  // clear previous event
  // (reusing saves having to reallocate memory for each event)
  m_binnedhits->resetBins();

  m_evtsProcessed++;
  return StatusCode::SUCCESS;
}

// Filter the bins above threshold into pairsets which output roads
StatusCode FPGATrackSimGenScanTool::pairThenGroupFilter(const BinEntry &bindata,
                                    std::vector<HitPairSet> &output_pairsets)
{
  ATH_MSG_VERBOSE("In pairThenGroupFilter");

  // Organize Hits by Layer
  std::vector<std::vector<const StoredHit *>> hitsByLayer(m_binnedhits->getNLayers());
  ATH_CHECK(sortHitsByLayer(bindata, hitsByLayer));
  
  // This is monitoring for each bin over threshold
  // It's here so it can get the hitsByLayer
  m_monitoring->fillHitsByLayer(hitsByLayer);

  // Make Pairs
  HitPairSet pairs;
  ATH_CHECK(makePairs(hitsByLayer, pairs));
  
  // Filter Pairs
  HitPairSet filteredpairs;
  ATH_CHECK(filterPairs(pairs, filteredpairs));

  // Require road is still over threshold
  bool passedPairFilter = (filteredpairs.lyrCnt() >= m_threshold);
  m_monitoring->pairFilterCheck(pairs, filteredpairs, passedPairFilter);

  // if passed Pair Filter proceed to group the filtered pairs into pairsets
  if (passedPairFilter)
  {
    // Pair Set Grouping
    std::vector<HitPairSet> pairsets;
    ATH_CHECK(groupPairs(filteredpairs, pairsets, false));

    // loop over pairsets and find those that are over thresold
    for (const FPGATrackSimGenScanTool::HitPairSet& pairset : pairsets)
    {
      // if over threshold add it to the output
      if (pairset.lyrCnt() >= m_threshold)
      {
        if (m_applyPairSetFilter) {
          output_pairsets.push_back(pairset);
        }
      }
    }
  }

  // set outputs if not all filters applied
  if (!m_applyPairFilter) {
    // output is just the filtered pairs
    output_pairsets.push_back(pairs);
  }
  else if (passedPairFilter && !m_applyPairSetFilter)
  {
    // output is just the filtered pairs
    output_pairsets.push_back(filteredpairs);
  }

  return StatusCode::SUCCESS;
}


void FPGATrackSimGenScanTool::updateState(const IntermediateState &inputstate,
                                          IntermediateState &outputstate,
                                          unsigned lyridx,
                                          const std::vector<const StoredHit *>& newhits)
{
  unsigned int allowed_missed_hits = m_binnedhits->getNLayers() - m_threshold;

  std::vector<bool> pairset_used(inputstate.pairsets.size(),false);
  
  for (auto &newhit : newhits) {

    // don't make new pairs with hits that the new hit is already paired with in a group
    std::set<const StoredHit *> vetoList;

    // try adding hit to existing pair sets
    for (unsigned ps_idx = 0; ps_idx < inputstate.pairsets.size(); ++ps_idx) {
      auto &pairset = inputstate.pairsets[ps_idx];
      HitPair nextpair(pairset.lastpair().second, newhit, m_reversePairDir);
      if (pairMatchesPairSet(pairset, nextpair, false) || (m_applyPairSetFilter==false)) {
        HitPairSet newset(pairset);
        newset.addPair(nextpair);
        pairset_used[ps_idx]=true;
        outputstate.pairsets.push_back(newset);
        // put inpair hits in list of hits not to pair again with the new hits
        for (auto vetohit : pairset.hitlist) {
          vetoList.insert(vetohit);
        }
      }
    }

    // make new pairsets with unpaired hits
    for (auto prevhit : inputstate.unpairedHits) {
      if (vetoList.count(prevhit) == 0) {
        HitPair newpair(prevhit, newhit, m_reversePairDir);
        if (pairPassesFilter(newpair) || (m_applyPairFilter == false)) {
          HitPairSet newset;
          newset.addPair(newpair);
          outputstate.pairsets.push_back(newset);
        }
      }
    }

    // if this can be the start of a the start of a track  and still
    // have enough hits to make a track, add it to the unpaired hits list
    if (lyridx <= allowed_missed_hits) {
      outputstate.unpairedHits.push_back(newhit);
    }
  }

  // Add groups to output without new hit if they have enough hits to skip
  // this layer. Note expected hits at this point is lyridx+1, since we start
  // counting lyridx from zero. Logic is then keep the pairset if
  // expected hits <= hits in set + allows misses
  for (unsigned ps_idx = 0; ps_idx < inputstate.pairsets.size(); ++ps_idx) {
    auto &pairset = inputstate.pairsets[ps_idx];
    if ((!pairset_used[ps_idx])&&(lyridx < (pairset.hitlist.size() + allowed_missed_hits))) {
        outputstate.pairsets.push_back(pairset);
    }
  }

  // Add hits to unpaired list if hits are still allowed to start a track
  // ---------------------------------------------------------------------
  // If you start a new track with a pair whose second element is
  // layer N (layer numbering starting from zero), then you'll have missed N-1
  // layers Minus 1 because the first hit was one of the N layers already
  // passed.
  //
  // The next pass will be layer N=lyridx+1 where lyridx is the current value
  // at this point in the code. You will have then missed lyridx layers, so...
  // E.g. if you allow one missed layer then this stops putting hits in the
  // unpairedHits list if lyridx>1, so tracks must start with layer 0 or 1,
  // which makes sense
  if (lyridx > allowed_missed_hits) {
    // make no new track starts
    outputstate.unpairedHits.clear();
  } else {
    // copy in any previous unpairedHits as well
    for (auto prevhit : inputstate.unpairedHits) {
      outputstate.unpairedHits.push_back(prevhit);
    }
  }
}

// Filter the bins above threshold into pairsets which output roads
StatusCode FPGATrackSimGenScanTool::incrementalBuildFilter(const BinEntry &bindata,
                                    std::vector<HitPairSet> &output_pairsets)
{
  ATH_MSG_VERBOSE("In buildGroupsWithPairs");

  // Organize Hits by Layer
  std::vector<std::vector<const StoredHit *>> hitsByLayer(m_binnedhits->getNLayers());
  ATH_CHECK(sortHitsByLayer(bindata, hitsByLayer));
  
  // This is monitoring for each bin over threshold
  // It's here so it can get the hitsByLayer
  m_monitoring->fillHitsByLayer(hitsByLayer);

  std::vector<IntermediateState> states{m_binnedhits->getNLayers()+1};
  for (unsigned lyridx = 0; lyridx < m_binnedhits->getNLayers(); lyridx++) {
    unsigned lyr = m_pairingLayers[lyridx];
    updateState(states[lyridx] , states[lyridx+1], lyridx, hitsByLayer[lyr]);
  }

  // this is a little ugly because it requires copying the output pairsets right now
  output_pairsets=states[m_binnedhits->getNLayers()].pairsets;

  m_monitoring->fillBuildGroupsWithPairs(states,m_binnedhits->getNLayers()-m_threshold);
  
  return StatusCode::SUCCESS;
}


// 1st step of filter: sort hits by layer
StatusCode FPGATrackSimGenScanTool::sortHitsByLayer(const BinEntry &bindata,
                                         std::vector<std::vector<const StoredHit *>>& hitsByLayer)
{
  ATH_MSG_DEBUG("In fillHitsByLayer");

  for (const FPGATrackSimBinUtil::StoredHit &hit : bindata.hits) {    
    hitsByLayer.at(hit.layer).push_back(&hit);    
  }

  return StatusCode::SUCCESS;
}





// 2nd step of filter: make pairs of hits from adjacent and next-to-adjacent layers
StatusCode FPGATrackSimGenScanTool::makePairs(const std::vector<std::vector<const StoredHit *>>& hitsByLayer,
                                   HitPairSet &pairs)
{
  ATH_MSG_VERBOSE("In makePairs");

  std::vector<const StoredHit *> const *  lastlyr = 0;
  std::vector<const StoredHit *> const *  lastlastlyr = 0;

  // order here is designed so lower radius hits come first
  for (unsigned lyr : m_pairingLayers) {
    for (const StoredHit *const &ptr1 :
         hitsByLayer[lyr]) {
      if (lastlyr) {
        for (const StoredHit *const &ptr2 : *lastlyr) {
          pairs.addPair(HitPair(ptr2, ptr1,m_reversePairDir));
        }
        // Add Pairs that skip one layer
        if (lastlastlyr) {
          for (const StoredHit *const &ptr2 : *lastlastlyr) {
            pairs.addPair(HitPair(ptr2, ptr1,m_reversePairDir));
          }
        }
      }
    }
    lastlastlyr = lastlyr;
    lastlyr = &hitsByLayer[lyr];    
    m_monitoring->fillPairingHits(lastlyr,lastlastlyr);
  }

  return StatusCode::SUCCESS;
}

// 3rd step of filter: make cuts on the pairs to ensure that are consisten with
// the bin they are in

bool FPGATrackSimGenScanTool::pairPassesFilter(const HitPair &pair) {
  m_monitoring->fillPairFilterCuts(pair,m_rin,m_rout);
  int lyr = std::min(pair.first->layer,pair.second->layer);
  return (std::abs(pair.dPhi()) < m_pairFilterDeltaPhiCut[lyr]) &&
        (std::abs(pair.dEta()) < m_pairFilterDeltaEtaCut[lyr]) &&
        (std::abs(pair.PhiInExtrap(m_rin)) < m_pairFilterPhiExtrapCut[0]) &&
        (std::abs(pair.PhiOutExtrap(m_rout)) < m_pairFilterPhiExtrapCut[1]) &&
        (std::abs(pair.EtaInExtrap(m_rin)) < m_pairFilterEtaExtrapCut[0]) &&
        (std::abs(pair.EtaOutExtrap(m_rout)) < m_pairFilterEtaExtrapCut[1]);
}

StatusCode FPGATrackSimGenScanTool::filterPairs(HitPairSet &pairs, HitPairSet &filteredpairs)
{
  ATH_MSG_VERBOSE("In filterPairs");

  for (const FPGATrackSimGenScanTool::HitPair &pair : pairs.pairList) {
    if (pairPassesFilter(pair)) {
      filteredpairs.addPair(pair);
    }
  }
  return StatusCode::SUCCESS;
}


// 4th step of filter: group pairs into sets where they are all consistent with being from the same track
StatusCode FPGATrackSimGenScanTool::groupPairs(HitPairSet &filteredpairs,
                                      std::vector<HitPairSet> &pairsets,
                                      bool verbose)
{
  ATH_MSG_VERBOSE("In groupPairs");
  for (const FPGATrackSimGenScanTool::HitPair & pair : filteredpairs.pairList)
  {
    bool added = false;
    for (FPGATrackSimGenScanTool::HitPairSet &pairset : pairsets)
    {      
      // Only add skip pairs if skipped layer is not already hit
      if ((std::abs(int(pair.second->layer) - int(pair.first->layer)) > 1) // gives if is a skip pair
          && (pairset.hasLayer(std::min(pair.first->layer,pair.second->layer) + 1))) // gives true if skipped layer already in set
      {
        // if it matches mark as added so it doesn't start a new pairset
        // false here is so it doesn't plot these either
        if (pairMatchesPairSet(pairset, pair, verbose))
          added = true;
        if (!added) {
          ATH_MSG_VERBOSE("Skip pair does not match non-skip pair");
        }
      }
      else
      {
        if (pairMatchesPairSet(pairset, pair, verbose))
        {
          int size = pairset.addPair(pair);
          if (verbose)
            ATH_MSG_VERBOSE("addPair " << pairsets.size() << " " << pairset.pairList.size() << " " << size);
          added = true;
        }
      }      

    }

    if (!added)
    {
      HitPairSet newpairset;
      newpairset.addPair(pair);
      pairsets.push_back(newpairset);
    }
  }
  
  return StatusCode::SUCCESS;

}

// used to determine if a pair is consistend with a pairset
bool FPGATrackSimGenScanTool::pairMatchesPairSet(const HitPairSet &pairset,
                                                 const HitPair &pair,
                                                 bool verbose) {
  // In order to make it easy to have a long list of possible cuts,
  // a vector of cutvar structs is used to represent each cut
  // then apply the AND of all the cuts is done with a std::count_if function

  // define the struct (effectively mapping because variable, configured cut value, 
  // and histograms for plotting
  struct cutvar {
    cutvar(std::string name, double val, double cut, std::vector<TH1D *>& histset) :
       m_name(std::move(name)), m_val(val), m_cut(cut), m_histset(histset) {}
    bool passed() { return std::abs(m_val) < m_cut; }
    void fill(unsigned cat) {  m_histset[cat]->Fill(m_val); }
    std::string m_name;
    double m_val;
    double m_cut;
    std::vector<TH1D *> &m_histset;
  };

  // add the cuts to the list of all cuts
  std::vector<cutvar> allcuts;
  allcuts.push_back(cutvar("MatchPhi", pairset.MatchPhi(pair),
                           m_pairSetMatchPhiCut,
                           m_monitoring->m_pairSetMatchPhi));
  allcuts.push_back(cutvar("MatchEta", pairset.MatchEta(pair),
                           m_pairSetMatchEtaCut,
                           m_monitoring->m_pairSetMatchEta));
  allcuts.push_back(cutvar("DeltaDeltaPhi", pairset.DeltaDeltaPhi(pair),
                           m_pairSetDeltaDeltaPhiCut,
                           m_monitoring->m_deltaDeltaPhi));
  allcuts.push_back(cutvar("DeltaDeltaEta", pairset.DeltaDeltaEta(pair),
                           m_pairSetDeltaDeltaEtaCut,
                           m_monitoring->m_deltaDeltaEta));
  allcuts.push_back(cutvar("PhiCurvature", pairset.PhiCurvature(pair),
                           m_pairSetPhiCurvatureCut,
                           m_monitoring->m_phiCurvature));
  allcuts.push_back(cutvar("EtaCurvature", pairset.EtaCurvature(pair),
                           m_pairSetEtaCurvatureCut,
                           m_monitoring->m_etaCurvature));
  if (pairset.pairList.size() > 1) {
    allcuts.push_back(cutvar(
        "DeltaPhiCurvature", pairset.DeltaPhiCurvature(pair),
        m_pairSetDeltaPhiCurvatureCut, m_monitoring->m_deltaPhiCurvature));
    allcuts.push_back(cutvar(
        "DeltaEtaCurvature", pairset.DeltaEtaCurvature(pair),
        m_pairSetDeltaEtaCurvatureCut, m_monitoring->m_deltaEtaCurvature));
  }
  allcuts.push_back(cutvar(
      "PhiInExtrapCurved", pairset.PhiInExtrapCurved(pair, m_rin),
      m_pairSetPhiExtrapCurvedCut[0], m_monitoring->m_phiInExtrapCurved));
  allcuts.push_back(cutvar(
      "PhiOutExtrapCurved", pairset.PhiOutExtrapCurved(pair, m_rout),
      m_pairSetPhiExtrapCurvedCut[1], m_monitoring->m_phiOutExtrapCurved));

  // count number of cuts passed
  unsigned passedCuts = std::count_if(allcuts.begin(), allcuts.end(),
                                      [](cutvar& cut) { return cut.passed(); });
  bool passedAll = (passedCuts == allcuts.size());

  // monitoring
  bool passedAllButOne = (passedCuts == allcuts.size() - 1);
  for (cutvar& cut: allcuts) {
    // the last value computes if an n-1 histogram should be filled
    m_monitoring->fillPairSetFilterCut(cut.m_histset, cut.m_val, pair,
                                        pairset.lastpair(),
                                        (passedAll || (passedAllButOne && !cut.passed())));
  }

  if (verbose)
  {
    std::string s = "";
    for (cutvar &cut : allcuts)
    {
      s += cut.m_name + " : (" + cut.passed() + ", " + cut.m_val + "),  ";
    }
    ATH_MSG_DEBUG("PairSet test " << passedAll << " " << s);
    ATH_MSG_DEBUG("Hits: \n   " << *pairset.lastpair().first << "\n   "
                               << *pairset.lastpair().second << "\n   "
                               << *pair.first << "\n   "
                               << *pair.second);
  }

  return passedAll;
}

// format final pairsets into expected output of getRoads
void FPGATrackSimGenScanTool::addRoad(std::vector<const StoredHit *> const &hits, const FPGATrackSimBinUtil::IdxSet &idx)
{
  layer_bitmask_t hitLayers = 0;
  std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>
      sorted_hits(m_binnedhits->getNLayers(),std::vector<std::shared_ptr<const FPGATrackSimHit>>());
  for (const FPGATrackSimBinUtil::StoredHit* hit : hits)
  {
    hitLayers |= 1 << hit->layer;
    sorted_hits[hit->layer].push_back(hit->hitptr);
  }

  m_roads.emplace_back(std::make_unique<FPGATrackSimRoad>());
  FPGATrackSimRoad *r = m_roads.back().get();

  r->setRoadID(m_roads.size() - 1);
  //    r.setPID(y * m_imageSize_y + x);
  r->setHits(std::move(sorted_hits));

  FPGATrackSimBinUtil::ParSet binCenterPars = m_binnedhits->getBinTool().lastStep()->binCenter(idx);
  FPGATrackSimTrackPars trackpars = m_binnedhits->getBinTool().binDesc()->parSetToTrackPars(binCenterPars);
  r->setX(trackpars[FPGATrackSimTrackPars::IPHI]);
  r->setY(trackpars[FPGATrackSimTrackPars::IHIP]);
  r->setXBin(idx[3]);
  r->setYBin(idx[4]);
  r->setHitLayers(hitLayers);
  r->setSubRegion(0);
}


///////////////////////////////////////////////////////////////////////
// HitPair and HitPairSet Storage

bool FPGATrackSimGenScanTool::HitPairSet::hasHit(const StoredHit * newhit) const
{for (const FPGATrackSimBinUtil::StoredHit*  hit : hitlist)
  {
    if (hit == newhit)
      return true;
  }
  return false;
}

int FPGATrackSimGenScanTool::HitPairSet::addPair(const HitPair &pair)
{
  if (pairList.size() > 0)
  {
    // these need to be done before the pair is added
    LastPhiCurvature = PhiCurvature(pair);
    LastEtaCurvature = EtaCurvature(pair);
  }

  pairList.push_back(pair);

  hitLayers |= (0x1 << pair.first->layer);
  hitLayers |= (0x1 << pair.second->layer);
  if (!hasHit(pair.first))
    hitlist.push_back(pair.first);
  if (!hasHit(pair.second))
    hitlist.push_back(pair.second);
  return this->pairList.size();
}


double FPGATrackSimGenScanTool::HitPairSet::MatchPhi(const HitPair &pair) const
{
  if ((lastpair().first->hitptr==pair.first->hitptr)||
      (lastpair().first->hitptr==pair.second->hitptr)||
      (lastpair().second->hitptr==pair.first->hitptr)||
      (lastpair().second->hitptr==pair.second->hitptr))
    return 0.0;

  double dr = (lastpair().second->hitptr->getR() - pair.first->hitptr->getR()) / 2.0;
  double lastpairextrap = lastpair().second->phiShift - lastpair().dPhi() / lastpair().dR() * dr;
  double newpairextrap = pair.first->phiShift + pair.dPhi() / pair.dR() * dr;
  return lastpairextrap - newpairextrap;
}

double FPGATrackSimGenScanTool::HitPairSet::MatchEta(const HitPair &pair) const
{
  if ((lastpair().first->hitptr==pair.first->hitptr)||
      (lastpair().first->hitptr==pair.second->hitptr)||
      (lastpair().second->hitptr==pair.first->hitptr)||
      (lastpair().second->hitptr==pair.second->hitptr))
    return 0.0;

  double dr = (lastpair().second->hitptr->getR() - pair.first->hitptr->getR()) / 2.0;
  double lastpairextrap = lastpair().second->etaShift - lastpair().dEta() / lastpair().dR() * dr;
  double newpairextrap = pair.first->etaShift + pair.dEta() / pair.dR() * dr;
  return lastpairextrap - newpairextrap;
}

double
FPGATrackSimGenScanTool::HitPairSet::DeltaDeltaPhi(const HitPair &pair) const {
  return (pair.dPhi() * lastpair().dR() - lastpair().dPhi() * pair.dR()) / (lastpair().dR() * pair.dR());
}

double FPGATrackSimGenScanTool::HitPairSet::DeltaDeltaEta(const HitPair &pair) const
{
  return (pair.dEta() * lastpair().dR() - lastpair().dEta() * pair.dR()) / (lastpair().dR() * pair.dR());
}

double FPGATrackSimGenScanTool::HitPairSet::PhiCurvature(const HitPair &pair) const
{
  return 2 * DeltaDeltaPhi(pair) / (lastpair().dR() + pair.dR());
}
double FPGATrackSimGenScanTool::HitPairSet::EtaCurvature(const HitPair &pair) const
{
  return 2 * DeltaDeltaEta(pair) / (lastpair().dR() + pair.dR());
}
double FPGATrackSimGenScanTool::HitPairSet::DeltaPhiCurvature(const HitPair &pair) const
{
  return PhiCurvature(pair) - LastPhiCurvature;
}
double FPGATrackSimGenScanTool::HitPairSet::DeltaEtaCurvature(const HitPair &pair) const
{
  return EtaCurvature(pair) - LastEtaCurvature;
}
double FPGATrackSimGenScanTool::HitPairSet::PhiInExtrapCurved(const HitPair &pair, double r_in) const
{
  double r = std::min(lastpair().first->hitptr->getR(),lastpair().second->hitptr->getR());
  return lastpair().PhiInExtrap(r_in) + 0.5 * PhiCurvature(pair) * (r_in - r) * (r_in - r);
}
double FPGATrackSimGenScanTool::HitPairSet::PhiOutExtrapCurved(const HitPair &pair, double r_out) const
{
  double r = std::max(lastpair().first->hitptr->getR(),lastpair().second->hitptr->getR());
  return pair.PhiOutExtrap(r_out) + 0.5 * PhiCurvature(pair) * (r_out - r) * (r_out - r);
}
