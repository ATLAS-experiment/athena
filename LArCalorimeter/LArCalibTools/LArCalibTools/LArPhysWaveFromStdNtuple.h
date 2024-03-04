/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARPHYSWAVEFROMSTDNTUPLE_H
#define LARPHYSWAVEFROMSTDNTUPLE_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include <vector>
#include <string>

/** @class LArPhysWaveFromStdNtuple

This algorithm allows to read wave forms from ntuples and builds a 
LArPhysWaveContainer containing the corresponding PhysWave. 
Version for standard Ntuple, produced by LArCalibTools algos....
 */


class LArPhysWaveFromStdNtuple : public AthAlgorithm
{
 public:
  LArPhysWaveFromStdNtuple(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArPhysWaveFromStdNtuple();

  //standard algorithm methods
  StatusCode initialize() {return StatusCode::SUCCESS;}
  StatusCode execute() {return StatusCode::SUCCESS;}
  StatusCode finalize(){return StatusCode::SUCCESS;}
  StatusCode stop();
 
 private:
  /// the first  m_skipPoints points of the waveform in the ntuple are skipped
  Gaudi::Property< unsigned int > m_skipPoints{this, "SkipPoints", 0};
  /// make a PhysWave with the first m_prefixPoints as zeros
  Gaudi::Property< unsigned int > m_prefixPoints{this, "PrefixPoints", 0};
  /// list of input ntuple file names 
  Gaudi::Property< std::vector<std::string> > m_root_file_names{this, "FileNames", {}, "Input root file names" };
  /// ntuple name
  Gaudi::Property< std::string > m_ntuple_name{this, "NtupleName", "PhysWave"};
  /// key of the PhysWave collection in Storegate
  Gaudi::Property< std::string > m_store_key{this, "StoreKey", "FromStdNtuple", "SG key to create"};
  /// Grouping type.  Default is Feedthrough.
  Gaudi::Property< std::string > m_groupingType{this,"GroupingType", "ExtendedFeedThrough", "Which COOL channel grouping to use"};
  /// is SC ?
  Gaudi::Property< bool > m_isSC{this,"isSC",false,"Running for SuperCells ?"};
};

#endif
