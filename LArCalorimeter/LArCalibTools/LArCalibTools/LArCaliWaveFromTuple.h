/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIWAVEFROMTuple_H
#define LARCALIWAVEFROMTuple_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include <string>

/** @class LArCaliWaveFromTuple

This algorithm allows to read wave forms from ntuples and builds a 
LArCaliWaveContainer containg the corresponding CaliWave. The root tree should
 contain the following branches :
 */


class LArCaliWaveFromTuple : public AthAlgorithm
{
 public:
  LArCaliWaveFromTuple(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArCaliWaveFromTuple();

  //standard algorithm methods
  StatusCode initialize() ; 

  StatusCode execute(const EventContext& ) override final {return StatusCode::SUCCESS;};

  StatusCode stop();
 
 private:
  /// max number of points of the waveform in the ntuple
  Gaudi::Property<unsigned int> m_NPoints{this, "NPoints", 32, "number of points per measurement"};
  /// the first  m_skipPoints points of the waveform in the ntuple are skipped
  Gaudi::Property<unsigned int> m_skipPoints{this, "SkipPoints", 0, "How many points to skip"};
  /// make a PhysWave with the first m_prefixPoints as zeros
  Gaudi::Property<unsigned int> m_prefixPoints{this, "PrefixPoints", 0, "How many points to add on front"};
  /// flag for the PhysWave container
  Gaudi::Property<unsigned int> m_flag{this, "LArWaveFlag", 20, "Flag to store with CaliWave"};
  /// input file name 
  Gaudi::Property<std::string> m_root_file_name{this, "FileName", "", "which file to open"};
  /// ntuple name
  Gaudi::Property<std::string> m_ntuple_name{this, "NtupleName", "CALIWAVE", "which ntuple to read"};
  /// key of the PhysWave collection in Storegate
  Gaudi::Property<std::string> m_store_key{this, "StoreKey", "FROMTUPLE", "SG key of created container"};
  /// Grouping type.  Default is Feedthrough.
  Gaudi::Property<std::string> m_groupingType{this, "GroupingType", "ExtendedFeedThrough", "container grouping type"};
  Gaudi::Property< bool > m_isSC{this, "isSC", false, "Running for SuperCells ?"};
  Gaudi::Property< float > m_dt{this, "DelayStep", 25./24., "what is the delay step in ns ?"};
};

#endif
