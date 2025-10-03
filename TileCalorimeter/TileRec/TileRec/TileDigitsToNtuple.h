/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileDigitsToNtuple.h
// Author   : 
// Created  : 
//
// DESCRIPTION
// 
//    To create Digits Ntuple file from TileHitContainer
//
// Properties (JobOption Parameters):
//
//    TileDigitsContainer         string   key value of Digits in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    string   ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TILEDIGITSTONTUPLE_H
#define TILEDIGITSTONTUPLE_H

#include "TileEvent/TileDigitsContainer.h"

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

class TileID;
class TileHWID;
class TileTBID;

#include <string>

class TileDigitsToNtuple : public AthAlgorithm {
 public:

  using AthAlgorithm::AthAlgorithm;
  virtual ~TileDigitsToNtuple() = default;

  //Gaudi Hooks
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;
  
 private:

  Gaudi::Property<bool> m_saveAll{this,
     "SaveAll", true, "Save all Tile digits"};

  Gaudi::Property<bool> m_saveE4prAndMBTS{this,
     "SaveE4prAndMBTS", true, "Save Tile digits for E4 prime and MBTS"};

  Gaudi::Property<int> m_saveMaxChannels{this,
     "SaveMaxChannels", 12288, "Maximum Tile raw channels to save"};

  Gaudi::Property<int> m_commitNtuple{this,
     "CommitNtuple", true, "Commit ntuple"};

  Gaudi::Property<std::string> m_infoName{this,
     "TileInfo", "TileInfo", "Tile info name"};

  Gaudi::Property<std::string> m_ntupleLoc{this,
     "NTupleLoc", "/TILE/TileRec", "Tile digits ntuple location"};

  Gaudi::Property<std::string> m_ntupleID{this,
     "NTupleID", "h40", "Tile digits ntuple ID"};

  SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
     "TileDigitsContainer", "TileDigitsCnt", "Tile digits container name."};

  const TileID* m_tileID{nullptr};
  const TileHWID* m_tileHWID{nullptr};
  const TileTBID* m_tileTBID{nullptr};

  NTuple::Tuple* m_ntuplePtr{nullptr};

  NTuple::Item<short> m_nChannel;

  NTuple::Array<short> m_ros;
  NTuple::Array<short> m_drawer;
  NTuple::Array<short> m_channel;
  NTuple::Array<short> m_gain;

  NTuple::Array<short> m_section;
  NTuple::Array<short> m_side;
  NTuple::Array<short> m_phi;
  NTuple::Array<short> m_eta;
  NTuple::Array<short> m_sample;
  NTuple::Array<short> m_pmt;
  NTuple::Array<short> m_adc;

  NTuple::Matrix<short> m_samples;
  
  std::string m_digitsContainer;
  int m_nSamples{7};
};

#endif
