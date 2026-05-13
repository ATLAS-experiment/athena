/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//
// IVrtInclusive.h - Description
//
/*
   Interface for inclusive secondary vertex reconstruction
   It returns a pointer to Trk::VxSecVertexInfo object which contains
   vector of pointers to xAOD::Vertex's of found secondary verteces.
   In case of failure pointer to Trk::VxSecVertexInfo is 0.
   

   Tool creates a derivative object VxSecVKalVertexInfo which contains also additional variables
   see  Tracking/TrkEvent/VxSecVertex/VxSecVertex/VxSecVKalVertexInfo.h
   
    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch

-----------------------------------------------------------------------------*/



#ifndef _Rec_IVrtSecInclusive_H
#define _Rec_IVrtSecInclusive_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "VxSecVertex/VxSecVertexInfo.h"

class EventContext;

//------------------------------------------------------------------------
namespace Rec {

  class IVrtInclusive : virtual public IAlgTool {
    public:
      DeclareInterfaceID(IVrtInclusive, 1, 0);

  /** @class IVrtInclusive

    Interface class for inclusive vertex reconstruction.
    All secondary vertices produced in a phase space around Primary Vertex(PV) and resulted in secondary tracks 
    provided by inputTracks collection are returned in Trk::VxSecVertexInfo.
    Interface assumes that necessary TrackParticles can be pre-selected by user before calling the actual tool implementation
    with default track quality cuts.
    
  */
      virtual std::unique_ptr<Trk::VxSecVertexInfo> findAllVertices( const EventContext& ctx,
                                                                     const std::vector<const xAOD::TrackParticle*> & inputTracks,
                                                                     const xAOD::Vertex & PV) const = 0;

  };

}  //end namespace

#endif
