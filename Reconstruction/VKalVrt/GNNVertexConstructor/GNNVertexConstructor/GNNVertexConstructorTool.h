#ifndef VKalVrt_GNNVertexConstructorTool_H
#define VKalVrt_GNNVertexConstructorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTracking/TrackParticleContainer.h"

//Headers to use the GNN Tool
#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/GNNTool.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"
#include <xAODEventInfo/EventInfo.h>

#include "lwtnn/parse_json.hh"

#include <fstream>
//#include <TH1.h>


class TH1D;
class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

namespace Rec {



    
    class GNNVertexConstructorTool : public AthAlgTool, virtual public IGNNVertexConstructorInterface {
     public: 
       /* Constructor */
      GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~GNNVertexConstructorTool();


      StatusCode initialize();
      StatusCode finalize();
      
      StatusCode initKey(const std::string&, SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> &decokey) const;
      
      unsigned int addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const final;
      virtual StatusCode decorateTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const;
      virtual StatusCode readDecorTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const;
      virtual StatusCode readDecorJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const;
      virtual StatusCode GNNDecoJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const;
      //virtual StatusCode UnionFindAlg( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const;
      
      
      
      //Read and Write Decor Handles
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>  m_decorTrackKey{ this, "TrackDecorKey", ".passGNN", "if track passes some GNN criteria"};
      SG::ReadDecorHandleKey<xAOD::TrackParticleContainer>   m_decorReadKey{ this, "TrackReadKey", "", "read tracks that pass GNN criteria"};
      SG::ReadDecorHandleKey<xAOD::JetContainer>             m_readJetKey{this, "JetReadKey", "", "read Jets from GNN"};
      
      //Gaudi Props      
      Gaudi::Property<std::string>   m_tracksKey { this, "TrackContainername", "InDetTrackParticles", "Track container name (same calling alg)" };
      Gaudi::Property<std::string>   m_readKey { this, "TrackContainerName", "InDetTrackParticle", "Track Container name (same calling alg)"};
      Gaudi::Property<std::string>   m_readJKey{this, "JetContainerName", "", "Jet Container Name"};
      
       
      // @brief the pre-configured GNNTool to use
      ToolHandle<FlavorTagDiscriminants::GNNTool>     m_gnn_Tool{this, "gnn_Tool", "", "GNN Decorator tool"};
      
      //ReadHandles      
      //SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>>   m_jetContainerKey{this, "jetContainerKey", "AntiKt4EMPFlowJets", "xAOD::JetContainer to read"};
      SG::ReadHandleKey<xAOD::EventInfo>                       m_eventInfoKey{this, "eventInfoKey", "EventInfo", "EventInfo container to use"};
      //Deco
      SG::ReadDecorHandleKey<xAOD::JetContainer>               m_jetReadKey{this, "jetDecoReadKey", "", "Jet GNN Deco Read Key"};
      //SG::ReadHandleKey<xAOD::TrackParticleContainer>          m_inTrackLinkKey{this, "InputTrackContainer", "InDetTrackParticles", "Input track particle container"};
      
      /// Gaudi Props
      Gaudi::Property<std::string> m_deco_suffix{this, "suffix", "", "Suffix to add after the decoration"};
      Gaudi::Property<std::string> m_jetContainerName{this, "JetContainerName", "AntiKt4EMPFlowJets", "Jet Container Name"};
      
      
    private:

  
  };
}

#endif
