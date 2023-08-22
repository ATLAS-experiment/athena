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
//Header for Data Containers
#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODCore/AuxContainerBase.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "lwtnn/parse_json.hh"
#include <vector>
#include "GaudiKernel/ServiceHandle.h"
//Remove in boost > 1.76 when the boost iterator issue
//is solved see ATLASRECTS-6358
#define BOOST_ALLOW_DEPRECATED_HEADERS
#include "boost/graph/adjacency_list.hpp"

#include "BeamSpotConditionsData/BeamSpotData.h"
#include "VxSecVertex/VxSecVertexInfo.h"


#include <fstream>
//#include <TH1.h>


class TH1D;
class TH2D;
class TH1F;
class TProfile;
class TTree;
class ITHistSvc;

namespace Trk{
  class TrkVKalVrtFitter;
  class IVertexFitter;
  class IVKalState;
}

namespace Rec {

  struct workVectorArrxAOD{
        std::vector<const xAOD::TrackParticle*> listSelTracks;  // Selected tracks after quality cuts
        std::vector<const xAOD::TrackParticle*> tmpListTracks;
        std::vector<const xAOD::TrackParticle*> inpTrk;         // All tracks provided to tool
        double beamX=0.;
        double beamY=0.;
        double beamZ=0.;
        double tanBeamTiltX=0.;
        double tanBeamTiltY=0.;
  };



    class GNNVertexConstructorTool : public AthAlgTool, virtual public IGNNVertexConstructorInterface {
     public: 
       /* Constructor */
      GNNVertexConstructorTool(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~GNNVertexConstructorTool();


      StatusCode initialize();
      StatusCode finalize();
      
      StatusCode initKey(const std::string&, SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> &decokey) const;
      
      unsigned int addTwoNumbers( const unsigned int & NoOne, const unsigned int & NoTwo) const final;                    //can be removed later
      virtual StatusCode decorateTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const;    //can be removed later
      virtual StatusCode readDecorTracks( const xAOD::TrackParticleContainer* trkCont, const EventContext& ctx ) const;   //can be removed later
      
      
      virtual StatusCode readDecorJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const; 
      
                 
      //virtual StatusCode vrtFitter( std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > & vrt ) const;
      
      virtual StatusCode GNNDecoJet( const xAOD::JetContainer* jetCont, const EventContext& ctx ) const;
      
      
      
      
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
      
      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
      ToolHandle<Trk::IExtrapolator>  m_extrapolator{this,"ExtrapolatorName","Trk::Extrapolator/Extrapolator"};
      ToolHandle<Trk::TrkVKalVrtFitter> m_VrtFit;
      const xAOD::Vertex*  m_thePV;
      const xAOD::VertexContainer*        m_vertexTES; // primary vertex container
      
      //{this, "SecVtxToo", "", "Secondary Vertexing Tool"};

      //ReadHandles      
      //SG::ReadHandleKey<ConstDataVector<xAOD::JetContainer>>   m_jetContainerKey{this, "jetContainerKey", "AntiKt4EMPFlowJets", "xAOD::JetContainer to read"};
      SG::ReadHandleKey<xAOD::EventInfo>                       m_eventInfoKey{this, "eventInfoKey", "EventInfo", "EventInfo container to use"};
      //Deco
      SG::ReadDecorHandleKey<xAOD::JetContainer>               m_jetReadKey_TO{this, "jetDecoReadKey", "", "Jet GNN Deco Read Key for track origin"};
      SG::ReadDecorHandleKey<xAOD::JetContainer>               m_jetReadKey_TL{this, "jetDecoReadKey", "", "Jet GNN Deco Read Key for track link"};
      SG::ReadDecorHandleKey<xAOD::JetContainer>               m_jetReadKey_TV{this, "jetDecoReadKey", "", "Jet GNN Deco Read Key for track link"};
      //SG::ReadHandleKey<xAOD::TrackParticleContainer>          m_inTrackLinkKey{this, "InputTrackContainer", "InDetTrackParticles", "Input track particle container"};
      
      /// Gaudi Props
      Gaudi::Property<std::string> m_deco_suffix{this, "suffix", "", "Suffix to add after the decoration"};
      Gaudi::Property<std::string> m_jetContainerName{this, "JetContainerName", "AntiKt4EMPFlowJets", "Jet Container Name"};
      
      
    private:
    
    std::vector<xAOD::Vertex*> vrtFitter( workVectorArrxAOD * inpParticlesxAOD, std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >  > & vrt ) const;
      

     using compatibilityGraph_t = boost::adjacency_list<boost::listS, boost::vecS, boost::undirectedS>;
      float m_chiScale[11]{};
      struct WrkVrt 
      {  bool Good=true;
         std::deque<long int> selTrk;
         Amg::Vector3D     vertex;
         TLorentzVector    vertexMom;
         long int   vertexCharge{};
         std::vector<double> vertexCov;
         std::vector<double> chi2PerTrk;
         std::vector< std::vector<double> > trkAtVrt;
         double chi2{};
         double projectedVrt=0.;
         int detachedTrack=-1;
         double BDT=1.1;
      };
      
      void select2TrVrt(std::vector<const xAOD::TrackParticle*> & SelectedTracks, const xAOD::Vertex  & primVrt,
                        std::multimap<int, ElementLink<DataVector<xAOD::TrackParticle_v1 > >> & vrt,
                        compatibilityGraph_t& compatibilityGraph) const;


/*    struct clique_visitor
      {
        clique_visitor(std::vector< std::vector<int> > & input): m_allCliques(input){ input.clear();}
        
        template <typename Clique, typename Graph>
        void clique(const Clique& clq, Graph& )
        { 
          std::vector<int> new_clique(0);
          for(auto i = clq.begin(); i != clq.end(); ++i) new_clique.push_back(*i);
          m_allCliques.push_back(new_clique);
        }
    
        std::vector< std::vector<int> > & m_allCliques;
    
      };*/

    
  };
}

#endif
