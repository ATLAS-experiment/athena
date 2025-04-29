/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
 

#include "EFTrackingSmearingAlg.h"
#include "xAODTracking/VertexContainer.h" 
#include "AthContainers/ConstAccessor.h"

#include "TH1.h"

#ifndef XAOD_ANALYSIS
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "TrkEventPrimitives/ParamDefs.h"
#else

namespace Amg
{
  // expression template to evaluate the required size of the compressed matrix at compile time
  template<int N>
  struct CalculateCompressedSize {
      static const int value = N + CalculateCompressedSize<N - 1>::value;
  }; 
  
  template<>
  struct CalculateCompressedSize<1> {
      static const int value = 1;
  };

  template<int N>
   inline void compress(const AmgSymMatrix(N)& covMatrix, std::vector<float>& vec ) {
       vec.reserve(CalculateCompressedSize<N>::value);
       for (unsigned int i = 0; i < N ; ++i)
         for (unsigned int j = 0; j <= i; ++j)
             vec.push_back(covMatrix(i,j));
     }
   inline void compress(const MatrixX& covMatrix, std::vector<float>& vec) {
       int rows = covMatrix.rows();
       for (int i = 0; i < rows; ++i) {
           for (int j = 0; j <= i; ++j) {
               vec.push_back(covMatrix(i, j));
           }
       }
   }
}

namespace Trk
{
  enum ParamDefs        {
    // Enums for LocalParameters - LocalPosition/
                             loc1=0, loc2=1,                     //!< generic first and second local coordinate
                             locX=0, locY=1 ,                    //!< local cartesian
                             locRPhi=0, locPhiR=0, locZ=1,       //!< local cylindrical
                             locR=0, locPhi=1,                   //!< local polar
                             iPhi=0, iEta=1,                     //!< (old readout) will be skipped
                             distPhi=0, distEta=1,               //!< readout for silicon
                             driftRadius=0,                      //!< trt, straws
    // Enums for const Amg::Vector3D & GlobalMomentum /
                             x=0,   y=1,  z=2,                   //!< global position (cartesian)
                             px=0, py=1, pz=2,                   //!< global momentum (cartesian)
    // Enums for Perigee //
                             d0=0, z0=1, phi0=2, theta=3, qOverP=4, //!< perigee
 /* Enums for TrackState on Surfaces
  The first two enums in the TrackParameters refer to the local Frame, i.e.
   - LocalCartesian for AtanArbitraryPlane
   - LocalCylindrical for AtaCylinder (includes line)
   - LocalPolar for AtaDisc
   The other three enums are standard \f$(\phi, \eta, \frac{q}{p_{T}})\f$
  */
                             phi=2,
    /**Enums for curvilinear frames*/
                             u=0,  v=1,        
             
   /** Extended perigee: mass*/              
             trkMass=5
             
                             };                            //!< parameters on surface

}
#endif


// from Tracking/TrkEventCnv/TrkEventTPCnv/TrkEventTPCnv/helpers/EigenHelpers.h
namespace EigenHelpers 
{
  template <class VECTOR, class COVARIANCE> 
   inline static void eigenMatrixToVector(VECTOR& vec, COVARIANCE& cov, const char* ) {                                                    
       Amg::compress(cov, vec);                                          
     }
}



EFTrackingSmearingAlg::EFTrackingSmearingAlg( const std::string& name, ISvcLocator* pSvcLocator ) 
: AthHistogramAlgorithm( name, pSvcLocator ){}


EFTrackingSmearingAlg::~EFTrackingSmearingAlg() {}


StatusCode EFTrackingSmearingAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");

  ATH_CHECK( m_inputTrackParticleKey.initialize() );
  ATH_CHECK( m_outputTrackParticleKey.initialize() );
  
  ATH_CHECK( m_inputTruthParticleKey.initialize() );
  ATH_CHECK( m_outputTruthParticleKey.initialize() );

 // Decoration keys
  m_d0DecoratorKey = m_outputTruthParticleKey.key() + ".d0";
  m_z0DecoratorKey = m_outputTruthParticleKey.key() + ".z0";
  m_ptDecoratorKey = m_outputTruthParticleKey.key() + ".pt";

  ATH_CHECK(m_d0DecoratorKey.initialize());
  ATH_CHECK(m_z0DecoratorKey.initialize());
  ATH_CHECK(m_ptDecoratorKey.initialize());

  ATH_MSG_INFO("########## EFTrackingSmearingAlg Configurations are ########## ");
  ATH_MSG_INFO("------- InputTrackParticleKey: "<<m_inputTrackParticleKey.key());
  ATH_MSG_INFO("------- OutputTrackParticleKey: "<<m_outputTrackParticleKey.key());
  ATH_MSG_INFO("------- InputTruthParticleKey: " <<m_inputTruthParticleKey.key());
  ATH_MSG_INFO("------- OutputTruthParticleKey: "<<m_outputTruthParticleKey.key());
  ATH_MSG_INFO("------- inputTracksPtCut [GeV]: "<<m_inputTracksPtCut);
  ATH_MSG_INFO("------- outputTracksPtCut [GeV]: "<<m_outputTracksPtCut);
  ATH_MSG_INFO("------- SmearingSigma: "<<m_SigmaScaleFactor);  
  ATH_MSG_INFO("------- trackEfficiency: "<<m_smearedTrackEfficiency);  
  ATH_MSG_INFO("------- parameterizedEfficiency: "<<m_parameterizedTrackEfficiency);
  ATH_MSG_INFO("------- parameterizedEfficiency LRT: "<<m_parameterizedTrackEfficiency_LRT);  
  ATH_MSG_INFO("------- parameterizedEfficiency LRT high d0 cut: "<<m_smearedTrackEfficiency_d0high_LRT );  
  ATH_MSG_INFO("------- parameterizedEfficiency LRT low d0 cut: "<<m_smearedTrackEfficiency_d0low_LRT );  
  ATH_MSG_INFO("------- UseResolutionPtCutOff: "<<m_UseResolutionPtCutOff);
  ATH_MSG_INFO("------- SetResolutionPtCutOff: "<<m_SetResolutionPtCutOff);
  ATH_MSG_INFO("------- EnableMonitoring:" <<m_enableMonitoring);
  ATH_MSG_INFO("------- SmearTruthParticle:"<< m_smearTruthParticle);
  
  ATH_MSG_INFO("------- IncludeDuplicatesAndFakes: "<<m_EnableFakes);
  ATH_MSG_INFO("------- RandomSeed: "   <<m_RandomSeed);
  ATH_MSG_INFO("------- UseCoinToss: "  <<m_UseCoinToss);
  ATH_MSG_INFO("------- FakeKillerEnable: "<<m_FakeKillerEnable);
  ATH_MSG_INFO("------- IncludeFakesInResolutionCalculation: "<<m_IncludeFakesInResolutionCalculation);
  ATH_MSG_INFO("########## EFTrackingSmearingAlg Configurations: That's it. ########## ");

  std::string smearerName;
  if (m_smearTruthParticle){
    ATH_MSG_INFO("Will output new truth container with name "<<m_outputTruthParticleKey.key());
    smearerName = m_outputTruthParticleKey.key()+"_smearer";
  }
  else { 
    ATH_MSG_INFO("Will output new track container with name "<<m_outputTrackParticleKey.key());
    smearerName = m_outputTrackParticleKey.key()+"_smearer";
  }

  // configure the Smearer
  m_mySmearer = std::make_unique<FakeTrackSmearer>(smearerName, m_RandomSeed, msgLvl (MSG::DEBUG));
  m_mySmearer->SetInputTracksPtCut(m_inputTracksPtCut);
  m_mySmearer->SetOutputTracksPtCut(m_outputTracksPtCut);
  m_mySmearer->SetTrackingEfficiency(m_smearedTrackEfficiency);
  m_mySmearer->SetParameterizedEfficiency(m_parameterizedTrackEfficiency);
  m_mySmearer->SetParameterizedEfficiency_LRT(m_parameterizedTrackEfficiency_LRT);
  m_mySmearer->SetParameterizedEfficiency_highd0_LRT(m_smearedTrackEfficiency_d0high_LRT);
  m_mySmearer->SetParameterizedEfficiency_lowd0_LRT(m_smearedTrackEfficiency_d0low_LRT);
  
  m_mySmearer->SetSigmaScaleFactor(m_SigmaScaleFactor.value());
  m_mySmearer->UseResolutionPtCutOff(m_UseResolutionPtCutOff.value());
  m_mySmearer->SetResolutionPtCutOff(m_SetResolutionPtCutOff.value());

  m_mySmearer->EnableFakes(m_EnableFakes.value());
  m_mySmearer->UseCoinToss(m_UseCoinToss.value());
  m_mySmearer->FakeKillerEnable(m_FakeKillerEnable.value());
  m_mySmearer->IncludeFakesInResolutionCalculation(m_IncludeFakesInResolutionCalculation.value());

  if (m_enableMonitoring) {
    // store the smearing functions
    TF1 *d0res_eta = m_mySmearer->d0res_eta;
    TF1 *z0res_eta = m_mySmearer->z0res_eta;
    TF1 *curvres_eta = m_mySmearer->curvres_eta;
    TF1 *d0res_pt  = m_mySmearer->d0res_pt;
    TF1 *z0res_pt  = m_mySmearer->z0res_pt;
    TF1 *curvres_pt  = m_mySmearer->curvres_pt;
    TF1 *effLRT_d0  = m_mySmearer->effLRT_d0;

    CHECK(book(new TH1F("d0res_function_vs_eta","#eta of track (p_{T}=10GeV);#eta",100, 0.0,4.0)));
    CHECK(book(new TH1F("z0res_function_vs_eta","#eta of track (p_{T}=10GeV);#eta",100, 0.0,4.0)));
    CHECK(book(new TH1F("curvres_function_vs_eta","#eta of track (p_{T}=10GeV);#eta",100, 0.0,4.0)));
    CHECK(book(new TH1F("d0res_function_vs_pt","p_{T} of track (#eta=1);p_{T} [GeV]",100, 1.0,200.0)));
    CHECK(book(new TH1F("z0res_function_vs_pt","p_{T} of track (#eta=1);p_{T} [GeV]",100, 1.0,200.0)));
    CHECK(book(new TH1F("curvres_function_vs_pt","p_{T} of track (#eta=1);p_{T} [GeV]",100, 1.0,200.0)));
    CHECK(book(new TH1F("effLRT_function_vs_d0","d_{0} of track;d_{0} [mm]",100, 0.001,600.0)));
    hist("d0res_function_vs_eta")->Add(d0res_eta);
    hist("z0res_function_vs_eta")->Add(z0res_eta);
    hist("curvres_function_vs_eta")->Add(curvres_eta);
    hist("d0res_function_vs_pt")->Add(d0res_pt);
    hist("z0res_function_vs_pt")->Add(z0res_pt);
    hist("curvres_function_vs_pt")->Add(curvres_pt);
    hist("effLRT_function_vs_d0")->Add(effLRT_d0);
    // book historgams
    CHECK(book_histograms());
  }
  return StatusCode::SUCCESS;
}


StatusCode EFTrackingSmearingAlg::smearTruthParticles(const EventContext& ctx) {
  
  SG::ReadHandle<xAOD::TruthParticleContainer> inputTruth_handle( m_inputTruthParticleKey, ctx );
  const xAOD::TruthParticleContainer* inputTruth = inputTruth_handle.cptr();
  if (not inputTruth) {
    ATH_MSG_FATAL("Unable to retrieve input truth particle");
   return StatusCode::FAILURE;
  }
  SG::WriteHandle<xAOD::TruthParticleContainer> outputTruth_handle( m_outputTruthParticleKey, ctx );
  ATH_CHECK( outputTruth_handle.record( std::make_unique<xAOD::TruthParticleContainer>(), std::make_unique<xAOD::TruthParticleAuxContainer>() ) );
  auto outputTruth = outputTruth_handle.ptr();

  // create decorators
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float > d0Decorator(m_d0DecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float > z0Decorator(m_z0DecoratorKey, ctx);
  SG::WriteDecorHandle<xAOD::TruthParticleContainer, float > ptDecorator(m_ptDecoratorKey, ctx);
   

  // clear the smearear
  m_mySmearer->Clear();

  static const SG::ConstAccessor<float> ptAcc("pt");
  static const SG::ConstAccessor<float> d0Acc("d0");
  static const SG::ConstAccessor<float> z0Acc("z0");
  static const SG::ConstAccessor<float> thetaAcc("theta");

  int n_input_tracks=0;
  int n_output_tracks=0;
  int n_output_broad_tracks=0;
  int n_output_narrow_tracks=0; 
  ATH_MSG_DEBUG ("Found "<<inputTruth->size()<< " input truth particles");
  for ( const auto* part : *inputTruth ) 
    {    
      double pt = part->pt();// MeV      
      float theta = thetaAcc(*part);
      float z0 = z0Acc(*part);
      float d0 = d0Acc(*part);
      float eta = part->eta();
      float phi = part->phi();
      if (part->isNeutral()) continue;
      if (pt <=0.) continue;

      ATH_MSG_DEBUG ("===> New Truth: "  
                      <<" curv=" << 1./part->pt()
                      <<" phi="  << part->phi()
                      <<" eta="  << part->eta()
                      <<" d0="   << d0Acc(*part)
                      <<" z0="   << z0Acc(*part)
                      <<" pT="   << part->pt()  
                      <<" PDGID=" << part->pdgId()
                      <<" status=" << part->status()                       
                      );        
      if (part->parent(0)) ATH_MSG_DEBUG (" parent status=" << part->parent(0)->pdgId());
      
      if (std::abs(pt)/1000. > m_inputTracksPtCut) //GeV cut
      	{
      	  n_input_tracks++;
          if (m_enableMonitoring) {
      	    hist("track_input_eta")->Fill(eta);
      	    hist("track_input_theta")->Fill(theta);
      	    hist("track_input_pt" )->Fill(pt/1000.);
      	    hist("track_input_phi")->Fill(phi);
      	    hist("track_input_z0" )->Fill(z0);
      	    hist("track_input_d0" )->Fill(d0);      	    
          }
          double qoverPt = part->charge()*1000./pt; //this must be in GeV
          m_mySmearer->AddTrack(d0,z0,qoverPt,eta,phi);  // smearing here
          n_output_tracks += m_mySmearer->GetNTracks();
      	  
          ATH_MSG_DEBUG ("Looping on output tracks #"<< m_mySmearer->GetNTracks());
          for (const auto& otrack : m_mySmearer->Tracks)
      	    {                                                         
                xAOD::TruthParticle * newtrk = new xAOD::TruthParticle(*part);
                outputTruth->push_back(newtrk); 
                *newtrk = *part;
                auto newpt = part->pt();
                if (m_SigmaScaleFactor !=0) {
                  // set the decorators                  
                  d0Decorator(*newtrk) = otrack.d0();
                  z0Decorator(*newtrk) = otrack.z0();
                  ptDecorator(*newtrk) = otrack.pt()*1000.; //MeV
                  //TrackParticle has already ::pt(), so the smeared value is in the decorator 
                  // and can be accessed by                
                  newpt = ptAcc(*newtrk);
                }
                if (newpt==0.) continue;
                ATH_MSG_DEBUG ("Smeared Truth: "
                      <<" curv=" << 1./newpt
                      <<" phi="  << newtrk->phi()
                      <<" eta="  << newtrk->eta()
                      <<" d0="   << d0Acc(*newtrk)
                      <<" z0="   << z0Acc(*newtrk)
                      <<" pT="   << newpt
                      <<" PDGID=" << newtrk->pdgId()
                      <<" status=" << newtrk->status()                      
                  );
                if (newtrk->parent(0)) ATH_MSG_DEBUG (" parent status=" << newtrk->parent(0)->pdgId());
                
                if (m_enableMonitoring) {
                  hist("track_output_eta")->Fill(otrack.eta());
                  hist("track_output_theta")->Fill(otrack.theta());
                  hist("track_output_pt" )->Fill(part->charge()*otrack.pt() );
                  hist("track_output_phi")->Fill(otrack.phi());
                  hist("track_output_z0" )->Fill(otrack.z0() );
                  hist("track_output_d0" )->Fill(otrack.d0() );      
        
                  hist("track_outputcoll_eta")->Fill(newtrk->eta());
                  hist("track_outputcoll_theta")->Fill(thetaAcc(*newtrk));
                  hist("track_outputcoll_pt" )->Fill(part->charge()* newpt/1000.); 
                  hist("track_outputcoll_phi")->Fill(newtrk->phi());
                  hist("track_outputcoll_z0" )->Fill(z0Acc(*newtrk));
                  hist("track_outputcoll_d0" )->Fill(d0Acc(*newtrk));
                
                  hist("track_delta_eta")->Fill(newtrk->eta() - part->eta());  
                  hist("track_delta_pt") ->Fill((newpt - part->pt())/1000.);  	      
                  hist("track_delta_crv")->Fill(newtrk->charge()*1000./newpt - ((part->charge()*1000./part->pt())));
                  hist("track_delta_phi")->Fill(newtrk->phi() - part->phi());
                  hist("track_delta_z0" )->Fill(z0Acc(*newtrk) - z0Acc(*part));
                  hist("track_delta_d0" )->Fill(d0Acc(*newtrk) - d0Acc(*part));
                }                         	                
      	    } // end of loop                      
	      }
      m_mySmearer->Clear(); // clear the smearer after each input track
    }

  ATH_MSG_DEBUG ("End of loop track #"<<n_input_tracks<<" ---> "<<" "<< n_output_tracks
              <<" "<<n_output_narrow_tracks<<" "<<n_output_broad_tracks);
  if (m_enableMonitoring) {
    hist("n_input_tracks")->Fill(n_input_tracks);
    hist("n_output_tracks")->Fill(n_output_tracks);
    hist("n_output_narrow_tracks")->Fill(n_output_narrow_tracks);
    hist("n_output_broad_tracks")->Fill(n_output_broad_tracks);
  }
  return StatusCode::SUCCESS;
}



StatusCode EFTrackingSmearingAlg::execute() {  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  auto ctx = getContext() ;
  if (m_smearTruthParticle)
    return smearTruthParticles(ctx);      
  
  SG::ReadHandle<xAOD::TrackParticleContainer> inputTracks_handle( m_inputTrackParticleKey, ctx );
  const xAOD::TrackParticleContainer* inputTracks = inputTracks_handle.cptr();
  if (not inputTracks) {
     ATH_MSG_FATAL("Unable to retrieve input ID tacks");
     return StatusCode::FAILURE;
  }

  SG::WriteHandle<xAOD::TrackParticleContainer> outputTracks_handle( m_outputTrackParticleKey, ctx );
  ATH_CHECK( outputTracks_handle.record( std::make_unique<xAOD::TrackParticleContainer>(), std::make_unique<xAOD::TrackParticleAuxContainer>() ) );
  auto outputTracks = outputTracks_handle.ptr();
  
  // clear the smearear
  m_mySmearer->Clear();

  //int trackno=0;
  int n_input_tracks=0;
  int n_output_tracks=0;
  int n_output_broad_tracks=0;
  int n_output_narrow_tracks=0;
  ATH_MSG_DEBUG ("Found "<<inputTracks->size()<< " input tracks");
  for ( const auto* trk : *inputTracks ) 
    {            
      // get Cov matrix of input track
      xAOD::ParametersCovMatrix_t trkcov = trk->definingParametersCovMatrix();  
      auto trkcovvec = trk->definingParametersCovMatrixVec();  
      double theta=trk->theta();
      double pt = std::sin(theta)/std::abs(trk->qOverP());// MeV
      
      ATH_MSG_DEBUG ("===> New Track: "
                      <<" curv=" << 1./trk->pt()
                      <<" phi="  << trk->phi0()
                      <<" eta="  << trk->eta()
                      <<" d0="   << trk->d0()
                      <<" z0="   << trk->z0()
                      <<" pT="   << trk->pt()
                      <<" cov_d0=" << trkcov(Trk::d0,Trk::d0)
                      <<" cov_z0=" << trkcov(Trk::z0,Trk::z0)
                      <<" sigma_d0="  << std::sqrt(std::abs(trkcov(Trk::d0,Trk::d0)))
                      <<" sigma_z0="  << std::sqrt(std::abs(trkcov(Trk::z0,Trk::z0))) );
      
      
                      
      if (std::abs(pt) > m_inputTracksPtCut)
      	{
      	  n_input_tracks++;
          if (m_enableMonitoring) {
      	    hist("track_input_eta")->Fill(trk->eta());
      	    hist("track_input_theta")->Fill(trk->theta());
      	    hist("track_input_pt" )->Fill(pt/1000.);
      	    hist("track_input_phi")->Fill(trk->phi0());
      	    hist("track_input_z0" )->Fill(trk->z0());
      	    hist("track_input_d0" )->Fill(trk->d0());
      	
      	    hist("track_input_sigma_theta") ->Fill(std::sqrt(trkcov(Trk::theta,Trk::theta)));
      	    hist("track_input_sigma_qOverP")->Fill(std::sqrt(trkcov(Trk::qOverP,Trk::qOverP)));
      	    hist("track_input_sigma_phi")   ->Fill(std::sqrt(trkcov(Trk::phi0,Trk::phi0)));
      	    hist("track_input_sigma_z0")    ->Fill(std::sqrt(trkcov(Trk::z0,Trk::z0)));
      	    hist("track_input_sigma_d0")    ->Fill(std::sqrt(trkcov(Trk::d0,Trk::d0)));
          }
      	  
          // get Cov matrix of input track          
          auto trkcovvec = trk->definingParametersCovMatrixVec();  
          double qoverPt = trk->charge()*1000./pt; //this must be in GeV
          m_mySmearer->AddTrack(trk->d0(),trk->z0(),qoverPt,trk->eta(),trk->phi0());
          n_output_tracks += m_mySmearer->GetNTracks();
      	  
          ATH_MSG_DEBUG ("Looping on output tracks #"<< m_mySmearer->GetNTracks());
          for (const auto& otrack : m_mySmearer->Tracks)
            {      	  
      	      xAOD::TrackParticle * newtrk = new xAOD::TrackParticle(*trk);
              outputTracks->push_back(newtrk);  
              *newtrk = *trk;

              double sintheta=std::sin(otrack.theta());
              trkcov = trk->definingParametersCovMatrix();  
              auto newtrkcov = trkcov;  
              if (m_SigmaScaleFactor !=0) {
                  // modify the cov matrix
                  ATH_MSG_DEBUG ("Setting parameters in covariance vector");                     	            	      
                  for (int ii=0;ii<5;ii++) for (int jj=0;jj<5;jj++) {
                    // correct the CM to be consistent with the smearing ofthe parameters
                    newtrkcov(ii,jj)=trkcov(ii,jj) * (std::pow(m_SigmaScaleFactor,2) + 1);
                  }
                  trkcovvec.clear();
                  EigenHelpers::eigenMatrixToVector(trkcovvec,newtrkcov,"");
                  newtrk->setDefiningParametersCovMatrixVec(trkcovvec);      
                  ATH_MSG_DEBUG ("Setting parameters covariance");
                  newtrk->setDefiningParametersCovMatrix(newtrkcov);      

                  /* old method stays for reference, since it's not yet validated
                  for (int ii=0;ii<5;ii++) for (int jj=0;jj<5;jj++) if (ii!=jj) trkcov(ii,jj)=0.0;
                  trkcov(Trk::d0    ,Trk::d0    ) = otrack.sigma_d0()   * otrack.sigma_d0()   ; 
                  trkcov(Trk::z0    ,Trk::z0    ) = otrack.sigma_z0()   * otrack.sigma_z0()   ; 
                  trkcov(Trk::theta ,Trk::theta ) = otrack.sigma_theta()* otrack.sigma_theta(); 
                  trkcov(Trk::phi0  ,Trk::phi0  ) = otrack.sigma_phi()  * otrack.sigma_phi()  ; 
                  trkcov(Trk::qOverP,Trk::qOverP) = otrack.sigma_curv()/1000. * otrack.sigma_curv()/1000. *sintheta*sintheta;               
                  
                  trkcovvec.clear();
                  EigenHelpers::eigenMatrixToVector(trkcovvec,trkcov,"");
                  newtrk->setDefiningParametersCovMatrixVec(trkcovvec);      
                  ATH_MSG_DEBUG ("Setting parameters covariance");
                  newtrk->setDefiningParametersCovMatrix(trkcov);                                               	      
                  */

                  //(float d0, float z0, float phi0, float theta, float qOverP)
                  newtrk->setDefiningParameters(
                        otrack.d0(),
                        otrack.z0(),
                        otrack.phi(),
                        otrack.theta(),
                        (otrack.curv()*sintheta/1000.) //oneOverp in MeV
                        );
              }
              xAOD::ParametersCovMatrix_t trkcov_out = newtrk->definingParametersCovMatrix();
              

              ATH_MSG_DEBUG ("Smeared Track: "
                      <<" curv=" << 1./newtrk->pt()
                      <<" phi="  << newtrk->phi()
                      <<" eta="  << newtrk->eta()
                      <<" d0="   << newtrk->d0()
                      <<" z0="   << newtrk->z0()
                      <<" pT="   << newtrk->pt()
                      <<" cov_d0=" << trkcov_out(Trk::d0,Trk::d0)
                      <<" cov_z0=" << trkcov_out(Trk::z0,Trk::z0)
                      <<" sigma_d0="  << std::sqrt(std::abs(trkcov_out(Trk::d0,Trk::d0)))
                      <<" sigma_z0="  << std::sqrt(std::abs(trkcov_out(Trk::z0,Trk::z0))) );
                       
              
      	      if (m_enableMonitoring) {
      	        hist("track_output_eta")->Fill(otrack.eta());
      	        hist("track_output_theta")->Fill(otrack.theta());
      	        hist("track_output_pt" )->Fill(trk->charge()*otrack.pt() );
      	        hist("track_output_phi")->Fill(otrack.phi());
      	        hist("track_output_z0" )->Fill(otrack.z0() );
      	        hist("track_output_d0" )->Fill(otrack.d0() );      
      
      	        hist("track_outputcoll_eta")->Fill(newtrk->eta());
      	        hist("track_outputcoll_theta")->Fill(newtrk->theta());
      	        hist("track_outputcoll_pt" )->Fill(trk->charge()*std::sin(newtrk->theta())/(1000.0*newtrk->qOverP()));
      	        hist("track_outputcoll_phi")->Fill(newtrk->phi0());
      	        hist("track_outputcoll_z0" )->Fill(newtrk->z0());
      	        hist("track_outputcoll_d0" )->Fill(newtrk->d0());
      	      
      	        hist("track_delta_eta")->Fill(newtrk->eta() - trk->eta()); 
                hist("track_delta_pt")->Fill((newtrk->pt() - trk->pt())/1000.);      	      
      	        hist("track_delta_crv")->Fill((1000.0*newtrk->qOverP()/std::sin(theta))-((1000.0*trk->qOverP())/std::sin(theta)));
      	        hist("track_delta_phi")->Fill(newtrk->phi() - trk->phi0());
      	        hist("track_delta_z0" )->Fill(newtrk->z0()  - trk->z0());
      	        hist("track_delta_d0" )->Fill(newtrk->d0()  - trk->d0());
              
      	        auto trkcov_original = trk->definingParametersCovMatrix(); 
      	        hist("track_delta_sigma_theta") ->Fill(std::sqrt(trkcov_out(Trk::theta,Trk::theta))   - std::sqrt(trkcov_original(Trk::theta,Trk::theta)));
      	        hist("track_delta_sigma_qOverP")->Fill(std::sqrt(trkcov_out(Trk::qOverP,Trk::qOverP)) - std::sqrt(trkcov_original(Trk::qOverP,Trk::qOverP)));
      	        hist("track_delta_sigma_phi")   ->Fill(std::sqrt(trkcov_out(Trk::phi0,Trk::phi0))     - std::sqrt(trkcov_original(Trk::phi0,Trk::phi0)));
      	        hist("track_delta_sigma_z0")    ->Fill(std::sqrt(trkcov_out(Trk::z0,Trk::z0))         - std::sqrt(trkcov_original(Trk::z0,Trk::z0)));
       	        hist("track_delta_sigma_d0")    ->Fill(std::sqrt(trkcov_out(Trk::d0,Trk::d0))         - std::sqrt(trkcov_original(Trk::d0,Trk::d0)));

                
      	        hist("track_outputcoll_sigma_theta") ->Fill(std::sqrt(trkcov(Trk::theta,Trk::theta)));
      	        hist("track_outputcoll_sigma_qOverP")->Fill(std::sqrt(trkcov(Trk::qOverP,Trk::qOverP)));
      	        hist("track_outputcoll_sigma_phi")   ->Fill(std::sqrt(trkcov(Trk::phi0,Trk::phi0)));
      	        hist("track_outputcoll_sigma_z0")    ->Fill(std::sqrt(trkcov(Trk::z0,Trk::z0)));
      	        hist("track_outputcoll_sigma_d0")    ->Fill(std::sqrt(trkcov(Trk::d0,Trk::d0)));
              
              }
              // do we need this hack?
      	      // hack!!!
      	      // Chi2: 1 -> "Broad Fake"
      	      //       0 -> "core tracks"
      	      //
      	      // NDF: index of parent track in input track collection 
      	      //newtrk->setFitQuality(otrack.FakeFlags(),trackno);
      
      	                  
      	    }
            
	      }
        m_mySmearer->Clear(); // clear teh smearer after each input track
        //trackno++;
      }

  ATH_MSG_DEBUG ("End of loop track #"<<n_input_tracks<<" ---> "<<" "<< n_output_tracks
              <<" "<<n_output_narrow_tracks<<" "<<n_output_broad_tracks);
  if (m_enableMonitoring) {
    hist("n_input_tracks")->Fill(n_input_tracks);
    hist("n_output_tracks")->Fill(n_output_tracks);
    hist("n_output_narrow_tracks")->Fill(n_output_narrow_tracks);
    hist("n_output_broad_tracks")->Fill(n_output_broad_tracks);
  }
  return StatusCode::SUCCESS;
}




StatusCode EFTrackingSmearingAlg::book_histograms()
{
  double b_eta[3]={50,-5,5};
  CHECK(book(new TH1F("track_input_eta","#eta of input tracks",b_eta[0],b_eta[1],b_eta[2])));
  CHECK(book(new TH1F("track_input_theta","#Theta of input tracks",50,0.0,4.0)));
  CHECK(book(new TH1F("track_input_pt" ,"p_T  of input tracks",50.0,-10.0,10.0)));
  CHECK(book(new TH1F("track_input_phi","#phi of input tracks",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_input_z0" ,"z_0  of input tracks",100,-50,50)));
  CHECK(book(new TH1F("track_input_d0" ,"d_0  of input tracks",50,-5,5)));

  CHECK(book(new TH1F("track_input_sigma_theta","#sigma_{#Theta} of input tracks"   ,50,0.0,0.001)));
  CHECK(book(new TH1F("track_input_sigma_qOverP"   ,"#sigma_{q/P}  of input tracks" ,50.0,0.0,0.0001)));
  CHECK(book(new TH1F("track_input_sigma_phi"  ,"#sigma_{#phi} of input tracks"     ,50,0.0,0.002)));
  CHECK(book(new TH1F("track_input_sigma_z0"   ,"#sigma_{z_0}  of input tracks"     ,100,0.0,5)));
  CHECK(book(new TH1F("track_input_sigma_d0"   ,"#sigma_{d_0}  of input tracks"     ,50,0.0,5)));
  
  int maxtracks=500;
  CHECK(book(new TH1F("n_input_tracks"       ,"Number of input tracks"            ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_tracks"      ,"Number of output tracks"           ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_narrow_tracks","Number of output tracks (narrow)" ,maxtracks,0,maxtracks)));
  CHECK(book(new TH1F("n_output_broad_tracks"  ,"Number of output tracks (broad)" ,maxtracks,0,maxtracks)));

  CHECK(book(new TH1F("track_output_eta","#eta of output tracks",50,-5,5)));
  CHECK(book(new TH1F("track_output_theta","#Theta of output tracks",50,0.0,4.0)));
  CHECK(book(new TH1F("track_output_pt" ,"p_T  of output tracks [GeV]",50,-10,10)));
  CHECK(book(new TH1F("track_output_phi","#phi of output tracks",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_output_z0" ,"z_0  of output tracks",100,-50,50)));
  CHECK(book(new TH1F("track_output_d0" ,"d_0  of output tracks",50,-5,5)));

  CHECK(book(new TH1F("track_outputcoll_eta","#eta of output tracks collection",50,-5,5)));
  CHECK(book(new TH1F("track_outputcoll_theta","#Theta of output tracks collection",50,0.0,4.0)));
  CHECK(book(new TH1F("track_outputcoll_pt" ,"p_T  of output tracks collection [GeV]",50,-10,10)));
  CHECK(book(new TH1F("track_outputcoll_phi","#phi of output tracks collection",50,-6.28,6.28)));
  CHECK(book(new TH1F("track_outputcoll_z0" ,"z_0  of output tracks collection",100,-50,50)));
  CHECK(book(new TH1F("track_outputcoll_d0" ,"d_0  of output tracks collection",50,-5,5)));

  CHECK(book(new TH1F("track_outputcoll_sigma_theta","#sigma_{#Theta} of output tracks collection"  ,50,0.0,0.001)));
  CHECK(book(new TH1F("track_outputcoll_sigma_qOverP"   ,"#sigma_{q/P}  of output tracks collection",50.0,0.0,0.0001)));
  CHECK(book(new TH1F("track_outputcoll_sigma_phi"  ,"#sigma_{#phi} of output tracks collection"    ,50,0.0,0.002)));
  CHECK(book(new TH1F("track_outputcoll_sigma_z0"   ,"#sigma_{z_0}  of output tracks collection"    ,100,0.0,5)));
  CHECK(book(new TH1F("track_outputcoll_sigma_d0"   ,"#sigma_{d_0}  of output tracks collection"    ,50,0.0,5)));

  CHECK(book(new TH1F("track_delta_sigma_theta", "#sigma_{#Theta} of output tracks collection",100,-0.001,0.001)));
  CHECK(book(new TH1F("track_delta_sigma_qOverP", "#sigma_{q/P}  of output tracks collection",100,-0.0001,0.0001)));
  CHECK(book(new TH1F("track_delta_sigma_phi", "#sigma_{#phi} of output tracks collection"   ,100,-0.002,0.002)));
  CHECK(book(new TH1F("track_delta_sigma_z0", "#sigma_{z_0}  of output tracks collection"    ,100,-1.,1.)));
  CHECK(book(new TH1F("track_delta_sigma_d0", "#sigma_{d_0}  of output tracks collection"    ,100,-0.5,0.5)));

  CHECK(book(new TH1F("track_delta_eta","tracks #Delta #eta",50,-1,1)));
  CHECK(book(new TH1F("track_delta_pt","tracks #Delta #pt [GeV]",50,-2.,2.)));
  CHECK(book(new TH1F("track_delta_crv" ,"tracks #Delta crv",50,-1,1)));
  CHECK(book(new TH1F("track_delta_phi","tracks #Delta #phi",50,-0.5,0.5)));
  CHECK(book(new TH1F("track_delta_z0" ,"tracks #Delta z_0 ",100,-10,10)));
  CHECK(book(new TH1F("track_delta_d0" ,"tracks #Delta d_0 ",50,-2,2)));
  return StatusCode::SUCCESS;
}



