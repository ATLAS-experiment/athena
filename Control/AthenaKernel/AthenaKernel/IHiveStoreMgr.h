///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_IHIVESTOREMGR_H
# define ATHENAKERNEL_IHIVESTOREMGR_H


#include "GaudiKernel/INamedInterface.h"


namespace SG {
  class DataProxy;
}

/** @class IHiveStoreMgr   
 * @brief the interface through which HiveWB control StoreGate instances 
 *
 * @author Paolo Calafiura - ATLAS
 */

class IHiveStoreMgr : virtual public INamedInterface {
public:
  /// Declare interface ID
  DeclareInterfaceID(IHiveStoreMgr, 1, 0);

  /// clear the store
  virtual StatusCode clearStore(bool forceRemove=false) = 0;

  /** Reset handles added since the last call to commit.
   */
  virtual void commitNewDataObjects() = 0;
  
  virtual ~IHiveStoreMgr() {}

};

#endif // ATHENAKERNEL_IHIVESTOREMGR_H
