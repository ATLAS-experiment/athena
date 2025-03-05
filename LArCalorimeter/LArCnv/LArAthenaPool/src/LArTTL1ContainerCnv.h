/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LAREVENTATHENAPOOL_LARTTL1CONTAINERCNV_H 
#define LAREVENTATHENAPOOL_LARTTL1CONTAINERCNV_H 

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

// LArTPCnv includes
#include "LArTPCnv/LArTTL1Container_p1.h"

// LArRawEvent includes
#include "LArRawEvent/LArTTL1Container.h"

/*
  The persistent type is typedef-ed to the transient type to avoid schema change.
  This choice also affects the implementation of createTransient (see the implementation).
*/
typedef LArTTL1Container  LArTTL1Container_PERS;

class LArTTL1ContainerCnv: public T_AthenaPoolCustomCnv<
                                      LArTTL1Container, 
                                      LArTTL1Container_PERS 
                                   > 

{

  // make the factory for this converter our friend
  friend class CnvFactory<LArTTL1ContainerCnv>;

 protected:

  /** Create the converter from the service locator
   */
public:
  LArTTL1ContainerCnv(ISvcLocator* svcloc);
protected:

  /** Build the persistent representation from the transient one.
   */
  virtual LArTTL1Container_PERS*
    createPersistent( LArTTL1Container* transCont );
  
  /** Build the transient representation from a persistent one
   */
  virtual LArTTL1Container* createTransient();

};

inline LArTTL1ContainerCnv::LArTTL1ContainerCnv( ISvcLocator* svcLocator ) :
  T_AthenaPoolCustomCnv<LArTTL1Container, LArTTL1Container_PERS>(svcLocator)
{}

#endif //> LAREVENTATHENAPOOL_LARTTL1CONTAINERCNV_H
