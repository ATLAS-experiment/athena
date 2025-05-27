///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// RootAsciiDumperAlgHandle.h 
// Header file for class RootAsciiDumperAlgHandle
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHENAROOTCOMPS_ATHENA_ROOTASCIIDUMPERALGHANDLE_H
#define ATHENAROOTCOMPS_ATHENA_ROOTASCIIDUMPERALGHANDLE_H 1

// STL includes
#include <string>
#include <vector>
#include <stdint.h>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

namespace Athena {

class RootAsciiDumperAlgHandle
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  /// Inherited constructor.
  using ::AthAlgorithm::AthAlgorithm;

  /// Destructor: 
  virtual ~RootAsciiDumperAlgHandle() = default;

  // Assignment operator: 
  //RootAsciiDumperAlgHandle &operator=(const RootAsciiDumperAlgHandle &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  RootAsciiDumperAlgHandle();

  /// ASCII output file name
  StringProperty m_ofname
    { this, "AsciiFileName", "d3pd.ascii",
      "Name of the ascii file where the content of the "
      "ROOT n-tuple file will be dumped." };

  /// file handle to the ASCII output file
  int m_ofd = -1;
  
  /// number of entries processed so-far
  uint64_t m_nentries = 0;

  /// run number
  SG::ReadHandleKey<uint32_t> m_runnbr
    { this, "RunNumber", "RunNumber", "handle to the run-nbr in event (read)" };

  /// event number
  SG::ReadHandleKey<uint32_t> m_evtnbr
    { this, "EventNumber", "EventNumber", "handle to the evt-nbr in event (read)" };

  /// number of electrons
  SG::ReadHandleKey<int32_t> m_el_n
    { this, "el_n", "el_n", "handle to the nbr of electrons in event (read)" };

  /// eta of electrons
  SG::ReadHandleKey<std::vector<float> > m_el_eta
    { this, "el_eta", "el_eta", "handle to the eta of electrons in event (read)" };

  /// jetcone dR
  SG::ReadHandleKey<std::vector<std::vector<float> > > m_el_jetcone_dr
    { this, "el_jetcone_dr", "el_jetcone_dr", "handle to the jetcone-dR of electrons in event (read)" };

  SG::ReadHandleKey<xAOD::EventInfo> m_eiKey
    { this, "eiKey", "EventInfo", "" };
}; 

// I/O operators
//////////////////////

/////////////////////////////////////////////////////////////////// 
// Inline methods: 
/////////////////////////////////////////////////////////////////// 

} //> end namespace Athena
#endif //> !ATHENAROOTCOMPS_ATHENA_ROOTASCIIDUMPERALGHANDLE_H
