/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EFLOWREC_EFLOWTRACKEXTRAPOLATORBASEALGTOOL_H
#define EFLOWREC_EFLOWTRACKEXTRAPOLATORBASEALGTOOL_H

/********************************************************************

NAME:     eflowTrackExtrapolatorBaseAlgTool.h
PACKAGE:  offline/Reconstruction/eflowRec

AUTHORS:  M.Hodgkinson, T.Velz
CREATED:  24th January, 2005

********************************************************************/

#include "GaudiKernel/IAlgTool.h"

#include "eflowCaloRegions.h"
#include "xAODTracking/TrackParticle.h"

class EventContext;
class eflowTrackCaloPoints;

/*
Pure virtual base class, inherits from IAlgTool. Defines execute method which takes xAOD::Track pointer and returns eflowTrackCaloPoints pointer. 
*/
class eflowTrackExtrapolatorBaseAlgTool : virtual public IAlgTool {
 public:
  virtual std::unique_ptr<eflowTrackCaloPoints> execute(const EventContext& ctx, const xAOD::TrackParticle* track) const = 0;
};

#endif
