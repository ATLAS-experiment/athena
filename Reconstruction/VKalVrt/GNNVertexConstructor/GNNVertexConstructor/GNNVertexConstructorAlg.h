#ifndef VKalVrt_GNNVertexConstructorAlg_H
#define VKalVrt_GNNVertexConstructorAlg_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"
#include "GaudiKernel/ToolHandle.h"


namespace Rec {

   class GNNVertexConstructorAlg : public AthReentrantAlgorithm {
     public: 

       GNNVertexConstructorAlg( const std::string& name, ISvcLocator* pSvcLocator );

       StatusCode initialize() override;
       StatusCode execute(const EventContext &ctx) const override;
       StatusCode finalize() override;

    private:
      ToolHandle<Rec::IGNNVertexConstructorInterface> m_testTool;
      
        

  };
}

#endif