///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Hist.h 
// Header file for class AthEx::Hist
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHEXHISTNTUP_ATHEXHIST_H
#define ATHEXHISTNTUP_ATHEXHIST_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

// fwd declares
class TH1F;

namespace AthEx {

class Hist
  : public ::AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 

  // Copy constructor: 

  /// Constructor with parameters: 
  Hist( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~Hist(); 

  // Assignment operator: 
  //Hist &operator=(const Hist &alg); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize();
  virtual StatusCode  execute();
  virtual StatusCode  finalize();

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Default constructor: 
  Hist();

  /// handle to the histogram service
  ServiceHandle<ITHistSvc> m_histSvc {this,"THistSvc", "THistSvc", "Handle to the histogram service"};

  /// pointer to our histogram
  TH1F *m_hist;

  /// key to the event-info
  SG::ReadHandleKey<xAOD::EventInfo> m_evt {this,"EventInfo", "EventInfo", "xAOD EventInfo name"};
}; 

} //> namespace AthEx

#endif //> !ATHEXHISTNTUP_ATHEXHIST_H
