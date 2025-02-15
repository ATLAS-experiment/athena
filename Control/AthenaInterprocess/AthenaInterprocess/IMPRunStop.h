/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAINTERPROCESS_IMPRUNSTOP_H
#define ATHENAINTERPROCESS_IMPRUNSTOP_H
#include <GaudiKernel/IInterface.h>
namespace AthenaInterprocess {
  class IMPRunStop : virtual public IInterface
  {
  public:
    /// InterfaceID
    DeclareInterfaceID( IMPRunStop, 1, 0 );

    virtual bool stopScheduled() const = 0;
  };
}  

#endif
