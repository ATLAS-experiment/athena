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
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include <GaudiKernel/StatusCode.h>
#include <nlohmann/json.hpp>

using FPGATrackSimBinUtil::StoredHit;

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

  if ((m_lyrmapFile.size()!=0)&&((m_lyrradiiFile.size()!=0))) {
    ATH_MSG_FATAL("Both lyrmap and layradii, these are mutually exclusive");
  }

  if (m_lyrmapFile.size()!=0) {
    readLayerMap(m_lyrmapFile);
  } else if (m_lyrradiiFile.size()!=0) {
    readLayerRadii(m_lyrradiiFile);
  } else {
    ATH_MSG_INFO("Analytically computing valid bins");
    m_bintool->computeValidBins(m_EvtSel.get());
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
    ATH_MSG_INFO("Step" << step->stepName() << " q Size (full): "
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
        if (m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits.size()!=0)
          ATH_MSG_VERBOSE("Looping over previous hits of size = " << m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits.size());

        for (const auto &hit :
             m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits) {
          StoredHit storedhit(hit);
          if (m_bintool->binDesc()->hitInBin(*step.get(), bin.idx(),storedhit)) {
            bool writeHit = true;

            // if it is the laststep, set the layer and only write hit if there is a valid layer                                 
            if (step.get() == m_bintool->lastStep()) {
              if (m_mod_to_lyr_map.size() != 0) { // there is a layermap                
                if (m_mod_to_lyr_map[bin.idx()].contains(hit.hitptr->getIdentifierHash())) {
                  storedhit.layer = m_mod_to_lyr_map[bin.idx()][hit.hitptr->getIdentifierHash()];
                } else {
                  ATH_MSG_VERBOSE("Hit not in layermap" << hit);
                  writeHit=false;
                }
              } else if (m_lyr_radii.size()!=0) { // there is a layer radius set
                 auto lyr_radii = m_lyr_radii[bin.idx()];
                 for (unsigned lyr = 0; lyr < lyr_radii.size(); lyr++) {
                    writeHit=false;
                    if ((hit.rzrad() > lyr_radii[lyr].first)&&(hit.rzrad() < lyr_radii[lyr].second)) {
                      storedhit.layer = lyr;
                      writeHit=true;
                      break;
                    } 
                  } 
                  if(msgLvl(MSG::VERBOSE) && !writeHit) {
                    std::vector<double> rads;
                    for (unsigned lyr = 0; lyr < lyr_radii.size(); lyr++) {             
                      rads.push_back(lyr_radii[lyr].first);
                      rads.push_back(lyr_radii[lyr].second);
                    }                      
                    ATH_MSG_VERBOSE("Hit not in layer radii " << bin.idx() << " " << hit.rzrad() << " "  << rads);
                  }
              } else { // no layer map
                // TODO is it right to ignore hits that were unmapped?
                if (hit.hitptr->isMapped()) {
                  storedhit.layer = hit.hitptr->getLayer();
                } else {
                  ATH_MSG_VERBOSE("Hit not mapped" << hit);
                  writeHit=false;
                }
              }
            }

            // add hit to bin
            if (writeHit) m_binnedHitsStep[stepnum][bin.idx()].addHit(storedhit);
          }
        }
        
        if (m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits.size()!=0)
        ATH_MSG_DEBUG("Bin Hit Count: step " << step.name()
          << " binidx = " << bin.idx()
          << " input hits = "  << m_binnedHitsStep[stepnum - 1][step->convertToPrev(bin.idx())].hits.size()
          << " layers=" << m_binnedHitsStep[stepnum][bin.idx()].lyrCnt() 
          << " hits="  << m_binnedHitsStep[stepnum][bin.idx()].hitCnt );
      }

    } //  end loop over bins
      
    stepnum++;
  } // end loop over steps

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
  hits.push_back(hit);
  if (hit.layer != StoredHit::invalidLayer)
  {
    lyrhit |= (0x1 << hit.layer);
  }
}

unsigned FPGATrackSimBinnedHits::BinEntry::hitsInLyr(unsigned lyr) const {
  return std::count_if(hits.begin(),hits.end(),[lyr](auto& hit){return hit.layer==lyr;});
}


//-------------------------------------------------------------------------------
//
// Layer map implementation
//
//-------------------------------------------------------------------------------

void FPGATrackSimBinnedHits::readLayerMap(const std::string &filename) {

  ATH_MSG_INFO("Reading lyrmap json: " << filename);
  
  std::ifstream f(filename);
  nlohmann::json data = nlohmann::json::parse(f);

  m_lyr_to_mod_map.setsize(m_bintool->lastStep()->nBins(),
                   std::vector <std::set<unsigned> >());
  m_mod_to_lyr_map.setsize(m_bintool->lastStep()->nBins(),
                   std::map <unsigned,unsigned>());

  // This sets the number of layer, to the number of layers in the layerMap
  m_nLayers = 0;
  for (const auto &binelem : data) {
    std::vector<unsigned> bin;
    binelem.at("bin").get_to(bin);
    auto& lyrmap = binelem["lyrmap"];
    ATH_MSG_VERBOSE("bin = " << bin);
    if (!m_lyr_to_mod_map.isValid(bin)) {
      ATH_MSG_FATAL("lyrmap includeds invalid bin:" << bin);
    }
    ATH_MSG_VERBOSE("lyrmap = " << lyrmap);
    for (auto &lyrelem : lyrmap) {
      unsigned lyr;
      lyrelem.at("lyr").get_to(lyr);
      m_lyr_to_mod_map[bin].push_back(std::set<unsigned>());
      lyrelem.at("mods").get_to(m_lyr_to_mod_map[bin][lyr]);
      ATH_MSG_VERBOSE("lyr = " << lyr);
      ATH_MSG_VERBOSE("mods = " << m_lyr_to_mod_map[bin][lyr]);
      for (auto &mod : m_lyr_to_mod_map[bin][lyr]) {
        m_mod_to_lyr_map[bin][mod]=lyr;
      }
      // set valid bins, this expects binning based on the last step
      m_bintool->setValidBin(bin);
    }
    for (auto &lyrmods : m_lyr_to_mod_map[bin]) {
      ATH_MSG_VERBOSE(" mods: "  << lyrmods);
    } 
    if (m_nLayers != 0 && m_nLayers != m_lyr_to_mod_map[bin].size())  {
      ATH_MSG_WARNING("Layer map bins have inconsistent numbers of layers: " << m_nLayers << ", " << m_lyr_to_mod_map[bin].size());
    }
    m_nLayers = std::max(m_nLayers, static_cast<unsigned>(m_lyr_to_mod_map[bin].size()));
  }
  ATH_MSG_INFO("JSON layer map speficied " << m_nLayers << " layers" );
}


void FPGATrackSimBinnedHits::readLayerRadii(const std::string &filename) {

  ATH_MSG_INFO("Reading lyrmap json: " << filename);
  
  std::ifstream f(filename);
  nlohmann::json data = nlohmann::json::parse(f);

  m_lyr_radii.setsize(m_bintool->lastStep()->nBins(),
  std::vector < std::pair<double,double> > ());

  // This sets the number of layer, to the number of layers in the layerMap
  m_nLayers = 0;
  for (const auto &binelem : data) {
    std::vector<unsigned> bin;
    binelem.at("bin").get_to(bin);
    ATH_MSG_VERBOSE("bin = " << bin);
    if (!m_lyr_radii.isValid(bin)) {
      ATH_MSG_FATAL("lyrmap includeds invalid bin:" << bin);
    }

    auto& lyrradii = binelem["lyrradii"];
    ATH_MSG_VERBOSE("lyrradii = " << lyrradii);

    for (auto &lyrelem : lyrradii) {
      unsigned lyr;
      lyrelem.at("lyr").get_to(lyr);
      std::vector<double> temp(2);
      lyrelem.at("radii").get_to(temp);
      m_lyr_radii[bin].push_back(std::pair<double,double>(temp[0],temp[1]));
      ATH_MSG_VERBOSE("lyr = " << lyr);
      ATH_MSG_VERBOSE("radii = " << m_lyr_radii[bin][lyr]);      
      // set valid bins, this expects binning based on the last step
      m_bintool->setValidBin(bin);
    }    
    if (m_nLayers != 0 && m_nLayers != m_lyr_radii[bin].size())  {
      ATH_MSG_WARNING("Layer map bins have inconsistent numbers of layers: " << m_nLayers << ", " << m_lyr_to_mod_map[bin].size());
    }
    m_nLayers = std::max(m_nLayers, static_cast<unsigned>(m_lyr_radii[bin].size()));
  }
  ATH_MSG_INFO("JSON layer map speficied " << m_nLayers << " layers" );
}
