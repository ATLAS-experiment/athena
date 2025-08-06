/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGCALOEVENT_TRIGT2JETCONTAINER_H
#define TRIGCALOEVENT_TRIGT2JETCONTAINER_H
/********************************************************************

NAME:     TrigT2JetContainer.h
PACKAGE:  
 
AUTHORS:  Kyle Cranmer
CREATED:  Oct. 05

********************************************************************/

// INCLUDE HEADER FILES:

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "TrigCaloEvent/TrigT2Jet.h"
#include "AthenaKernel/BaseInfo.h"

/** container of TrigT2Jet elements */
class TrigT2JetContainer : public DataVector<TrigT2Jet> 
{
 public:
 
  /** Destructor */
  virtual ~TrigT2JetContainer()  {  } ;
  
};

CLASS_DEF(TrigT2JetContainer, 1178384516, 1)
SG_BASE(TrigT2JetContainer, DataVector<TrigT2Jet>);
#endif
