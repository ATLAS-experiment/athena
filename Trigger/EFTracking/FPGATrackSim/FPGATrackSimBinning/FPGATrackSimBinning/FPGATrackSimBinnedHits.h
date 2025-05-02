// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimBinnedHits_H
#define FPGATrackSimBinnedHits_H

/**
 * @file FPGATrackSimBinnedHits.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for GenScanTool
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimBinnedHits
 *      
 * 
 * Overview of stucture:
 *    -- 
 * 

 * 
 * References:
 *
 */
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimBinning/FPGATrackSimBinTool.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimBinning/FPGATrackSimBinArray.h"

#include "GaudiKernel/StatusCode.h"

#include <GaudiKernel/ToolHandle.h>

#include <string>
#include <vector>

using FPGATrackSimBinUtil::StoredHit;

//-------------------------------------------------------------------------------------------------------
// BinnedHits
//       Nomenclature:
//          parameter = parameter of track e.g. (pT,eta,phi,d0,z0) but could be
//          in different coordinates
//             such at in the keylayer format where its
//             (z_in,z_out,phi_in,phi_out,x_m)
//          bin = a bin in the full parameters space (upto 5-d)
//          subbin = a binning used as part of the total binning e.g. only in
//          (pT,phi) or only in (z_in,z_out)
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimBinnedHits : public AthAlgTool {
public:
  FPGATrackSimBinnedHits(const std::string &algname, const std::string &name,
                         const IInterface *ifc)
      : AthAlgTool(algname, name, ifc) {}

  virtual StatusCode initialize() override;
  void initBinnedDataArrays();

  StatusCode fill(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits);
  void resetBins();

  // access to tool
  const FPGATrackSimBinTool& getBinTool() const { return *m_bintool.get();}
  FPGATrackSimBinTool& getBinTool() { return *m_bintool.get();}

  ///////////////////////////////////////////////////////////////////////
  // Internal Storage Classes

  // each bin contains a list of StoredHit objects
  struct BinEntry {
    BinEntry() {}
    void reset();
    void addHit(const StoredHit& hit);
    unsigned int lyrCnt() const { return __builtin_popcount(lyrhit); };
    unsigned int hitsInLyr(unsigned lyr) const; 
    unsigned int hitCnt = 0;
    layer_bitmask_t lyrhit = 0;
    std::vector<StoredHit> hits{};
  };

  // access to binned hits..
  const std::vector<FPGATrackSimBinArray<BinEntry>> &binnedHits() const {
    return m_binnedHitsStep;
  }
  const FPGATrackSimBinArray<BinEntry> &lastStepBinnedHits() const {
    return m_binnedHitsStep.back();
  }

  // Layer Map
  // structure is indexed on bin, then layer, then a set of modules
  void readLayerMap(const std::string & filename);
  FPGATrackSimBinArray< std::vector <std::set<unsigned> > > m_lyr_to_mod_map;
  FPGATrackSimBinArray< std::map<unsigned,unsigned> > m_mod_to_lyr_map;

  // Number of layers
  void setNLayers(const unsigned &nLayers) { m_nLayers = nLayers; }
  unsigned getNLayers() const {return m_nLayers;}

private:
  Gaudi::Property<std::string> m_lyrmapFile{this, "layerMapFile",{""}, "use externally defined layer map"};
  ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel{this, "FPGATrackSimEventSelectionSvc", ""};
    
  // Vector of BinEntry for each step
  std::vector<FPGATrackSimBinArray<BinEntry>> m_binnedHitsStep;

  // The tool where the steps are defined
  ToolHandle<FPGATrackSimBinTool> m_bintool {
    this, "BinTool", "FPGATrackSimBinTool",
        "FPGATrackSimBinTool: contains tools describe which parameters are used and each step of binning"};
  
  // The number of layers, either set externally from pmap or set by the layerMap
  unsigned m_nLayers{0}; 
};

#endif // FPGATrackSimBinnedHits_H
