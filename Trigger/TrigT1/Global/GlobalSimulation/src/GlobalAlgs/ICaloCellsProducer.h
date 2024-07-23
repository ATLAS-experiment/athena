/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_ICELLSPRODUCER_H
#define GLOBALSIM_ICELLSPRODUCER_H

#include "GaudiKernel/IAlgTool.h"
#include "Identifier/Identifier.h"

#include "GaudiKernel/EventContext.h"

class CaloCell;

namespace GlobalSim {
  class ICaloCellsProducer : virtual public IAlgTool {
    /** PABC (Pure Abstract Base Class) for CaloCellProducers */

  public:
    DeclareInterfaceID(ICaloCellsProducer, 1, 0);
    virtual ~ICaloCellsProducer(){};
  
  
    /** obtain a vector of CaloCells. The implementation classes
	determine the choice of the Cells. */
  
    virtual StatusCode cells(std::vector<const CaloCell*>&,
			     const EventContext&) const = 0;
  };
}
#endif
