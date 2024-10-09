/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PERMONKERNEL_IPERFMONMTSV_H
#define PERMONKERNEL_IPERFMONMTSV_H

/// STL includes
#include <string>

/// Framework include
#include "GaudiKernel/IService.h"

class IPerfMonMTSvc : virtual public IService
{

  public:
    /// Framework - Service InterfaceID
    DeclareInterfaceID(IPerfMonMTSvc, 1, 0);

    /// Start Auditing
    virtual void startAud( const std::string& stepName,
                           const std::string& compName = "PerfMonMTSlice" ) = 0;

    /// Stop Auditing
    virtual void stopAud( const std::string& stepName,
                          const std::string& compName = "PerfMonMTSlice" ) = 0;


}; // class IPerfMonMTSvc

#endif // PERMONKERNEL_IPERFMONMTSV_H
