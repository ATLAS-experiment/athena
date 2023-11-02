// Headers
#include "GNNVertexConstructor/GNNVertexConstructorTool.h"
// Headers to Read & Write Decorations
#include "AnalysisUtils/AnalysisMisc.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "iostream"
#include "iterator"
#include "map"
#include "vector"
#include  "TrkToolInterfaces/ITrackSummaryTool.h"
#include  "TMath.h"


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
      m_MultiVertex(false),
      m_MultiWithPrimary(false)
      {
  declareInterface<IGNNVertexConstructorInterface>(this);

  declareProperty("JetTrackLinks", m_trackLinksKey = "AntiKt4EMPFlowJetsAuxDyn.TrackLinks");
  declareProperty("JetVertexLinks",
                  m_vertexLinksKey = "AntiKt4EMPFlowJetsAuxDyn.vertex_indices");

  declareProperty("GNNTool", m_gnn_Tool, "GNN Tool");
  declareProperty("VertexFitterTool", m_vertexFitterTool, "Vertex fitting tool");
  declareProperty("ExistIBL",   m_existIBL, "Inform whether 3-layer or 4-layer detector is used "  );
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
  declareProperty("MultiVertex",        m_MultiVertex,       "Run Multiple Secondary Vertices in jet finder"  );
  declareProperty("MultiWithPrimary",   m_MultiWithPrimary,  "Find Multiple Secondary Vertices + primary vertex in jet. MultiVertex Finder only!"  );
  
  m_massPi  = 139.5702 ;
  
//  m_instanceName=name;

}

/* Destructor */
GNNVertexConstructorTool::~GNNVertexConstructorTool() {
  ATH_MSG_DEBUG("GNNVertexConstructorTool destructor called");
}

StatusCode GNNVertexConstructorTool::initialize() {

  ATH_MSG_DEBUG("GNNVertexConstructor Tool in initialize()");

  //bool m_existIBL=True;
  
  if(m_existIBL){ // 4-layer pixel detector
   if( m_Rbeampipe==0.)  m_Rbeampipe=24.0;    
   if( m_RlayerB  ==0.)  m_RlayerB  =34.0;
   if( m_Rlayer1  ==0.)  m_Rlayer1  =51.6;
   if( m_Rlayer2  ==0.)  m_Rlayer2  =90.0;
   m_Rlayer3  =122.5;
  } else {   // 3-layer pixel detector
   if( m_Rbeampipe==0.)  m_Rbeampipe=29.4;    
   if( m_RlayerB  ==0.)  m_RlayerB  =51.5;
   if( m_Rlayer1  ==0.)  m_Rlayer1  =90.0;
   if( m_Rlayer2  ==0.)  m_Rlayer2  =122.5;
  } 
  
  // Initialize keys
  ATH_CHECK(m_trackLinksKey.initialize());
  ATH_CHECK(m_vertexLinksKey.initialize());

  m_jetWriteDecorKeyVertexLink = m_jetCollection + ".GNNVerticesLink";
  ATH_CHECK( m_jetWriteDecorKeyVertexLink.initialize()); 

  // Retrieve tools
  ATH_CHECK(m_gnn_Tool.retrieve());
  ATH_CHECK(m_vertexFitterTool.retrieve());

  //Additional Info for Vertex Fit
  ATH_CHECK(m_beamSpotKey.initialize());
  ATH_CHECK(m_eventInfoKey.initialize());

  return StatusCode::SUCCESS;
}

//Use GNN to decorate the tracks
//May be removed in future
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
/*
std::vector<double> GNNVertexConstructorTool::estimVrtPos( int nTrk, std::deque<long int> &selTrk, std::map<long int, std::vector<double>> & vrt) const
  {
    std::vector<double> estimation(3,0.);
    int ntsel=selTrk.size();
    for( int i=0; i<ntsel-1; i++){
       for( int j=i+1; j<ntsel; j++){
          int k = selTrk[i]<selTrk[j] ? selTrk[i]*nTrk+selTrk[j] : selTrk[j]*nTrk+selTrk[i];
          estimation[0]+=vrt.at(k)[0];
          estimation[1]+=vrt[k][1];
          estimation[2]+=vrt[k][2];
    }  }
    estimation[0] /= ntsel*(ntsel-1)/2;
    estimation[1] /= ntsel*(ntsel-1)/2;
    estimation[2] /= ntsel*(ntsel-1)/2;
    return estimation;
  }*/


//  double GNNVertexConstructorTool::VrtVrtDist(const xAOD::Vertex & PrimVrt, const Amg::Vector3D & SecVrt, 
//                                          const std::vector<double> SecVrtErr, double& Signif)
//  const
//  {
//    double distx =  PrimVrt.x()- SecVrt.x();
//    double disty =  PrimVrt.y()- SecVrt.y();
//    double distz =  PrimVrt.z()- SecVrt.z();
//
//
//    AmgSymMatrix(3)  PrimCovMtx=PrimVrt.covariancePosition();  //Create
//    PrimCovMtx(0,0) += SecVrtErr[0];
//    PrimCovMtx(0,1) += SecVrtErr[1];
//    PrimCovMtx(1,0) += SecVrtErr[1];
//    PrimCovMtx(1,1) += SecVrtErr[2];
//    PrimCovMtx(0,2) += SecVrtErr[3];
//    PrimCovMtx(2,0) += SecVrtErr[3];
//    PrimCovMtx(1,2) += SecVrtErr[4];
//    PrimCovMtx(2,1) += SecVrtErr[4];
//    PrimCovMtx(2,2) += SecVrtErr[5];
//
//    AmgSymMatrix(3)  WgtMtx = PrimCovMtx.inverse();
//
//    Signif = distx*WgtMtx(0,0)*distx
//            +disty*WgtMtx(1,1)*disty
//            +distz*WgtMtx(2,2)*distz
//         +2.*distx*WgtMtx(0,1)*disty
//         +2.*distx*WgtMtx(0,2)*distz
//         +2.*disty*WgtMtx(1,2)*distz;
//    Signif=sqrt(Signif);
//    if( Signif!=Signif ) Signif = 0.;
//    return sqrt(distx*distx+disty*disty+distz*distz);
//  }

//Perform Vertex fit using the jet decorations of the GNN
StatusCode GNNVertexConstructorTool::performVertexFit(const xAOD::JetContainer *inJetContainer,
                            xAOD::VertexContainer *outVertexContainer,
                            const xAOD::Vertex & primVrt,
                            const EventContext &ctx) const {

  using TLC = std::vector<ElementLink<DataVector<xAOD::TrackParticle_v1>>>;
  using TL  = ElementLink<DataVector<xAOD::TrackParticle_v1>>;

  //Read Decor Handle for Track links and Vertex links
  SG::ReadDecorHandle<xAOD::JetContainer, TLC> trackLinksHandle(m_trackLinksKey, ctx);
  SG::ReadDecorHandle<xAOD::JetContainer, std::vector<char, std::allocator<char>>>
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
  
  // Create a map of track links and track vertexing values (Using mutlimap)
  std::multimap<int, TL> vertexMap;

  // Loop over the jets
  for (const auto &jet : *inJetContainer) {
    
    //Ensure map is empty from previous iterations
    vertexMap.clear();

    //Retrieve the Vertex and Track Collections
    auto vertexCollection = vertexLinksHandle(*jet);
    auto trackCollection = trackLinksHandle(*jet);

    //Fill the map
    int i = 0;
    for (auto v : vertexCollection) {
      vertexMap.insert(std::pair<int, TL>(v, (trackCollection[i])));
      i++;
    }
    
    std::multimap<int, TL>::iterator itr;

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

//    xAODwrk->listSelTracks.clear();

    // returns next value after last key - easier for "for loop"
    auto lastKey = (vertexMap.end())->first;
    
    for (int k = 0; k < lastKey; ++k) {
      if (vertexMap.count(k) >= 2) {
        // Need at least 2 tracks to perform a fit
        auto elements = vertexMap.equal_range(k);
              
        int NTRKS =vertexMap.count(k);
        std::vector<double> InpMass(NTRKS,m_massPi);
        m_vertexFitterTool->setMassInputParticles( InpMass, *state);

        xAODwrk->listSelTracks.clear();
//        //newvrt.selTrk.clear();
              
        //Retrieve the tracks and push to working xAOD
        for (auto i = elements.first; i != elements.second; ++i) {
          xAODwrk->listSelTracks.push_back(*(i->second));
          //newvrt.selTrk.push_back((i->first));
          }
        
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
        if (newvrt.chi2<20){
        ATH_MSG_DEBUG("Found IniVertex=" << newvrt.vertex[0] << ", " << newvrt.vertex[1] << ", "
                                         << newvrt.vertex[2] << " trks " << newvrt.trkAtVrt.size());

        Amg::Vector3D vDir = newvrt.vertex - primVrt.position();  //Vertex Dirction in relation to Primary
        
        Amg::Vector3D jetVrtDir(jet->p4().Px()*vDir[0],jet->p4().Py()*vDir[1],jet->p4().Pz()*vDir[2]);
        
        double vPos = (vDir.x() * newvrt.vertexMom.Px() + vDir.y() * newvrt.vertexMom.Py() +
                       vDir.z() * newvrt.vertexMom.Pz()) /
                      newvrt.vertexMom.Rho();
      
        double L3D =sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]+vDir[2]*vDir[2]);
        ATH_MSG_DEBUG("L3D  " << L3D);
        
        double drJPVSV = Amg::deltaR(jetVrtDir,vDir); //DeltaR

        int NGTatVtx=newvrt.trkAtVrt.size();
       
        double xvt=newvrt.vertex[0]; double yvt=newvrt.vertex[1];
        double Dist2DBP=sqrt( (xvt-m_Xbeampipe)*(xvt-m_Xbeampipe) + (yvt-m_Ybeampipe)*(yvt-m_Ybeampipe) ); 
        double Dist2DBL=sqrt( (xvt-m_XlayerB)*(xvt-m_XlayerB) + (yvt-m_YlayerB)*(yvt-m_YlayerB) ); 
        double Dist2DL1=sqrt( (xvt-m_Xlayer1)*(xvt-m_Xlayer1) + (yvt-m_Ylayer1)*(yvt-m_Ylayer1) );
        double Dist2DL2=sqrt( (xvt-m_Xlayer2)*(xvt-m_Xlayer2) + (yvt-m_Ylayer2)*(yvt-m_Ylayer2) );
        double minDstMat=39.9;      
        minDstMat=TMath::Min(minDstMat,fabs(Dist2DBL-m_RlayerB));
        minDstMat=TMath::Min(minDstMat,fabs(Dist2DL1-m_Rlayer1));
        minDstMat=TMath::Min(minDstMat,fabs(Dist2DL2-m_Rlayer2));
        minDstMat=TMath::Min(minDstMat,fabs(Dist2DBP-m_Rbeampipe));
        if(m_existIBL) minDstMat=TMath::Min(minDstMat,fabs(Dist2DL2-m_Rlayer3));  // 4-layer pixel detector
       
//        ATH_MSG_DEBUG("min dst Material   " << minDstMat);
        
        TLorentzVector MomentumJet = TotalMom(xAODwrk->listSelTracks);
        
        double eRatio = newvrt.vertexMom.E()/jet->p4().E(); 
        ATH_MSG_DEBUG("Test E ration MomJet " << eRatio);
        
        double Signif3D=L3D/newvrt.chi2;
        
       if(newvrt.vertex.perp()>m_Rbeampipe && Signif3D<20.)  continue; 
               
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
        GNNvertex->setFitQuality(newvrt.chi2, 1);

        decor_mass(*GNNvertex)            = newvrt.vertexMom.M();
        decor_pT(*GNNvertex)              = newvrt.vertexMom.Perp();
        decor_charge(*GNNvertex)          = newvrt.vertexCharge;
        decor_vPos(*GNNvertex)            = vPos;
        decor_Lxy(*GNNvertex)             = sqrt(vDir[0]*vDir[0]+vDir[1]*vDir[1]);
        decor_L3D(*GNNvertex)             = L3D;
        decor_significance3d(*GNNvertex)  = L3D/newvrt.chi2;
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
      
      }//end of Chi2 cut
      }//end of 2 Track requirement
    }
    delete xAODwrk;
    //delete WrkVrt;
  } // end loop over jets
  return StatusCode::SUCCESS;
} // end performVertexFit

StatusCode GNNVertexConstructorTool::finalize()
{
  return StatusCode::SUCCESS;
}

} // namespace Rec
