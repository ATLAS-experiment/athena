/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARPHYSWAVEFROMASCII_H
#define LARPHYSWAVEFROMASCII_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

/** @class LArPhysWaveFromAscii

This algorithm allows to read wave forms from asci file and builds a 
LArPhysWaveContainer containing the corresponding PhysWave. 
 */


class LArPhysWaveFromAscii : public AthAlgorithm
{
 public:
  LArPhysWaveFromAscii(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArPhysWaveFromAscii();

  //standard algorithm methods
  ///StatusCode initialize() = default;
  virtual StatusCode execute() {return StatusCode::SUCCESS;}
  //StatusCode finalize(){return StatusCode::SUCCESS;}
  virtual StatusCode stop() final;
 
 private:
  /// the first  m_skipPoints points of the waveform in the file are skipped
  UnsignedIntegerProperty m_skipPoints{this, "SkipPoints", 0};
  /// make a PhysWave with the first m_prefixPoints as zeros
  UnsignedIntegerProperty m_prefixPoints{this, "PrefixPoints", 0};
  /// list of input ntuple file names 
  StringProperty m_input_file_name{this, "InputFile", {}, "Input ascii file name" };
  /// Grouping type.  Default is Feedthrough.
  StringProperty m_groupingType{this,"GroupingType", "ExtendedFeedThrough", "Which COOL channel grouping to use"};
  /// which gain to store ?
  UnsignedIntegerProperty m_gain{this, "Gain", 0};
  /// is SC ?
  BooleanProperty m_isSC{this,"isSC",false,"Running for SuperCells ?"};
  /// needs interpolation ?
  BooleanProperty m_interpolate{this,"Interpolate",false,"Needs interpolate input data ?"};
  /// has index in input data ?
  BooleanProperty m_hasIndex{this,"Index",false,"Has index in  input data ?"};
  
  /// key of the PhysWave collection 
  StringProperty m_store_key{this, "StoreKey", "FromAscii", "SG key to create"};
};

#endif
