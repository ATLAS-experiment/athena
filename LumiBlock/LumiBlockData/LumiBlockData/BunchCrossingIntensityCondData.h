// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file LumiBlockData/BunchCrossingIntensityCondData.h
 * @author Lorenzo Rossini <lorenzo.rossini@cern.ch>
 * @date July 2024
 * @brief Adding more information about Bunch Current Intensities (for Lumi studies)
 */


#ifndef LUMIBLOCKDATA_BUNCHCROSSINGINTENSITYCONDDATA_H
#define LUMIBLOCKDATA_BUNCHCROSSINGINTENSITYCONDDATA_H


#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <vector>
#include <bitset>


class BunchCrossingIntensityCondData {

public:

  typedef unsigned int bcid_type;
  static constexpr int m_MAX_BCID=3564;
  static constexpr int m_BUNCH_SPACING = 25;

  // channel =0 -> BPTX. channel = 1 -> fBCT

  // Retrieve beam intensitity of specific BCID. Only paired BCID. 
  float GetBeam1IntensityBCID(const bcid_type bcid, int channel ) const;
  float GetBeam2IntensityBCID(const bcid_type bcid, int channel ) const;

  // Retrieve the whole per-BCID beam intensitity vector. Only paired BCID
  const std::vector<float>& GetBeam1IntensityPerBCIDVector(int channel) const;  // In  10^10
  const std::vector<float>& GetBeam2IntensityPerBCIDVector(int channel) const;  // In  10^10

  // Retrieve total beam intensitity. Paired and unpaired BCID
  float GetBeam1IntensityAll(int channel ) const;
  float GetBeam2IntensityAll(int channel ) const;
  unsigned long long GetRunLB( ) const;

  // Set total beam intensitity. Paired and unpaired BCID
  void SetBeam1IntensityAll( float  Beam1IntensityAll,int channel) ;
  void SetBeam2IntensityAll( float  Beam2IntensityAll,int channel) ;

  // Set the whole per-BCID beam intensitity vector. Only paired BCID
  void setBeam1IntensityPerBCIDVector (std::vector<float>&& val,int channel);
  void setBeam2IntensityPerBCIDVector (std::vector<float>&& val,int channel);

  void SetRunLB( unsigned long long  RunLB) ;

  /// Enumeration type for a given bunch crossing
  /**
  * This enumeration can specify what kind of bunch crossing one BCID
  * belongs to. The types could easily be extended later on.
  */
  enum BunchCrossingType
  {
    Empty = 0,       ///< An empty bunch far away from filled bunches
    FirstEmpty = 1,  ///< The first empty bunch after a train
    MiddleEmpty = 2, ///< An empty BCID in the middle of a train
    Single = 100,    ///< This is a filled, single bunch (not in a train)
    Front = 200,     ///< The BCID belongs to the first few bunches in a train
    Middle = 201,    ///< The BCID belongs to the middle bunches in a train
    Tail = 202,      ///< The BCID belongs to the last few bunces in a train
    Unpaired = 300   ///< This is an unpaired bunch (either beam1 or beam2)
  };

  /// Enumeration specifying the units in which to expect the bunch distance type
  /**
   * To make it clear for the following functions what units to interpret their
   * return values in, it is possible to request their return values in different
   * units.
   */
  enum BunchDistanceType {
    NanoSec, ///< Distance in nanoseconds
    BunchCrossings, ///< Distance in units of 25 nanoseconds
    /// Distance in units of filled bunches (depends on filling scheme)
    FilledBunches
  };

private:

  friend class BunchCrossingIntensityCondAlg;// The cond-alg filling this class


  // Data

  std::vector<float> m_beam1Intensity = std::vector<float> (m_MAX_BCID);
  std::vector<float> m_beam2Intensity = std::vector<float> (m_MAX_BCID);

  std::vector<float> m_beam1Intensity_fBCT = std::vector<float> (m_MAX_BCID);
  std::vector<float> m_beam2Intensity_fBCT = std::vector<float> (m_MAX_BCID);

  float m_beam1IntensityAll = 0;
  float m_beam2IntensityAll = 0;
  float m_beam1IntensityAll_fBCT = 0;
  float m_beam2IntensityAll_fBCT = 0;

  unsigned long long m_RunLB = 0;
  const static int m_headTailLength = 300; // magic number 300 ns from Run 2 tool

};  
CLASS_DEF( BunchCrossingIntensityCondData , 185321563 , 1 )
CONDCONT_MIXED_DEF (BunchCrossingIntensityCondData, 97829245 );



#endif // not COOLLUMIUTILITIES_FILLPARAMSCONDDATA_H
