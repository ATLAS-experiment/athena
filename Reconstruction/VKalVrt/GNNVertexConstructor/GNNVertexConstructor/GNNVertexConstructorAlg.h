#ifndef VKalVrt_GNNVertexConstructorAlg_H
#define VKalVrt_GNNVertexConstructorAlg_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ReadDecorHandleKey.h"
//Headers to use the GNN Tool
#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"

#include "StoreGate/ReadDecorHandle.h"
#include <AthContainers/ConstDataVector.h>
#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"
#include <xAODEventInfo/EventInfo.h>
//#include "PathResolver/PathResolver.h"
#include "lwtnn/parse_json.hh"

#include <fstream>

#include <SystematicsHandles/SysReadHandle.h>



namespace Rec {

   class GNNVertexConstructorAlg : public AthReentrantAlgorithm {
     public: 

       GNNVertexConstructorAlg( const std::string& name, ISvcLocator* pSvcLocator );

       StatusCode initialize() override;
       StatusCode execute(const EventContext &ctx) const override;
       StatusCode finalize() override;

      
    struct GNNProperties {
    
    std::string nnFile = "../network.onnx";
    
    };
      
    private:
      ToolHandle<Rec::IGNNVertexConstructorInterface> m_testTool;
      
      SG::ReadHandleKey<xAOD::TrackParticleContainer> m_inTrackKey{this, "InputTrackContainer", "InDetTrackParticles", "Input track particle container"};      
      
      /// @brief the name of the jet container to use
      SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>> m_jetContainerKey{
      this, "jetContainerKey", "", "xAOD::JetContainer to read"};
      /// @brief the name of the EventInfo object
      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{
      this, "eventInfoKey", "EventInfo", "EventInfo container to use"};
      
      /// @brief the suffix to add to the decorations
      Gaudi::Property<std::string> m_deco_suffix{
      this, "suffix", "", "Suffix to add after the decoration"};
      
      /// @brief the pre-configured GNNTool to use
      ToolHandle<FlavorTagDiscriminants::GNNTool> m_gnn_Tool{
      this, "gnn_Tool", "", "GNN Decorator tool"};
      
      GNNProperties m_props;
      
      SG::ReadDecorHandleKey<xAOD::JetContainer>m_jetReadKey{
      this, "jetDecoReadKey", "", "Jet GNN Deco Read Key"};
      
  };
}

#endif
