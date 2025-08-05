/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
//
// ITwoTrackVertexSelector.h - Description
//
/*
   Interface for good 2-track vertex selection during track-track compatibility graph 
   construction for inclusive vertex reconstruction
   
   
    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch
-----------------------------------------------------------------------------*/


#ifndef _Rec_ITwoTrackVertexSelector_H
#define _Rec_ITwoTrackVertexSelector_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "Math/Vector4D.h"
 
//------------------------------------------------------------------------
namespace Rec {

//------------------------------------------------------------------------
  static const InterfaceID IID_ITwoTrackVertexSelector("ITwoTrackVertexSelector", 1, 0);

  class ITwoTrackVertexSelector : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_ITwoTrackVertexSelector;}
//---------------------------------------------------------------------------

  /** @class ITwoTrackVertexSelector
    Interface class to select good 2-track vertex for inclusive vertexing
  */
     virtual bool isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> tracks,
		                      const xAOD::Vertex & candV,
                                std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
		                      const xAOD::Vertex & tPV) const =0;
     virtual bool isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> tracks,
		                      const xAOD::Vertex & candV,
                                std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
		                      const xAOD::Vertex & tPV,
                          float & quality) const =0;
  };

}  //end namespace

#endif
