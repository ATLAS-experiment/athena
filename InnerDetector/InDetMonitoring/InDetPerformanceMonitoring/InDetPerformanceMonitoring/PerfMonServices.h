/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef IDPERFMON_SERVICES_H
#define IDPERFMON_SERVICES_H

#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/Algorithm.h"

#include "StoreGate/StoreGateSvc.h"

class PerfMonServices
{
 public:
  // Constructors and Destructors.
  PerfMonServices();
  ~PerfMonServices();

  enum CONTAINERS
  {
    MUON_START = 0,
    MUID_COLLECTION  = MUON_START,
    STACO_COLLECTION,
    MUON_COLLECTION,
    MUON_END,
    
    ELEC_START = MUON_END,
    ELECTRON_COLLECTION = ELEC_START,
    ELEC_END,
    
    PHOT_START = ELEC_END,
    PHOTON_COLLECTION = PHOT_START,
    PHOT_END,

    MET_START = PHOT_END,
    MET_COLLECTION = MET_START,
    MET_END,
    
    TRK_START = MET_END,
    TRK_COLLECTION = TRK_START,
    TRK_END,
   
    VTX_START = TRK_END,
    VTX_COLLECTION = VTX_START,
    VTX_END,
    
    NUM_CONTAINERS = VTX_END
  };

  static const std::string&  getContainerName( CONTAINERS eContainer )         { return s_sContainerNames[eContainer]; }
  static const std::string&  getAtlfastContainerName( CONTAINERS eContainer )  { return s_sAtlfastContainerNames[eContainer]; }

  template <class T>  static const T* getContainer ( CONTAINERS eContainer ) {
    const T* pxContainer = nullptr;
    const std::string& sContainerName = PerfMonServices::getContainerName( eContainer );
    SmartIF<StoreGateSvc> storeGate{Gaudi::svcLocator()->service("StoreGateSvc")};
    if ( storeGate ) {
      storeGate->retrieve( pxContainer , sContainerName ).ignore();
    }
    return pxContainer;
  }

  template <class T> T static const GetMax (T a, T b) {
    T result;
    result = a>b? a : b;
    return result;
  }
  
 protected:

 private:
  // Names of the various object containers.
  static const std::string s_sContainerNames[NUM_CONTAINERS];
  static const std::string s_sAtlfastContainerNames[NUM_CONTAINERS];
};

#endif
