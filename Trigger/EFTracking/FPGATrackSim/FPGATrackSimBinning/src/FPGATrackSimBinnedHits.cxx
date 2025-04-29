// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinDesc.cxx
 * @author Elliot Lipeles
 * @date Feb 13, 2025
 * @brief See header file.
 */

#include "FPGATrackSimBinning/FPGATrackSimBinnedHits.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "FPGATrackSimBinning/FPGATrackSimBinStep.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include <GaudiKernel/StatusCode.h>
#include <nlohmann/json.hpp>


StatusCode FPGATrackSimBinnedHits::initialize() {
  // Dump the configuration to make sure it propagated through right
  const std::vector<Gaudi::Details::PropertyBase*> props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }


  ATH_MSG_DEBUG("Retrieving BinTool");
  ATH_CHECK(m_bintool.retrieve());
  ATH_MSG_DEBUG("Retrieving EvtSel");
  ATH_CHECK(m_EvtSel.retrieve());
  // Compute which bins correspond to track parameters that are in the region
  // i.e. the pT, eta, phi, z0 and d0 bounds
  // list of valid bins is extracted from the layer map if its loaded
  m_bintool->initValidBins();
  if (m_lyrmapFile.size()==0)
  {
    m_bintool->computeValidBins(m_EvtSel.get());
  } else {
    readLayerMap(m_lyrmapFile);
  }
  m_bintool->printValidBin(); // also dumps firmware constants
  initBinnedDataArrays();

  return StatusCode::SUCCESS;
}

void FPGATrackSimBinnedHits::initBinnedDataArrays() {
  m_binnedHitsStep.resize(m_bintool->steps().size());
  int i = 0;
  for (auto &step : m_bintool->steps()) {
    m_binnedHitsStep[i].setsize(step->nBins(), BinEntry());
    ATH_MSG_INFO("Step" << step->stepName() << " Image Size (full): "
                        << m_binnedHitsStep[i].size());
    ++i;
  }
  resetBins(); // just to be sure
}

void FPGATrackSimBinnedHits::resetBins() {
  for (auto &stepdata : m_binnedHitsStep) {
    for (FPGATrackSimBinArray<BinEntry>::Iterator bin : stepdata) {
      bin.data().reset();
    }
  }
}

// Put hits in all track parameter bins they could be a part of (binning is defined
// by m_binning object)
StatusCode FPGATrackSimBinnedHits::fill(
    const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits) {
  ATH_MSG_DEBUG("In fill");

  int stepnum = 0;
  for (const auto &step : m_bintool->steps()) {

    ATH_MSG_DEBUG("fill binning: step num " << stepnum << " " << step->stepName());
    ATH_MSG_DEBUG("Number of valid bins (full) = " << step->validBinsFull().size());
    for (auto &bin : step->validBinsFull()) {

      // skip bin if it is invalid
      if (!bin.data())
        continue;

      ATH_MSG_VERBOSE("valid bin");
      if (stepnum == 0) {

        // first step, hits from input stream
        for (const std::shared_ptr<const FPGATrackSimHit> &hit : hits) {
          StoredHit storedhit(hit);
          if (m_bintool->binDesc()->hitInBin(*step.get(), bin.idx(),
                                             storedhit)) {
            m_binnedHitsStep[stepnum][bin.idx()].addHit(storedhit);
          }
        }

      } else {        
        // subsequent steps, use hits from previous step
        ATH_MSG_VERBOSE("Looping over previous hits of size = " << m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits.size());
        for (const auto &hit :
             m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits) {
          StoredHit storedhit(hit);
          if (m_bintool->binDesc()->hitInBin(*step.get(), bin.idx(),
                                             storedhit)) {
            ATH_MSG_VERBOSE("Hit found to be in bin, is it mapped? " << hit.hitptr->isMapped());
            // One last step, set layer based on layerMap or use default from pmap
            if (step.get() == m_bintool->lastStep()) {
              if (m_mod_to_lyr_map.size() != 0) {
                if (m_mod_to_lyr_map[bin.idx()].contains(hit.hitptr->getIdentifierHash())) {
                  storedhit.layer = m_mod_to_lyr_map[bin.idx()][hit.hitptr->getIdentifierHash()];
                  m_binnedHitsStep[stepnum][bin.idx()].addHit(storedhit);
                } 
              } else {
                // TODO is it right to ignore hits that were unmapped?
                if (hit.hitptr->isMapped()) {
                  storedhit.layer = hit.hitptr->getLayer();
                  m_binnedHitsStep[stepnum][bin.idx()].addHit(storedhit);
                }
              }
            }            
          }
        }
      }

    } //  end loop over bins
      
    stepnum++;
  } // end loop over stepsd

  return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Internal Storage Class

void FPGATrackSimBinnedHits::BinEntry::reset()
{
  hitCnt = 0;
  lyrhit = 0;
  hits.clear();
}

void FPGATrackSimBinnedHits::BinEntry::addHit(const StoredHit& hit)
{
  hitCnt++;
  if (((lyrhit >> hit.layer) & 0x1) == 0x0)
  {
    lyrhit |= (0x1 << hit.layer);
  }
  hits.push_back(hit);
}

unsigned FPGATrackSimBinnedHits::BinEntry::hitsInLyr(unsigned lyr) const {
  return std::count_if(hits.begin(),hits.end(),[lyr](auto& hit){return hit.layer==lyr;});
}


//-------------------------------------------------------------------------------
//
// Layer map implementation
//
//-------------------------------------------------------------------------------

void FPGATrackSimBinnedHits::readLayerMap(const std::string& filename) {
  std::ifstream f(filename);
  nlohmann::json data = nlohmann::json::parse(f);

  m_lyr_to_mod_map.setsize(m_bintool->lastStep()->nBins(),
                   std::vector <std::set<unsigned> >(8,std::set<unsigned>()));
  m_mod_to_lyr_map.setsize(m_bintool->lastStep()->nBins(),
                   std::map <unsigned,unsigned>());

  // This sets the number of layer, to the number of layers in the layerMap
  m_nLayers = 0;
  for (const auto &binelem : data) {
    std::vector<unsigned> bin;
    binelem.at("bin").get_to(bin);
    auto& lyrmap = binelem["lyrmap"];
    ATH_MSG_DEBUG("bin = " << bin);
    ATH_MSG_DEBUG("lyrmap = " << lyrmap);
    for (auto &lyrelem : lyrmap) {
      unsigned lyr;
      lyrelem.at("lyr").get_to(lyr);
      lyrelem.at("mods").get_to(m_lyr_to_mod_map[bin][lyr]);
      ATH_MSG_DEBUG("lyr = " << lyr);
      ATH_MSG_DEBUG("mods = " << m_lyr_to_mod_map[bin][lyr]);
      for (auto &mod : m_lyr_to_mod_map[bin][lyr]) {
        m_mod_to_lyr_map[bin][mod]=lyr;
      }
      // set valid bins, this expects binning based on the last step
      m_bintool->setValidBin(bin);
    }
    if (m_nLayers == 0) {
      m_nLayers = m_lyr_to_mod_map[bin].size();
    } else if (m_nLayers != m_lyr_to_mod_map[bin].size())  {
      ATH_MSG_FATAL("Layer map bins have inconsistent numbers of layers");
    }
  }

}
