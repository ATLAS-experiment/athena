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

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

//#include "PathResolver/PathResolver.h"
#include "lwtnn/parse_json.hh"

#include <fstream>


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
      
      
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>  m_decorTrackKey{ this, "TrackDecorKey", ".passGNN", "if track passes some GNN criteria"};
      Gaudi::Property<std::string>   m_tracksKey { this, "TrackContainername", "InDetTrackParticles", "Track container name (same calling alg)" };

      SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_decorReadKey{ this, "TrackReadKey", "", "read tracks that pass GNN criteria"};
      Gaudi::Property<std::string>  m_readKey { this, "TrackContainerName", "InDetTrackParticle", "Track Container name (same calling alg)"};
      
      
    private:
  };
}

#endif
