#ifndef VKalVrt_IGNNVertexConstructorInterface_H
#define VKalVrt_IGNNVertexConstructorInterface_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
 
namespace Rec {

  static const InterfaceID IID_IGNNVertexConstructorInterface("IGNNVertexConstructorInterface", 1, 0);

  class IGNNVertexConstructorInterface : virtual public IAlgTool {
    public:
      static const InterfaceID& interfaceID() { return IID_IGNNVertexConstructorInterface;}

      virtual StatusCode performVertexFit( const xAOD::JetContainer* jetCont, xAOD::VertexContainer* vertexCont, const EventContext& ctx ) const = 0;
      
  };

}

#endif
