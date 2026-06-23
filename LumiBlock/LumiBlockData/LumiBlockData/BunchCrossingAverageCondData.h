// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file LumiBlockData/BunchCrossingAverageCondData.h
 * @author Lorenzo Rossini <lorenzo.rossini@cern.ch>
 * @date July 2024
 * @brief Adding more information about Bunch Current Intensities (for Lumi studies)
 */


#ifndef LUMIBLOCKDATA_BUNCHCROSSINGAVERAGECONDDATA_H
#define LUMIBLOCKDATA_BUNCHCROSSINGAVERAGECONDDATA_H


#include "AthenaKernel/CondCont.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <vector>
#include <bitset>


class BunchCrossingAverageCondData {

public:

  typedef unsigned int bcid_type;
  static constexpr int m_MAX_BCID=3564;
  static constexpr int m_BUNCH_SPACING = 25;



  // Retrieve the Beam1/2 total intensity. Only paired bunches 
  float GetBeam1Intensity(int channel ) const; 
  float GetBeam2Intensity(int channel ) const;

  // Retrieve the Beam1/2 total intensity. Paired and unpaired bunches 
  float GetBeam1IntensityAll(int channel ) const;
  float GetBeam2IntensityAll(int channel ) const;

  // Retrieve the Beam1/2 total intensity error. Only paired  bunches 
  float GetBeam1IntensitySTD(int channel ) const;
  float GetBeam2IntensitySTD(int channel ) const;

  // Retrieve the Beam1/2 total intensity error. Paired and unpaired bunches 
  float GetBeam1IntensityAllSTD(int channel ) const;
  float GetBeam2IntensityAllSTD(int channel ) const;

  unsigned long long GetRunLB( ) const;
  // Set the Beam1/2 total intensity. Only paired bunches 
  void SetBeam1Intensity( float  Beam1Intensity,int channel) ;
  void SetBeam2Intensity( float  Beam2Intensity,int channel) ;
  
  // Set the Beam1/2 total intensity. Paired and unpaired bunches 
  void SetBeam1IntensityAll( float  Beam1IntensityAll,int channel) ;
  void SetBeam2IntensityAll( float  Beam2IntensityAll,int channel) ;
  
  // Set the Beam1/2 total intensity error. Only paired  bunches 
  void SetBeam1IntensitySTD( float  Beam1IntensitySTD,int channel) ;
  void SetBeam2IntensitySTD( float  Beam2IntensitySTD,int channel) ;
  
  // Set the Beam1/2 total intensity error. Paired and unpaired bunches 
  void SetBeam1IntensityAllSTD( float  Beam1IntensityAllSTD,int channel) ;
  void SetBeam2IntensityAllSTD( float  Beam2IntensityAllSTD,int channel) ;
  


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


  enum BunchDistanceType {
    NanoSec, ///< Distance in nanoseconds
    BunchCrossings, ///< Distance in units of 25 nanoseconds
    /// Distance in units of filled bunches (depends on filling scheme)
    FilledBunches
  };

  

private:

  friend class BunchCrossingAverageCondAlg;// The cond-alg filling this class


  // Data


  float m_beam1IntensityAll = 0;
  float m_beam2IntensityAll = 0;
  float m_beam1IntensityAll_fBCT = 0;
  float m_beam2IntensityAll_fBCT = 0;
  float m_beam1IntensityAll_DCCT = 0;
  float m_beam2IntensityAll_DCCT = 0;
  float m_beam1IntensityAll_DCCT24 = 0;
  float m_beam2IntensityAll_DCCT24 = 0;

  float m_beam1Intensity = 0;
  float m_beam2Intensity = 0;
  float m_beam1Intensity_fBCT = 0;
  float m_beam2Intensity_fBCT = 0;
  float m_beam1Intensity_DCCT = 0;
  float m_beam2Intensity_DCCT = 0;
  float m_beam1Intensity_DCCT24 = 0;
  float m_beam2Intensity_DCCT24 = 0;


  float m_beam1IntensityAllSTD = 0;
  float m_beam2IntensityAllSTD = 0;
  float m_beam1IntensityAllSTD_fBCT = 0;
  float m_beam2IntensityAllSTD_fBCT = 0;
  float m_beam1IntensityAllSTD_DCCT = 0;
  float m_beam2IntensityAllSTD_DCCT = 0;
  float m_beam1IntensityAllSTD_DCCT24 = 0;
  float m_beam2IntensityAllSTD_DCCT24 = 0;

  float m_beam1IntensitySTD = 0;
  float m_beam2IntensitySTD = 0;
  float m_beam1IntensitySTD_fBCT = 0;
  float m_beam2IntensitySTD_fBCT = 0;
  float m_beam1IntensitySTD_DCCT = 0;
  float m_beam2IntensitySTD_DCCT = 0;
  float m_beam1IntensitySTD_DCCT24 = 0;
  float m_beam2IntensitySTD_DCCT24 = 0;
  unsigned long long m_RunLB = 0;
  const static int m_headTailLength = 300; // magic number 300 ns from Run 2 tool



};  
CLASS_DEF( BunchCrossingAverageCondData , 120132775 , 1 )
CONDCONT_MIXED_DEF (BunchCrossingAverageCondData, 102554749 );


#endif // not COOLLUMIUTILITIES_FILLPARAMSCONDDATA_H
