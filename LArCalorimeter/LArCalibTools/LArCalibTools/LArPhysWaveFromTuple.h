/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARPHYSWAVEFROMTuple_H
#define LARPHYSWAVEFROMTuple_H

#include "AthenaBaseComps/AthAlgorithm.h"

#include <vector>
#include <string>

/** @class LArPhysWaveFromTuple

This algorithm allows to read wave forms from ntuples and builds a 
LArPhysWaveContainer conating the corresponding PhysWave. The root tree should
 contain the following branches :
- channelId online ID
- timeIndex : number of samples
- Time[timeIndex] : double array of times
- Amplitude[timeIndex] : double array of amplitudes
 */


class LArPhysWaveFromTuple : public AthAlgorithm
{
 public:
  LArPhysWaveFromTuple(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArPhysWaveFromTuple();

  //standard algorithm methods
  StatusCode initialize() ; 

  StatusCode execute() {return StatusCode::SUCCESS;}

  StatusCode finalize(){return StatusCode::SUCCESS;}
  StatusCode stop();
 
 private:
  /// max number of points of the waveform in the ntuple
  Gaudi::Property<unsigned int> m_NPoints{this, "NPoints", 800, "Max number of points in ntuple"};
  /// the first  m_skipPoints points of the waveform in the ntuple are skipped
  Gaudi::Property<unsigned int> m_skipPoints{this, "SkipPoints", 0, "How many points to skip"};
  /// make a PhysWave with the first m_prefixPoints as zeros
  Gaudi::Property<unsigned int> m_prefixPoints{this, "PrefixPoints", 0, "How many points to add on front"};
  /// flag for the PhysWave container
  Gaudi::Property<unsigned int> m_flag{this, "LArWaveFlag", 20, "Flag to store with PhysWave"};
  /// input file name 
  Gaudi::Property<std::string> m_root_file_name{this, "FileName", "", "which file to open"};
  /// ntuple name
  Gaudi::Property<std::string> m_ntuple_name{this, "NtupleName", "PHYSWAVE", "which ntuple to read"};
  /// key of the PhysWave collection in Storegate
  Gaudi::Property<std::string> m_store_key{this, "StoreKey", "FROMTUPLE", "SG key of created container"};
  /// Grouping type.  Default is Feedthrough.
  Gaudi::Property<std::string> m_groupingType{this, "GroupingType", "ExtendedFeedThrough", "container grouping type"};
  Gaudi::Property< bool > m_isSC{this, "isSC", false, "Running for SuperCells ?"};
  Gaudi::Property< int > m_gain{this, "Gain", 0, "which gain to use ?"};
};

#endif
