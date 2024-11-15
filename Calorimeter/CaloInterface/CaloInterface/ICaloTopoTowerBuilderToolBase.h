/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOINTERFACE_ICALOTOPOTOWERBUILDERTOOLBASE_H
#define CALOINTERFACE_ICALOTOPOTOWERBUILDERTOOLBASE_H
///////////////////////////////////////////////////////////////////////////////
/// \brief ICaloTopoTowerBuilderToolBase is abstract interface for CALOTOPOTOWERBUILDERTOOLBASE
///
/// ICaloTopoTowerBuilderToolBase is an abstract class to access tower builder 
/// tool.
///
///     Concrete tools, derived from the CaloTowerBuilderToolBase base abstract
///	class are controlled via this interface.
///
/// \author Luca Fiorini <Luca.Fiorini@cern.ch>
/// \date October 4, 2007 -
///
///////////////////////////////////////////////////////////////////////////////

#include "GaudiKernel/IAlgTool.h"

#include <string>

class CaloTopoTowerContainer;
class CaloTowerSeg;
class CaloCellContainer;
class CaloClusterContainer;
class EventContext;

class ICaloTopoTowerBuilderToolBase : virtual public extend_interfaces<IAlgTool>
{
 public:
  DeclareInterfaceID(ICaloTopoTowerBuilderToolBase, 1 , 0);

  //Virtual destructor
  virtual ~ICaloTopoTowerBuilderToolBase() {}

  /// execute is abstract
  virtual StatusCode execute(const EventContext& ctx,
                             CaloTopoTowerContainer* theContainer,const CaloCellContainer* theCell=0) const = 0;

  virtual StatusCode initializeTool() = 0;

  virtual void setTowerSeg(const CaloTowerSeg& theTowerSeg) = 0;

};
#endif
