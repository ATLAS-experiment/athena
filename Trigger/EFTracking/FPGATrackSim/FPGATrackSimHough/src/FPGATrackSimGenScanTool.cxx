// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanTool.cxx
 * @author Elliot Lipeles
 * @date Sept 6th, 2024
 * @brief See header file.
 */

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


std::ostream& operator<<(std::ostream &os, const FPGATrackSimGenScanTool::StoredHit &hit)
{
  os << "lyr: " << hit.layer << " ";
  os << "(" << hit.hitptr->getR() << ", " << hit.hitptr->getGPhi() << ", " << hit.hitptr->getZ() << ") ";
  os << "[" << hit.phiShift << ", " << hit.etaShift << "]";
  return os;
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
  ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st(getSubRegion())->getNLogiLayers();
  ATH_MSG_INFO("Map specifies :" << m_nLayers);
  ATH_CHECK(m_monitoring.retrieve());
  ATH_MSG_INFO("Monitoring Dir :" << m_monitoring->dir());
  
  // Check inputs
 bool ok = false;
  if (m_pairFilterDeltaPhiCut.size() != m_nLayers - 1)
    ATH_MSG_FATAL("initialize() pairFilterDeltaPhiCut must have size nLayers-1=" << m_nLayers - 1 << " found " << m_pairFilterDeltaPhiCut.size());
  else if (m_pairFilterDeltaEtaCut.size() != m_nLayers - 1)
    ATH_MSG_FATAL("initialize() pairFilterDeltaEtaCut must have size nLayers-1=" << m_nLayers - 1 << " found " << m_pairFilterDeltaEtaCut.size());
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


  // Build Binning Object
  if (m_parSet == "StdTrkPars")
  {
    m_binning = new FPGATrackSimGenScanStdTrkBinning();
  }
  else if (m_parSet == "KeyLyrPars")
  {
    m_binning = new FPGATrackSimGenScanKeyLyrBinning(m_rin, m_rout);
  }
  else if (m_parSet == "PhiSlicedKeyLyrPars")
  {
    m_binning = new FPGATrackSimGenScanPhiSlicedKeyLyrBinning(m_rin, m_rout);
  }
  else
  {
    ATH_MSG_FATAL("Unknown binning: " << m_parSet);
    return StatusCode::FAILURE;
  }
  
  // Dump Binning
  for (unsigned par : m_binning->slicePars()) { ATH_MSG_INFO("Slice Par: " << m_binning->parNames(par)); }
  for (unsigned par : m_binning->scanPars()) { ATH_MSG_INFO("Scan Par: " << m_binning->parNames(par)); }
  ATH_MSG_INFO("Row Par: " << m_binning->parNames(m_binning->rowParIdx()));

  // Configure Binning
  for (unsigned par = 0; par < FPGATrackSimGenScanBinningBase::NPars; par++)
  {    
    m_binning->m_parMin[par] = m_parMin[par];
    m_binning->m_parMax[par] = m_parMax[par];
    m_binning->m_parBins[par] = m_parBins[par];
    if (m_parBins[par] <= 0)
    {
      ATH_MSG_FATAL("Every dimension must be at least one bin");
      return StatusCode::FAILURE;
    }
    m_binning->m_parStep[par] = (m_parMax[par] - m_parMin[par]) / m_parBins[par];
  }

  // Build Image
  m_image.setsize(m_binning->m_parBins, BinEntry());
  ATH_MSG_INFO("Final Image Size: " << m_image.size());
  // make sure image is starts reset
  // (reusing saves having to reallocate memory for each event)
  for (FPGATrackSimGenScanArray<BinEntry>::Iterator bin : m_image)
  {
    bin.data().reset();
  }

  // Compute which bins correspond to track parameters that are in the region
  // i.e. the pT, eta, phi, z0 and d0 bounds
  computeValidBins();

  // register histograms
  ATH_CHECK(m_monitoring->registerHistograms(m_nLayers, m_binning, m_rout, m_rin));

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Internal Storage Classes

void FPGATrackSimGenScanTool::BinEntry::reset()
{
  hitCnt = 0;
  lyrhit = 0;
  hits.clear();
}

void FPGATrackSimGenScanTool::BinEntry::addHit(StoredHit hit)
{
  hitCnt++;
  if (((lyrhit >> hit.layer) & 0x1) == 0x0)
  {
    lyrhit |= (0x1 << hit.layer);
  }
  hits.push_back(hit);
}

///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimGenScanTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits,
                                             std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads)
{
  ATH_MSG_DEBUG("In getRoads, Processing Event# " << ++m_evtsProcessed << " hit size = " << hits.size());

  roads.clear();
  m_roads.clear();
  
  // Currently assume that if less than 100 hits its a single track MC
  m_monitoring->parseTruthInfo(getTruthTracks(),(hits.size() < 100),m_validBin);
  
  // do the binning...
  ATH_CHECK(fillImage(hits));

  // scan over image building pairs for bins over threshold
  for (FPGATrackSimGenScanArray<BinEntry>::Iterator &bin : m_image)
  {
    // apply threshold
    if (bin.data().lyrCnt() < m_threshold) continue;
    ATH_MSG_DEBUG("newRoad " << bin.data().lyrCnt() << " " << bin.idx());

    // pass hits for bin to filterRoad and get back pairs of hits grouped into pairsets
    std::vector<HitPairSet> pairsets;
    ATH_CHECK(filterRoad(bin.data(), bin.idx(), pairsets));
    ATH_MSG_DEBUG("grouped PairSets " << pairsets.size());

    // convert the group pairsets to FPGATrackSimRoads
    for (const FPGATrackSimGenScanTool::HitPairSet& pairset: pairsets)
      {      
        addRoad(pairset.hitlist, bin.idx());

        // debug statement if more than one road found in bin
        // not necessarily a problem
        if (pairsets.size() >1) {
          std::string s = "";
          for (const FPGATrackSimGenScanTool::StoredHit* const hit : pairset.hitlist)
          {
            s += "(" + std::to_string(hit->hitptr->getLayer()) + "," + std::to_string(hit->hitptr->getR()) + "), ";
          }        
          ATH_MSG_DEBUG("Duplicate Group " << s);
        }
      }  
  }

  // copy roads to output vector
  roads.reserve(m_roads.size());  
  for (FPGATrackSimRoad & r : m_roads) roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
  ATH_MSG_DEBUG("Roads = " << roads.size());
  m_monitoring->fillOutputSummary(m_validSlice, m_validSliceAndScan);

  // clear previous event 
  // (reusing saves having to reallocate memory for each event)
  for (FPGATrackSimGenScanArray<BinEntry>::Iterator& bin : m_image)
  {
    bin.data().reset();
  }

  m_evtsProcessed++;
  return StatusCode::SUCCESS;
}

// Put hits in all track parameter bins they could be a part of (binning is defined
// by m_binning object)
StatusCode FPGATrackSimGenScanTool::fillImage(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits)
{
  ATH_MSG_DEBUG("In fillImage");
  
  for (const std::shared_ptr<const FPGATrackSimHit>& hit : hits)
  {
    m_monitoring->fillHitLevelInput(hit.get());

    // The following code loops sequentially of the slice, then scan, then row parameters
    // skipping invalid slices/scans/row/bins and adding hits if the hits are in the bin
    // according to the m_binning class

    // this will contain current bin idx as it is built from slices and scans
    FPGATrackSimGenScanBinningBase::IdxSet idx;    

    // iterate over slices
    for (FPGATrackSimGenScanArray<int>::Iterator slicebin : m_validSlice)
    {
      // if slice is not valid continue
      if (!slicebin.data()) continue; 

      // set the slice bins in the current bin idx object
      ATH_CHECK(m_binning->setIdxSubVec(idx, m_binning->slicePars(), slicebin.idx()));

      // if hit is not in slice skip slice (continue)
      if (!m_binning->hitInSlice(idx, hit.get()))
      {
        m_monitoring->sliceCheck(slicebin.idx());
        continue;
      }

      m_monitoring->incrementInputPerSlice(slicebin.idx());

      // iterate over scan bins
      for (FPGATrackSimGenScanArray<int>::Iterator scanbin : m_validScan) {

        // if scan bin is not valid continue
        if (!scanbin.data()) continue; // scan bin not valid

        // set the scan bins in the current bin idx object
        ATH_CHECK(m_binning->setIdxSubVec(idx, m_binning->scanPars(), scanbin.idx()));

        // Find the min/max bins for hit in the row
        std::pair<unsigned, unsigned> rowRange = m_binning->idxsetToRowParBinRange(idx, hit.get());

        // put hit in row bins according to row range
        for (unsigned rowbin = rowRange.first; rowbin < rowRange.second; rowbin++)
        {
          idx[m_binning->rowParIdx()] = rowbin;
    
          // if full bin is not valid continue
          if (!m_validBin[idx]) continue;
          
          // make a stored hit and fill it with the eta/phishifts as
          // speficied by the Binning class
          StoredHit s_hit; 
          s_hit.hitptr = hit;
          s_hit.layer = hit->getLayer(); 
          s_hit.phiShift = m_binning->phiShift(idx, hit.get());
          s_hit.etaShift = m_binning->etaShift(idx, hit.get());
          
          // add hit to the BinEntry for the bin
          m_image[idx].addHit(s_hit);
        }
      }
    }
  }

  m_monitoring->fillInputSummary(hits, m_validSlice, m_validSliceAndScan);

  return StatusCode::SUCCESS;
}

// Filter the bins above threshold (=roads) into pairsets which output roads
StatusCode FPGATrackSimGenScanTool::filterRoad(const BinEntry &bindata,
                                    const FPGATrackSimGenScanBinningBase::IdxSet &idx,
                                    std::vector<HitPairSet> &output_pairsets)
{
  ATH_MSG_VERBOSE("In filterRoad");

  // Organize Hits by Layer
  std::vector<std::vector<const StoredHit *>> hitsByLayer(m_nLayers);
  ATH_CHECK(sortHitsByLayer(bindata, hitsByLayer));
  
  // This is monitoring for each bin over threshold
  // It's here so it can get the hitsByLayer
  m_monitoring->fillBinLevelOutput(idx, bindata, hitsByLayer);

  // Make Pairs
  HitPairSet pairs;
  ATH_CHECK(makePairs(hitsByLayer, pairs));
  
  // Filter Pairs
  HitPairSet filteredpairs;
  ATH_CHECK(filterPairs(pairs, filteredpairs));
  m_monitoring->fillPairFilterCuts(pairs);

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

// 1st step of filter: sort hits by layer
StatusCode FPGATrackSimGenScanTool::sortHitsByLayer(const BinEntry &bindata,
                                         std::vector<std::vector<const StoredHit *>>& hitsByLayer)
{
  ATH_MSG_VERBOSE("In fillHitsByLayer");

  for (const FPGATrackSimGenScanTool::StoredHit& hit : bindata.hits)
  {
    hitsByLayer[hit.hitptr->getLayer()].push_back(&hit);    
  }

  return StatusCode::SUCCESS;
}


// 2nd step of filter: make pairs of hits from adjacent and next-to-adjacent layers
StatusCode FPGATrackSimGenScanTool::makePairs(const std::vector<std::vector<const StoredHit *>>& hitsByLayer,
                                   HitPairSet &pairs)
{
  ATH_MSG_VERBOSE("In makePairs");


  std::vector<const FPGATrackSimGenScanTool::StoredHit *> const *  lastlastlyr = 0;
  std::vector<const FPGATrackSimGenScanTool::StoredHit *> const *  lastlyr = &hitsByLayer[0];

  // order here is designed so lower radius hits come first
  for (unsigned lyr = 1; lyr < m_nLayers; lyr++)
  {
    for (const FPGATrackSimGenScanTool::StoredHit* const & ptr1 : hitsByLayer[lyr])
    {
      for (const FPGATrackSimGenScanTool::StoredHit* const & ptr2 : *lastlyr)
      {
        pairs.addPair(HitPair(ptr2, ptr1));
      }
      // Add Pairs that skip one layer
      if (lastlastlyr)
      {
        for (const FPGATrackSimGenScanTool::StoredHit* const & ptr2 : *lastlastlyr)
        {
          pairs.addPair(HitPair(ptr2, ptr1));
        }
      }
    }
    lastlastlyr = lastlyr;
    lastlyr = &hitsByLayer[lyr];
  }
  
  return StatusCode::SUCCESS;
}

// 3rd step of filter: make cuts on the pairs to ensure that are consisten with the bin they are in
StatusCode FPGATrackSimGenScanTool::filterPairs(HitPairSet &pairs, HitPairSet &filteredpairs)
{
  ATH_MSG_VERBOSE("In filterPairs");

  for (const FPGATrackSimGenScanTool::HitPair & pair : pairs.pairList)
  {
    int lyr = pair.first->hitptr->getLayer();
    if ((std::abs(pair.dPhi()) < m_pairFilterDeltaPhiCut[lyr]) &&
        (std::abs(pair.dEta()) < m_pairFilterDeltaEtaCut[lyr]) &&
        (std::abs(pair.PhiInExtrap(m_rin)) < m_pairFilterPhiExtrapCut[0]) &&
        (std::abs(pair.PhiOutExtrap(m_rout)) < m_pairFilterPhiExtrapCut[1]) &&
        (std::abs(pair.EtaInExtrap(m_rin)) < m_pairFilterEtaExtrapCut[0]) &&
        (std::abs(pair.EtaOutExtrap(m_rout)) < m_pairFilterEtaExtrapCut[1])) {
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
      if ((pair.second->layer > pair.first->layer + 1) && (pairset.hasLayer(pair.first->layer + 1)))
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
bool FPGATrackSimGenScanTool::pairMatchesPairSet(const HitPairSet& pairset, const HitPair& pair, bool verbose)
{  
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
void FPGATrackSimGenScanTool::addRoad(std::vector<const StoredHit *> const &hits, const FPGATrackSimGenScanBinningBase::IdxSet &idx)
{
  layer_bitmask_t hitLayers = 0;
  std::vector<std::shared_ptr<const FPGATrackSimHit>> outhits;
  for (const FPGATrackSimGenScanTool::StoredHit* hit : hits)
  {
    hitLayers |= 1 << hit->hitptr->getLayer();
    outhits.push_back(hit->hitptr);
  }

  std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>  sorted_hits = ::sortByLayer(outhits);
  sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short

  m_roads.emplace_back();
  FPGATrackSimRoad &r = m_roads.back();

  r.setRoadID(m_roads.size() - 1);
  //    r.setPID(y * m_imageSize_y + x);
  r.setHits(std::move(sorted_hits));
  FPGATrackSimTrackPars trackpars = m_binning->parSetToTrackPars(m_binning->binCenter(idx));
  r.setX(trackpars[FPGATrackSimTrackPars::IPHI]);
  r.setY(trackpars[FPGATrackSimTrackPars::IHIP]);
  r.setXBin(idx[3]);
  r.setYBin(idx[4]);
  r.setHitLayers(hitLayers);
  r.setSubRegion(0);
}


///////////////////////////////////////////////////////////////////////
// HitPair and HitPairSet Storage

bool FPGATrackSimGenScanTool::HitPairSet::hasHit(const StoredHit * newhit) const
{for (const FPGATrackSimGenScanTool::StoredHit*  hit : hitlist)
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

  hitLayers |= (0x1 << pair.first->hitptr->getLayer());
  hitLayers |= (0x1 << pair.second->hitptr->getLayer());
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

double FPGATrackSimGenScanTool::HitPairSet::DeltaDeltaPhi(const HitPair &pair) const
{
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
  double r = lastpair().first->hitptr->getR();
  return lastpair().PhiInExtrap(r_in) + 0.5 * PhiCurvature(pair) * (r_in - r) * (r_in - r);
}
double FPGATrackSimGenScanTool::HitPairSet::PhiOutExtrapCurved(const HitPair &pair, double r_out) const
{
  double r = pair.second->hitptr->getR();
  return pair.PhiOutExtrap(r_out) + 0.5 * PhiCurvature(pair) * (r_out - r) * (r_out - r);
}



// Compute which bins correspond to track parameters that are in the region
// i.e. the pT, eta, phi, z0 and d0 bounds
void FPGATrackSimGenScanTool::computeValidBins() {
  // determine which bins are valid
  m_validBin.setsize(m_binning->m_parBins,false);
  m_validSlice.setsize(m_binning->sliceBins(),false);
  m_validScan.setsize(m_binning->scanBins(),false);
  m_validSliceAndScan.setsize(m_binning->sliceAndScanBins(),false);

  FPGATrackSimTrackPars min_padded;
  FPGATrackSimTrackPars max_padded;
  FPGATrackSimTrackPars padding;
  padding[FPGATrackSimTrackPars::ID0] = m_d0FractionalPadding;
  padding[FPGATrackSimTrackPars::IZ0] = m_z0FractionalPadding;
  padding[FPGATrackSimTrackPars::IETA] = m_etaFractionalPadding;
  padding[FPGATrackSimTrackPars::IPHI] = m_phiFractionalPadding;
  padding[FPGATrackSimTrackPars::IHIP] = m_qOverPtFractionalPadding;
  for (unsigned par = 0; par < FPGATrackSimTrackPars::NPARS; par++)
  {
    min_padded[par] = m_EvtSel->getMin()[par] - padding[par] * (m_EvtSel->getMax()[par]-m_EvtSel->getMin()[par]);
    max_padded[par] = m_EvtSel->getMax()[par] + padding[par] * (m_EvtSel->getMax()[par]-m_EvtSel->getMin()[par]);
    if (par==FPGATrackSimTrackPars::IHIP) {
      // working in units of GeV internally
      min_padded[par] *= 1000;
      max_padded[par] *= 1000;
    }
    ATH_MSG_INFO("Padded Parameter Range: " << FPGATrackSimTrackPars::parName(par)
                                            << " min=" << min_padded[par] << " max=" << max_padded[par]);
  }


  for (FPGATrackSimGenScanArray<int>::Iterator bin : m_validBin) 
  {
    // this finds the parameters at all 2^5 corners of the bin and then finds the min and max of those
    std::vector<FPGATrackSimGenScanBinningBase::ParSet> parsets = m_binning->makeVariationSet(std::vector<unsigned>({0,1,2,3,4}),bin.idx());
    FPGATrackSimTrackPars minvals = m_binning->parSetToTrackPars(m_binning->binCenter(bin.idx()));
    FPGATrackSimTrackPars maxvals = m_binning->parSetToTrackPars(m_binning->binCenter(bin.idx()));
    for (FPGATrackSimGenScanBinningBase::ParSet & parset : parsets) {
      FPGATrackSimTrackPars trackpars = m_binning->parSetToTrackPars(parset);
      for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
        minvals[par] = std::min(minvals[par],trackpars[par]);
        maxvals[par] = std::max(maxvals[par],trackpars[par]);
      }
    }

    // make sure bin overlaps with active region
    bool inRange = true;
    for (unsigned par =0; par < FPGATrackSimTrackPars::NPARS; par++) {
      inRange = inRange && (minvals[par] < max_padded[par]) && (maxvals[par] > min_padded[par]);
    }
    if (inRange)
    {
      bin.data() = true;
      m_validSlice[m_binning->sliceIdx(bin.idx())] = true;
      m_validScan[m_binning->scanIdx(bin.idx())] = true;
      m_validSliceAndScan[m_binning->sliceAndScanIdx(bin.idx())] = true;
    }

    if (bin.data() == false)
    {
      ATH_MSG_VERBOSE("Invalid bin: " << bin.idx() << " :" << m_binning->parSetToTrackPars(m_binning->binCenter(bin.idx()))
      << " minvals: " << minvals << " maxvals: " << maxvals );
    }
  }

  // count valid bins
  int validBins = 0;
  for (FPGATrackSimGenScanArray<int>::Iterator bin : m_validBin) { if(bin.data()) validBins++;}

  int validSlices = 0;
  for (FPGATrackSimGenScanArray<int>::Iterator bin : m_validSlice) { if(bin.data()) validSlices++;}

  int validScans = 0;
  for (FPGATrackSimGenScanArray<int>::Iterator bin : m_validScan) { if(bin.data()) validScans++;}

  ATH_MSG_INFO("Valid Bins: " << validBins
                              << " valid slices: " << validSlices
                              << " valid scans: " << validScans);
}



