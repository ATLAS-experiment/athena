#ifndef VKalVrt_IGNNVertexConstructorInterface_H
#define VKalVrt_IGNNVertexConstructorInterface_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODTracking/TrackParticleContainer.h"
 
//------------------------------------------------------------------------
namespace Rec {

//------------------------------------------------------------------------
  static const InterfaceID IID_IGNNVertexConstructorInterface("IGNNVertexConstructorInterface", 1, 0);

  class IGNNVertexConstructorInterface : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_IGNNVertexConstructorInterface;}
//---------------------------------------------------------------------------

  /** @class IGNNVertexConstructorInclusive

    Interface class for GNN Vertex Constructor tool
    
    
  */
      virtual unsigned int addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const =0;
      virtual StatusCode decorateTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const = 0;

  };

}  //end namespace

#endif
