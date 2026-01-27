///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Ntup.h 
// Header file for class AthEx::Ntup
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXHISTNTUP_ATHEXNTUP_H
#define ATHEXHISTNTUP_ATHEXNTUP_H 1

// STL includes
#include <string>
#include <vector>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// fwd declares
class TTree;

namespace AthEx {

class Ntup
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  Ntup( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~Ntup(); 

  // Assignment operator: 
  //Ntup &operator=(const Ntup &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  Ntup();

  /// handle to the histogram service
  ServiceHandle<ITHistSvc> m_ntSvc {this, "THistSvc", "THistSvc", "Handle to the histogram service"};

  // n-tuple data members
  unsigned int m_size;
  unsigned int m_run;
  unsigned int m_event;
  std::vector<long> m_rundata;

  /// pointer to the n-tuple
  TTree* m_ntuple;
  
  /// key to the event-info
  SG::ReadHandleKey<xAOD::EventInfo> m_evt {this, "EventInfo", "EventInfo", 
                                            "key to the event-info instance to ntuple-ize."};
}; 

} //> namespace AthEx

#endif //> !ATHEXHISTNTUP_ATHEXNTUP_H
