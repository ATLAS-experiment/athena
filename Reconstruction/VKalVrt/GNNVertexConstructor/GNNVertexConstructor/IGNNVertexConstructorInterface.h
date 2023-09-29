#ifndef VKalVrt_IGNNVertexConstructorInterface_H
#define VKalVrt_IGNNVertexConstructorInterface_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODJet/JetContainer.h"
 
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
      virtual StatusCode readDecorJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const = 0;
      virtual StatusCode GNNDecoJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const = 0;
      
  };

}  //end namespace

#endif
