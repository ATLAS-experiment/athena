/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// IVertexCascadeFitter.h  - 
//---------------------------------------------------------------
#ifndef TRKVKALVRTFITTER_IVERTEXCASCADEFITTER_H
#define TRKVKALVRTFITTER_IVERTEXCASCADEFITTER_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
//
#include  "xAODTracking/TrackParticleFwd.h"
#include  "TrkVKalVrtFitter/IVKalState.h"
#include <memory>
#include <vector>
#include <span>

class EventContext;

namespace Trk{
  class Vertex;
  class IVKalState;
  class VxCascadeInfo;

 typedef int VertexID;

//------------------------------------------------------------------------
  static const InterfaceID IID_IVertexCascadeFitter("IVertexCascadeFitter", 1, 0);

  class IVertexCascadeFitter : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_IVertexCascadeFitter;}
//---------------------------------------------------------------------------
//Interface itself

     /*
      * Context aware method
      */
      virtual std::unique_ptr<IVKalState> makeState(const EventContext& ctx) const = 0;
      
     /*
      * For non-migrated clients whcih should always use the context aware method
      */
      virtual std::unique_ptr<IVKalState> makeState() const 
          {
              return makeState(Gaudi::Hive::currentContext());
          }
      
    virtual VertexID startVertex(const  std::vector<const xAOD::TrackParticle*> & list,
                                   std::span<const double> particleMass,
                                   IVKalState& istate,
				   double massConstraint = 0.) const = 0;
 
      virtual VertexID  nextVertex(const  std::vector<const xAOD::TrackParticle*> & list,
                                   std::span<const double> particleMass,
                                   IVKalState& istate,
				   double massConstraint = 0.) const = 0;
 
      virtual VertexID  nextVertex(const  std::vector<const xAOD::TrackParticle*> & list,
                                   std::span<const double> particleMass,
		                   const  std::vector<VertexID> &precedingVertices,
                                   IVKalState& istate,
				   double massConstraint = 0.) const = 0;

      virtual VxCascadeInfo * fitCascade(IVKalState& istate,
                                         const Vertex * primVertex = 0, bool FirstDecayAtPV = false ) const = 0;

      virtual StatusCode  addMassConstraint(VertexID Vertex,
                                         const std::vector<const xAOD::TrackParticle*> & tracksInConstraint,
                                         const std::vector<VertexID> &verticesInConstraint, 
                                         IVKalState& istate,
				         double massConstraint ) const = 0;

   };

} //end of namespace

#endif
