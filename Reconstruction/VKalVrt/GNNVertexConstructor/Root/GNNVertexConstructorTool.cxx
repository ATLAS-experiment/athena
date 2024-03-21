// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
// Headers to Read & Write Decorations
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "ranges"
#include <boost/iterator/zip_iterator.hpp>
#include "TH1F.h"
#include "PathResolver/PathResolver.h"

namespace Rec {

GNNVertexConstructorTool::GNNVertexConstructorTool(const std::string &type, const std::string &name,
                                                   const IInterface *parent)
    : AthAlgTool(type, name, parent),
      m_vertexFitterTool("Trk::TrkVKalVrtFitter/VertexFitterTool", this),
      m_jetCollection("AntiKt4EMPFlowJets"),
      m_Xbeampipe (0.),
      m_Ybeampipe (0.),
      m_XlayerB (0.),
      m_YlayerB (0.),
      m_Xlayer1 (0.),
      m_Ylayer1 (0.),
      m_Xlayer2 (0.),
      m_Ylayer2 (0.),
      m_Rbeampipe (0.),  //Correct values are filled     
      m_RlayerB   (0.),  // in jobO or initialize()
      m_Rlayer1   (0.),
      m_Rlayer2   (0.),
      m_MultiWithPrimary(true),
      m_fitHists(false),
      m_minD0(1),
      m_minSig3D(20),
      m_maxChi2(20),
      m_minPerp(0.5),
      m_SingleHFTrack(false),
      m_HFTrackRatio(false),
      m_HFRatioThres(0.3)            
      {
  declareInterface<IGNNVertexConstructorInterface>(this);
  declareProperty("JetTrackLinks", m_trackLinksKey = "AntiKt4EMPFlowJetsAuxDyn.TrackLinks");
  declareProperty("JetTrackLinks", m_trackOriginsKey = "AntiKt4EMPFlowJetsAuxDyn.GN2v01_TrackOrigin");
  declareProperty("JetVertexLinks",m_vertexLinksKey = "AntiKt4EMPFlowJetsAuxDyn.GN2v01_VertexIndex");
  declareProperty("GNNTool", m_gnn_Tool, "GNN Tool");
  declareProperty("VertexFitterTool", m_vertexFitterTool, "Vertex fitting tool");
  declareProperty("Xbeampipe", m_Xbeampipe);
  declareProperty("Ybeampipe", m_Ybeampipe);
  declareProperty("XlayerB",   m_XlayerB  );
  declareProperty("YlayerB",   m_YlayerB  );
  declareProperty("Xlayer1",   m_Xlayer1  );
  declareProperty("Ylayer1",   m_Ylayer1  );
  declareProperty("Xlayer2",   m_Xlayer2  );
  declareProperty("Ylayer2",   m_Ylayer2  );
  declareProperty("Rbeampipe", m_Rbeampipe);
  declareProperty("RlayerB",   m_RlayerB  );
  declareProperty("Rlayer1",   m_Rlayer1  );
  declareProperty("Rlayer2",   m_Rlayer2  );
  declareProperty("fitHists", m_fitHists, "Fit extra histograms on numer of vertices within jet and combined energy fraction");
  declareProperty("mind0", m_minD0, "D0 cut on vertices");
  declareProperty("minSig3D", m_minSig3D, "Sig 3D cut on vertices");
  declareProperty("maxChi2", m_maxChi2, "Maximum Chi2 for fitted vertices");
  declareProperty("minPerp", m_minPerp, "Perpendicular distance from primary cut on fitted vertices");
  declareProperty("SingleHFTracks", m_SingleHFTrack, "Select any vertice with at least one HF track identified by the GNN");
  declareProperty("HFTrackRatio", m_HFTrackRatio, "Select any vertice that passes the threshold for number of HF tracks to all tracks in vertice");
  declareProperty("HFRatio", m_HFRatioThres, "The threshold for the ratio between HF tracks and all tracks for a vertex");
  m_massPi  = 139.5702 ;
}

/* Destructor */
GNNVertexConstructorTool::~GNNVertexConstructorTool() {
  ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
}

StatusCode GNNVertexConstructorTool::initialize() {

  ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

   if( m_Rbeampipe==0.)  m_Rbeampipe=24.0;    
   if( m_RlayerB  ==0.)  m_RlayerB  =34.0;
   if( m_Rlayer1  ==0.)  m_Rlayer1  =51.6;
   if( m_Rlayer2  ==0.)  m_Rlayer2  =90.0;
   m_Rlayer3  =122.5;
  
  // Initialize keys
  ATH_CHECK(m_trackLinksKey.initialize());
  ATH_CHECK(m_trackOriginsKey.initialize());
  ATH_CHECK(m_vertexLinksKey.initialize());

  m_jetWriteDecorKeyVertexLink = m_jetCollection + ".GNNVerticesLink";
  ATH_CHECK( m_jetWriteDecorKeyVertexLink.initialize()); 

  m_jetWriteDecorKeyVertexNumber = m_jetCollection + ".GNNVerticesNumber";
  ATH_CHECK( m_jetWriteDecorKeyVertexNumber.initialize()); 


  //ADD a config for histogramming
  //Hists
  if (m_fitHists==true){
    ATH_CHECK(service("THistSvc", m_thistSvc));
    StatusCode sc; 
    m_eFrac                   = new TH1F("eFrac","Energy Fraction of vertices within a jet and jet energy",25,0,1);  
    ATH_CHECK( m_thistSvc->regHist("/GNNPlots/EFrac/", m_eFrac));
    m_vertexN                 = new TH1F("vertexN","No. of Vertices within a Jet",8,-0.5,7.5);  
    ATH_CHECK(m_thistSvc->regHist("/GNNPlots/VertexN/", m_vertexN));
  }
  // Retrieve tools
  ATH_CHECK(m_gnn_Tool.retrieve());
  ATH_CHECK(m_vertexFitterTool.retrieve());
  //Additional Info for Vertex Fit
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  return StatusCode::SUCCESS;
}

//Use GNN to decorate the tracks || May be removed in future
StatusCode GNNVertexConstructorTool::decorateJets(const xAOD::JetContainer *jetCont) const {

  for (auto jet : *jetCont) {
    m_gnn_Tool->decorate(*jet);
  }
  
  return StatusCode::SUCCESS;
}

//Total Momentum of Jet
TLorentzVector GNNVertexConstructorTool::TotalMom(const std::vector<const xAOD::TrackParticle*>& selTrk) 
const
{
   TLorentzVector sum(0.,0.,0.,0.); 
   for (int i = 0; i < (int)selTrk.size(); ++i) {
     if( selTrk[i] == NULL ) continue; 
     sum += selTrk[i]->p4();
   }
   return sum; 
 }

//Vertex to Vertex Distance
double GNNVertexConstructorTool::vrtVrtDist(const xAOD::Vertex & primVrt, const Amg::Vector3D & secVrt, 
                                          const std::vector<double>& secVrtErr, double& signif)
  const
  {
    double distx =  primVrt.x()- secVrt.x();
    double disty =  primVrt.y()- secVrt.y();
    double distz =  primVrt.z()- secVrt.z();

    AmgSymMatrix(3)  primCovMtx=primVrt.covariancePosition();  //Create
    primCovMtx(0,0) += secVrtErr[0];
    primCovMtx(0,1) += secVrtErr[1];
    primCovMtx(1,0) += secVrtErr[1];
    primCovMtx(1,1) += secVrtErr[2];
    primCovMtx(0,2) += secVrtErr[3];
    primCovMtx(2,0) += secVrtErr[3];
    primCovMtx(1,2) += secVrtErr[4];
    primCovMtx(2,1) += secVrtErr[4];
    primCovMtx(2,2) += secVrtErr[5];

    AmgSymMatrix(3)  wgtMtx = primCovMtx.inverse();

    signif = distx*wgtMtx(0,0)*distx
            +disty*wgtMtx(1,1)*disty
            +distz*wgtMtx(2,2)*distz
         +2.*distx*wgtMtx(0,1)*disty
         +2.*distx*wgtMtx(0,2)*distz
         +2.*disty*wgtMtx(1,2)*distz;
    signif=std::sqrt(std::abs(signif));
    if( signif!=signif ) signif = 0.;
    return std::sqrt(distx*distx+disty*disty+distz*distz);
  }

//Perform Vertex fit using the jet decorations of the GNN
StatusCode GNNVertexConstructorTool::performVertexFit(const xAOD::JetContainer *inJetContainer,
                            xAOD::VertexContainer *outVertexContainer,
                            const xAOD::Vertex & primVrt,
                            const EventContext &ctx) const {

  using TLC = std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1>>>;
  using TL  = ElementLink<DataVector<xAOD::TrackParticle_v1>>;

  //Read Decor Handle for Track links and Vertex links
  SG::ReadDecorHandle<xAOD::JetContainer, TLC> trackLinksHandle(m_trackLinksKey, ctx);
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char>> trackOriginsHandle(m_trackOriginsKey, ctx);
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char>>
      vertexLinksHandle(m_vertexLinksKey, ctx);
  SG::WriteDecorHandle< xAOD::JetContainer, std::vector<ElementLink<xAOD::VertexContainer>>> 
      jetWriteDecorHandleVertexLink (m_jetWriteDecorKeyVertexLink, ctx);
  
  // Vertex decorators
  SG::AuxElement::Decorator<float> decor_mass("mass");
  SG::AuxElement::Decorator<float> decor_pT("pt");
  SG::AuxElement::Decorator<float> decor_charge("charge");
  SG::AuxElement::Decorator<float> decor_vPos("vPos");
  SG::AuxElement::Decorator<float> decor_Lxy("Lxy");
  SG::AuxElement::Decorator<float> decor_significance3d("significance3d");
  SG::AuxElement::Decorator<float> decor_deltaR("deltaR");
  SG::AuxElement::Decorator<float> decor_NGTinSvx("NGTinSvx");
  SG::AuxElement::Decorator<float> decor_L3D("L3d");
  SG::AuxElement::Decorator<float> decor_N2Tpair("N2Tpair");
  SG::AuxElement::Decorator<float> decor_minDstMat("minDstMat");
  SG::AuxElement::Decorator<float> decor_efracsv("efracsv");
  SG::AuxElement::Decorator<float> decor_badChi2("badChi");
  SG::AuxElement::Decorator<float> decor_JetN("JetN");
  
  // Loop over the jets
  for (const auto &jet : *inJetContainer) {
        
    //Retrieve the Vertex and Track Collections
    auto vertexCollection = vertexLinksHandle(*jet);
    auto trackCollection = trackLinksHandle(*jet);
    auto trackOriginCollection = trackOriginsHandle(*jet);

    using indexList            = std::vector< int >;
    using vertexHFMap          = std::map< char, bool>;
    using trackCountMap        = std::map< char, std::set<TL> >;
       
    indexList iList(vertexCollection.size());
        
    vertexHFMap   v2HFMap;
    trackCountMap v2TLMap;
    trackCountMap v2NHFTLMap;
    trackCountMap FittingMap;
    
    FittingMap.clear();
    
    std::for_each(boost::make_zip_iterator(boost::make_tuple(vertexCollection.cbegin(), trackOriginCollection.cbegin(), trackCollection.cbegin())),
		boost::make_zip_iterator(boost::make_tuple(vertexCollection.cend(), trackOriginCollection.cend(), trackCollection.cend())),
		[&v2HFMap, &v2NHFTLMap, &v2TLMap, &FittingMap](const boost::tuple<const char &, const char &, const TL &> &e)
		{
		  auto v  = e.get<0>();
		  auto to = e.get<1>();
      auto tl = e.get<2>();    
      
      v2TLMap[v].insert(tl);
      
      if (InDet::ExclusiveOrigin::FromB==to||InDet::ExclusiveOrigin::FromBC==to|| InDet::ExclusiveOrigin::FromC==to){    //Checking if vertex has a heavy flavour track
        v2HFMap[v]=(true);
        v2NHFTLMap[v].insert(tl);
      }
    });
    
    auto InclusiveFunc =[&FittingMap](const auto &c){
      const auto&[v,tcm] =c;
        FittingMap.insert(std::pair<char, std::set<TL>>(v, tcm));
    };
    
    auto HFRatioFunc =[&v2NHFTLMap, &HFRatio=m_HFRatioThres, &FittingMap](const auto &d){
      const auto&[v,tcm] =d;
      if(v2NHFTLMap.find(v)!=v2NHFTLMap.end() && (static_cast<float>(v2NHFTLMap[v].size())/tcm.size())>=HFRatio){
        FittingMap.insert(std::pair<char, std::set<TL>>(v, tcm));
      };    
    };

    auto vertex2trackOrigin = [&v2HFMap, &FittingMap](const auto &e){
      const auto &[v, tcm] = e;
      if (v2HFMap.find(v) !=v2HFMap.end() && v2HFMap[v]==true)  //if exists and is true
      {
        FittingMap.insert(std::pair<char, std::set<TL>>(v, tcm));  //check if copies set or just takes a reference - 
      };
    };    
    
    if(m_SingleHFTrack==true){
      ATH_MSG_INFO("At least ONE HF Track vertex");
      std::for_each(v2TLMap.cbegin(), v2TLMap.cend(), vertex2trackOrigin);    
    }
    else if(m_HFTrackRatio==true){
      ATH_MSG_INFO("Vertex with HF Ratio ");
      std::for_each(v2TLMap.cbegin(), v2TLMap.cend(), HFRatioFunc);    
    }
    else if (m_SingleHFTrack==false && m_HFTrackRatio==false){
      ATH_MSG_INFO("No Requirement on Track Origins");
      std::for_each(v2TLMap.cbegin(), v2TLMap.cend(), InclusiveFunc);    
    }
    //Working xAOD
    workVectorArrxAOD *xAODwrk = new workVectorArrxAOD();
    SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};

    //Beam Conditions
    xAODwrk->beamX = beamSpotHandle->beamPos().x();
    xAODwrk->beamY = beamSpotHandle->beamPos().y();
    xAODwrk->beamZ = beamSpotHandle->beamPos().z();
    xAODwrk->tanBeamTiltX = tan(beamSpotHandle->beamTilt(0));
    xAODwrk->tanBeamTiltY = tan(beamSpotHandle->beamTilt(1));

    std::unique_ptr<std::vector<WrkVrt>> wrkVrtSet = std::make_unique<std::vector<WrkVrt>>();
    WrkVrt newvrt;
    newvrt.Good = true;
    std::unique_ptr<Trk::IVKalState> state = m_vertexFitterTool->makeState();
    std::vector<const xAOD::NeutralParticle *> neutralPartDummy(0);
    Amg::Vector3D IniVrt(0.,0.,0.);

    float vertexN=1;
    float TrackE=0;
      
    for(const auto& pair : FittingMap){
          
      if (pair.second.size() >= 2) {
      
        // Need at least 2 tracks to perform a fit
        int NTRKS =pair.second.size();        
        std::vector<double> InpMass(NTRKS,m_massPi);
        m_vertexFitterTool->setMassInputParticles( InpMass, *state);

        xAODwrk->listSelTracks.clear();

        xAODwrk->listSelTracks.clear();
//        //newvrt.selTrk.clear();
              
        //Retrieve the tracks and push to working xAOD
//        for (auto i = elements.first; i != elements.second; ++i) {
//          xAODwrk->listSelTracks.push_back(*(i->second));
//          //ATH_MSG_DEBUG((*(i->second))->d0());
//          }
       
       ATH_MSG_DEBUG("#Tracks test " << xAODwrk->listSelTracks.size());
        
       Amg::Vector3D FitVertex, vDist;
       TLorentzVector jetDir(jet->p4().Px(),jet->p4().Py(),jet->p4().Pz(), jet->p4().E()); //Jet Direction
        
       //Get Estimate
       StatusCode sc=m_vertexFitterTool->VKalVrtFitFast(xAODwrk->listSelTracks, FitVertex, *state);
        
        if(sc.isFailure() || FitVertex.perp()>m_Rlayer2*2){  /* No initial estimation */
          IniVrt=primVrt.position();
          if( m_MultiWithPrimary ) IniVrt.setZero();
        }else{
            vDist=FitVertex-primVrt.position();
            double JetVrtDir = jetDir.Px()*vDist.x() + jetDir.Py()*vDist.y() + jetDir.Pz()*vDist.z();
            if( m_MultiWithPrimary ) JetVrtDir=fabs(JetVrtDir); /* Always positive when primary vertex is seeked for*/ 
            if( JetVrtDir>0. ) IniVrt=FitVertex;                /* Good initial estimation */ 
            else               IniVrt=primVrt.position();
        }
   
        m_vertexFitterTool->setApproximateVertex( IniVrt.x(), IniVrt.y(), IniVrt.z(), *state );
         
        //Perform the Vertex Fit
        sc = (m_vertexFitterTool->VKalVrtFit(
            xAODwrk->listSelTracks, neutralPartDummy, newvrt.vertex, newvrt.vertexMom,
            newvrt.vertexCharge, newvrt.vertexCov, newvrt.chi2PerTrk, newvrt.trkAtVrt, newvrt.chi2,
            *state, false));          
        if (sc.isFailure())          continue;

        //Chi2 Cut       
        auto NDOF = 2*(newvrt.trkAtVrt.size())-3.0;  //From VrtSecInclusive
        
        if (newvrt.chi2/NDOF>=m_maxChi2 ) continue;
        ATH_MSG_DEBUG("Found IniVertex=" << newvrt.vertex[0] << ", " << newvrt.vertex[1] << ", "
                                         << newvrt.vertex[2] << " trks " << newvrt.trkAtVrt.size());

        Amg::Vector3D vDir = newvrt.vertex - primVrt.position();  //Vertex Dirction in relation to Primary
               
        Amg::Vector3D jetVrtDir(jet->p4().Px(),jet->p4().Py(),jet->p4().Pz());
        
        double vPos = (vDir.x() * newvrt.vertexMom.Px() + vDir.y() * newvrt.vertexMom.Py() +
                       vDir.z() * newvrt.vertexMom.Pz()) /
                      newvrt.vertexMom.Rho();
        
        double Lxy=sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]);
        double L3D =sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]+vDir[2]*vDir[2]);
        ATH_MSG_DEBUG("L3D  " << L3D);
        
        double drJPVSV = Amg::deltaR(jetVrtDir,vDir); //DeltaR

        int NGTatVtx=newvrt.trkAtVrt.size(); //# Tracks in Vertex
       
        double xvt=newvrt.vertex[0]; double yvt=newvrt.vertex[1];
        //double Dist2DBP=sqrt( (xvt-m_Xbeampipe)*(xvt-m_Xbeampipe) + (yvt-m_Ybeampipe)*(yvt-m_Ybeampipe) ); 
        //double Dist2DBL=sqrt( (xvt-m_XlayerB)*(xvt-m_XlayerB) + (yvt-m_YlayerB)*(yvt-m_YlayerB) ); 
        //double Dist2DL1=sqrt( (xvt-m_Xlayer1)*(xvt-m_Xlayer1) + (yvt-m_Ylayer1)*(yvt-m_Ylayer1) );
        double Dist2DL2=sqrt( (xvt-m_Xlayer2)*(xvt-m_Xlayer2) + (yvt-m_Ylayer2)*(yvt-m_Ylayer2) );
        double minDstMat=39.9;      
        minDstMat=TMath::Min(minDstMat,fabs(Dist2DL2-m_Rlayer3));  // 4-layer pixel detector
       
        TLorentzVector MomentumVtx = TotalMom(xAODwrk->listSelTracks);
        TrackE+=newvrt.vertexMom.E();
        vertexN+=1;
        
        double eRatio = MomentumVtx.E()/jet->p4().E(); 
        double signif3D;
        double Signif3D=vrtVrtDist(primVrt, newvrt.vertex, newvrt.vertexCov, signif3D);          
        
        if(newvrt.vertex.perp()>m_Rbeampipe && Signif3D<m_minSig3D)  continue; 
        if(Lxy<=m_minD0 )  continue;
        if(newvrt.vertex.perp()<m_minPerp) continue;
        
        //Make New Container        
        xAOD::Vertex *GNNvertex = new xAOD::Vertex;
        outVertexContainer->emplace_back(GNNvertex);       
        // Registering tracks comprising the vertex to xAOD::Vertex
        // loop over the tracks comprising the vertex
        for( const auto *trk : xAODwrk->listSelTracks ) {
          // Acquire link the track to the vertex
          ElementLink<xAOD::TrackParticleContainer> link_trk( *( dynamic_cast<const xAOD::TrackParticleContainer*>( trk->container() ) ), static_cast<long unsigned int>(trk->index()) );
          // Register the link to the vertex
          GNNvertex->addTrackAtVertex( link_trk, 1. );
        }
       
        //Add Vertex Info into Container 
        GNNvertex->setVertexType(xAOD::VxType::SecVtx);
        GNNvertex->setPosition(newvrt.vertex);
        GNNvertex->setFitQuality(newvrt.chi2, NDOF);
        decor_mass(*GNNvertex)            = newvrt.vertexMom.M();
        decor_pT(*GNNvertex)              = newvrt.vertexMom.Perp();
        decor_charge(*GNNvertex)          = newvrt.vertexCharge;
        decor_vPos(*GNNvertex)            = vPos;
        decor_Lxy(*GNNvertex)             = Lxy;
        decor_L3D(*GNNvertex)             = L3D;
        decor_significance3d(*GNNvertex)  = Signif3D; 
        decor_NGTinSvx(*GNNvertex)        = NGTatVtx;
        decor_deltaR(*GNNvertex)          = drJPVSV;
        decor_minDstMat(*GNNvertex)       = minDstMat;
        decor_efracsv(*GNNvertex)         = eRatio;
        
        if (newvrt.trkAtVrt.size()==2){
          decor_N2Tpair(*GNNvertex)=newvrt.trkAtVrt.size();
        }
        ElementLink< xAOD::VertexContainer> linkVertex;
        linkVertex.setElement(GNNvertex);
        linkVertex.setStorableObject(*outVertexContainer);
        jetWriteDecorHandleVertexLink(*jet).push_back(linkVertex);
             
      }//end of 2 Track requirement
    }
    if (m_fitHists==true){
      m_vertexN->Fill(vertexN, 1.);
      if(TrackE==0)continue;
      else{
      float eFracNew = TrackE/jet->p4().E();
      m_eFrac->Fill(eFracNew, 1.);
      }   
    } 
    delete xAODwrk;
  } // end loop over jets
  return StatusCode::SUCCESS;
} // end performVertexFit

StatusCode GNNVertexConstructorTool::finalize()
{
  return StatusCode::SUCCESS;
}

} // namespace Rec
