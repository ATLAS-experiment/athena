#ifndef VKalVrt_GNNVertexConstructorTool_H
#define VKalVrt_GNNVertexConstructorTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GNNVertexConstructor/IGNNVertexConstructorInterface.h"

#include "xAODTracking/TrackParticleContainer.h"

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
      
      SG::WriteDecorHandleKey<xAOD::TrackParticleContainer>  m_decorTrackKey{ this, "TrackDecorKey", ".passGNN", "if track passes some GNN criteria"};
      Gaudi::Property<std::string>   m_tracksKey { this, "TrackContainername", "InDetTrackParticles", "Track container name (same calling alg)" };


    private:
  };
}

#endif
