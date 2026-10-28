///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// PhysValMET.cxx 
// Implementation file for class PhysValMET
// Contact: atlas-cp-jetetmiss-jsv-conveners@cern.ch
///////////////////////////////////////////////////////////////////

// PhysVal includes
#include "PhysValMET.h"
// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "xAODMissingET/versions/MissingETBase.h" 
#include "xAODMissingET/MissingET.h" 
#include "xAODMissingET/MissingETContainer.h" 
#include "xAODMissingET/MissingETAuxContainer.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAssociationHelper.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODBase/IParticle.h"


#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "PATCore/AcceptData.h"
#include "METUtilities/METHelpers.h"
#include "AthContainers/Decorator.h"

// Root includes
#include "TH1.h"


#include "FourMomUtils/P4Helpers.h"

// STL includes
#include <cmath>
#include <map>
#include <vector>

namespace {
  const std::map<std::string, unsigned int> map_index = {
    {"RefEle", 0},
    {"RefGamma", 1},
    {"RefTau", 2},
    {"RefMuons", 3},
    {"RefJet", 4},
    {"SoftClus", 5},
    {"PVSoftTrk", 6},
    {"FinalTrk", 7},
    {"FinalClus", 8}
  };

  const std::size_t mapSize = map_index.size();

  std::pair<xAOD::IParticle::FourMom_t, double>
  calculateParticleSums(const auto & particles){
    xAOD::IParticle::FourMom_t fourMom{};
    double sumPt = 0.0;
    for (const auto* p : particles) {
      fourMom += p->p4();
      sumPt += p->pt();
    }

    return {fourMom, sumPt};
  }

}


using namespace xAOD;

//Setup namespace
namespace MissingEtDQA {

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 

  // Constructors
  ////////////////

  PhysValMET::PhysValMET( const std::string& type, 
                          const std::string& name, 
                          const IInterface* parent) : 
    ManagedMonitorToolBase( type, name, parent ),
    m_metmaker(nullptr)
  {
    declareProperty( "InputElectrons", m_eleColl   = "Electrons"         );
    declareProperty( "InputPhotons",   m_gammaColl = "Photons"           );
    declareProperty( "InputTaus",      m_tauColl   = "TauJets"           );
    declareProperty( "InputMuons",     m_muonColl  = "Muons"             );
    declareProperty( "DoTruth",        m_doTruth = false                 );
    declareProperty( "InputIsDAOD",    m_inputIsDAOD = false             );
    declareProperty( "DoMETRefPlots",  m_doMETRefPlots = false           );
    declareProperty( "METMapName",     m_mapname   = "METAssoc"          );
    declareProperty( "METCoreName",    m_corename  = "MET_Core"          );
  }
  
  // Destructor
  ///////////////
  PhysValMET::~PhysValMET()
  {
  }
   
  // Athena algtool's Hooks
  ////////////////////////////

  StatusCode PhysValMET::initialize()
  {
    ATH_MSG_DEBUG ("Initializing " << name() << "...");    
    ATH_CHECK(ManagedMonitorToolBase::initialize());
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_gammaColl.initialize());
    ATH_CHECK(m_tauColl.initialize());
    ATH_CHECK(m_muonColl.initialize());
    ATH_CHECK(m_eleColl.initialize());
    ATH_CHECK(m_jetKeys.initialize());
    ATH_CHECK(m_metCaloKey.initialize());
    ATH_CHECK(m_metCaloAuxKey.initialize());

    ATH_CHECK(m_coreMetKeys.initialize());

    ATH_MSG_DEBUG("Retrieving tools...");

    //retrieve tools
    ATH_CHECK( m_metmakerTopo.retrieve() ); 
    ATH_CHECK( m_metmakerPFlow.retrieve() );
    ATH_CHECK( m_muonSelTool.retrieve() );
    ATH_CHECK( m_elecSelLHTool.retrieve() );
    ATH_CHECK( m_photonSelIsEMTool.retrieve() );
    ATH_CHECK( m_tauSelTool.retrieve() );
   
    return StatusCode::SUCCESS;
  }
 
 
  //Book histograms
  StatusCode PhysValMET::bookHistograms()
  { 

    ATH_MSG_DEBUG ("Booking hists " << name() << "...");
      
    //define hist info
    int nbinp = 100;
    int nbinpxy = 100;
    int nbinphi = 63;
    int nbinE = 100;
    double suptmi = 500.;
    double suptmixy = 250.;
    double binphi = 3.15;
    double lowET = 0.;
    double suET = 2500.;


    // Physics validation plots are level 10
    // this is the level of detail used in the monitoring
    if (m_detailLevel >= 10) {

      //loop through jet types
      for (const auto& jet_type : m_jet_types){
        //define variables
        std::string name_met;
        std::string name_sub;
        std::string dir;

        //Create and Register histograms for Rebuilt
        std::vector <std::string> met_type = {"MET_Rebuilt_"};
        ATH_MSG_DEBUG("****STARTING****");

        //loop through EMPFlow and EMTopo
        for (const auto& type : met_type){
          //define variables for histograms 
          name_met = type + jet_type;
          m_dir_met.clear();
          std::vector<TH1D*> v_MET;
          std::vector<TH1D*> v_MET_x;
          std::vector<TH1D*> v_MET_y;
          std::vector<TH1D*> v_MET_phi;
          std::vector<TH1D*> v_MET_sum;
          std::vector<TH1D*> v_MET_Cumu;
          std::vector<TH1D*> v_MET_Resolution;
          std::vector<TH1D*> v_MET_Significance;
          std::vector<TH1D*> v_MET_dPhi;
          std::vector<TH2D*> v_MET_CorrFinalTrk;
          std::vector<TH2D*> v_MET_CorrFinalClus;
          std::vector<TH1D*> v_MET_Diff;
          std::vector<TH1D*> v_MET_Diff_x;
          std::vector<TH1D*> v_MET_Diff_y;
          std::vector<TH1D*> v_MET_Diff_phi;
          std::vector<TH1D*> v_MET_Diff_sum;
          std::vector<TH1D*> v_MET_Kinematic_pt;
          std::vector<TH1D*> v_MET_Kinematic_eta;
          std::vector<TH1D*> v_MET_Kinematic_phi;
          std::vector<TH1D*> v_MET_multi;

          //Create histograms

          //Setup Term histograms
          //these produce x, y, phi, and sum histos for FinalClus, FinalTrk, RefMuons, PVsoftTrk, RefEle, RefGamma, RefJet, RefTau, and SoftClus 
          for(const auto& term : m_terms){
            const std::string hist_title = std::format("{} {}", name_met, m_term_names[term]);
            v_MET.push_back( new  TH1D((name_met + "_" + term).c_str(), (hist_title + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
            v_MET_x.push_back( new  TH1D((name_met + "_" + term +"_x").c_str(), (hist_title + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
            v_MET_y.push_back( new  TH1D((name_met + "_" + term + "_y").c_str(), (hist_title + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
            v_MET_phi.push_back( new  TH1D((name_met + "_" + term + "_phi").c_str(), (hist_title + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
            v_MET_sum.push_back( new  TH1D((name_met + "_" + term + "_sum").c_str(), (hist_title + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET) );
            m_dir_met.push_back("MET/" + name_met + "/Terms/" + term + "/");
          }

          std::size_t n = v_MET.size();
          m_MET[name_met] = std::move(v_MET);
          m_MET_x[name_met] = std::move(v_MET_x);
          m_MET_y[name_met] = std::move(v_MET_y);
          m_MET_phi[name_met] = std::move(v_MET_phi);
          m_MET_sum[name_met] = std::move(v_MET_sum);

          //Register Term histograms
          for(std::size_t i = 0; i < n; ++i) {
            ATH_CHECK(regHist(m_MET[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_sum[name_met].at(i),m_dir_met[i],all));
          }
 
          //Setup cumulative hists
          //these produce cumulative histos for FinalClus and FinalTrk
          name_sub = name_met + "/Cumulative";
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalClus").c_str(), (name_met + " CST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalTrk").c_str(), (name_met + " TST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
        
          m_MET_Cumu[name_met] = std::move(v_MET_Cumu);
        
          //Register cumulative hists
          for(std::vector<TH1D*>::size_type i = 0; i < m_MET_Cumu[name_met].size(); ++i) {
            ATH_CHECK(regHist(m_MET_Cumu[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          //Setup Residual histograms        
          //these produce residual histos for FinalClus x and y & FinalTrk x and y
          name_sub = name_met + "/Residuals";
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_x").c_str(), ("x-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_y").c_str(), ("y-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_x").c_str(), ("x-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_y").c_str(), ("y-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
        
          m_MET_Resolution[name_met] = std::move(v_MET_Resolution);
        
          //register Residual histograms
          for(std::vector<TH1D*>::size_type i = 0; i < m_MET_Resolution[name_met].size(); ++i){
            ATH_CHECK(regHist(m_MET_Resolution[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //Setup Significance hists        
          //these produce significance histos for FinalClus and FinalTrk
          name_sub = name_met + "/Significance";
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalClus").c_str(), ("MET / sqrt(sumet) for " + name_met + " CST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalTrk").c_str(), ("MET / sqrt(sumet) for " + name_met + " TST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
        
          m_MET_Significance[name_met] = std::move(v_MET_Significance);
        
          //Register Significance hists
          for(std::vector<TH1D*>::size_type i = 0; i < m_MET_Significance[name_met].size(); ++i) {
            ATH_CHECK(regHist(m_MET_Significance[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //Setup dPhi hists        
          //these produce dPhi histos for leading & sub leading jet and leading lepton for FinalClus and FinalTrk
          name_sub = name_met + "/dPhi";
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalClus").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " CST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalClus").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " CST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalClus").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " CST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " TST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " TST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalTrk").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " TST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
        
          m_MET_dPhi[name_met] = std::move(v_MET_dPhi);
        
          //Register dPhi hists
          for(std::vector<TH1D*>::size_type i = 0; i < m_MET_dPhi[name_met].size(); ++i) {
            ATH_CHECK(regHist(m_MET_dPhi[name_met].at(i),"MET/" + name_sub + "/",all));
          }
         
          //Setup Correlation hists
          //these produce correlation histos
          //for FinalClus there are correlation plots for RefMuons, RefEle, RefGamma, RefJet, RefTau, & SoftClus
          //for FinalTrk there are correlation plots for RefMuons, RefEle, RefGamma, RefJet, RefTau, & PVSoftTrk
          name_sub = name_met + "/Correlations";

          v_MET_CorrFinalClus.reserve(m_terms.size()-3);

          for(const auto& it : m_terms) {
            if(it == "PVSoftTrk"){continue;}
            if(it == "FinalTrk"){continue;}
            if(it == "FinalClus"){continue;}
            v_MET_CorrFinalClus.push_back( new  TH2D((name_met + "_" + it + "_FinalClus").c_str(), (name_met + " " + m_term_names[it] + " vs. CST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,CST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }
          v_MET_CorrFinalTrk.reserve(m_terms.size()-3);

          for(const auto& it : m_terms) {
            if(it == "SoftClus"){continue;}
            if(it == "FinalTrk"){continue;}
            if(it == "FinalClus"){continue;} 
            v_MET_CorrFinalTrk.push_back( new  TH2D((name_met + "_" + it + "_FinalTrk").c_str(), (name_met + " " + m_term_names[it] + " vs. TST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,TST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }

          m_MET_CorrFinalClus[name_met] = std::move(v_MET_CorrFinalClus);
          m_MET_CorrFinalTrk[name_met] = std::move(v_MET_CorrFinalTrk);

          //Register Correlation hists
          for(std::vector<TH2D*>::size_type i = 0; i < m_MET_CorrFinalTrk[name_met].size(); ++i) {
            ATH_CHECK(regHist(m_MET_CorrFinalTrk[name_met].at(i),"MET/" + name_sub + "/",all));
          }
          for(std::vector<TH2D*>::size_type i = 0; i < m_MET_CorrFinalClus[name_met].size(); ++i) {
            ATH_CHECK(regHist(m_MET_CorrFinalClus[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          m_dir_met.clear();
         
          //Setup Diff histograms
          //these produce difference histograms for each particle: RefMuons, RefEle, RefGamma, RefJet, and RefTau
          //For each particle histograms of pT, px, py, delta phi, and sum
          for(const auto& it : m_terms) {
            if(it == "SoftClus"){continue;}
            if(it == "PVSoftTrk"){continue;}
            if(it == "FinalTrk"){continue;}
            if(it == "FinalClus"){continue;} 
            const std::string hist_title = m_term_names[it] + " in " + name_met;
            v_MET_Diff.push_back( new  TH1D((name_met + "_Diff_" + it).c_str(), ("MET_Diff " + hist_title +"; #Sigma p_{T}^{Val} - #Sigma p_{T}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200));
            v_MET_Diff_x.push_back( new  TH1D((name_met + "_Diff_" + it +"_x").c_str(), ("MET_Diff x " + hist_title +"; #Sigma p_{x}^{Val} - #Sigma p_{x}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200) );
            v_MET_Diff_y.push_back( new  TH1D((name_met + "_Diff_" + it +"_y").c_str(), ("MET_Diff y " + hist_title +"; #Sigma p_{y}^{Val} - #Sigma p_{y}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200) );
            v_MET_Diff_phi.push_back( new  TH1D((name_met + "_Diff_" + it +"_phi").c_str(), ("MET_Diff phi " + hist_title +"; #Delta#Phi(#Sigma p_{T}^{Val},#Sigma p_{T}^{No Val}); Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
            v_MET_Diff_sum.push_back( new  TH1D((name_met + "_Diff_" + it +"_sum").c_str(), ("MET_Diff sumet " + hist_title +"; E_{T}^{sum Val} - #Sigma |p_{T}^{No Val}| [GeV]; Entries / 6 GeV").c_str(), nbinpxy, -300, 300) );
            m_dir_met.push_back("MET/" + name_met + "/Differences/" + it + "/");
          }
        
          n = v_MET_Diff.size();
          m_MET_Diff[name_met] = std::move(v_MET_Diff);
          m_MET_Diff_x[name_met] = std::move(v_MET_Diff_x);
          m_MET_Diff_y[name_met] = std::move(v_MET_Diff_y);
          m_MET_Diff_phi[name_met] = std::move(v_MET_Diff_phi);
          m_MET_Diff_sum[name_met] = std::move(v_MET_Diff_sum);

        
          //Register Diff histograms
          for(std::size_t i = 0; i < n; ++i) {
            ATH_CHECK(regHist(m_MET_Diff[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_sum[name_met].at(i),m_dir_met[i],all));
          }
          m_dir_met.clear();

          //Setup Kinematic histos
          //these produce kinematic histos for each particle: RefMuons, Refele, RefGamma, RefJet, and RefTau
          //For each particle histograms of particle pt, eta, and phi. As well a histogram for multiplicity of each particle for events are stored
          for(const auto& it : m_terms){
            if(it == "SoftClus"){continue;}
            if(it == "PVSoftTrk"){continue;}
            if(it == "FinalTrk"){continue;}
            if(it == "FinalClus"){continue;}
            const std::string hist_title = m_term_names[it] + " in " + name_met;
            v_MET_Kinematic_pt.push_back( new  TH1D((name_met + "_Kinematic_" + it+"_pt").c_str(), ("MET_Kinematic_pt " + hist_title +"; p_{T} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, 0, 300));
            v_MET_Kinematic_eta.push_back( new  TH1D((name_met + "_Kinematic_" + it +"_eta").c_str(), ("MET_Kinematic_eta " + hist_title +"; #eta ; Entries / 0.1").c_str(), 100, -5, 5) );
            v_MET_Kinematic_phi.push_back( new  TH1D((name_met + "_Kinematic_" + it +"_phi").c_str(), ("MET_Kinematic_phi " + hist_title +"; #Phi ; Entries / 0.1").c_str(), nbinphi, -binphi, binphi) );
            v_MET_multi.push_back( new  TH1D((name_met + "_Multi_" + it).c_str(), ("MET_multi " + hist_title +"; Multiplicity; Entries").c_str(), 20,-0.5,20.5) );
            m_dir_met.push_back("MET/" + name_met + "/Kinematics/" + it + "/");                                      
          }

          m_MET_Kinematic_pt[name_met] = std::move(v_MET_Kinematic_pt);
          m_MET_Kinematic_eta[name_met] = std::move(v_MET_Kinematic_eta);
          m_MET_Kinematic_phi[name_met] = std::move(v_MET_Kinematic_phi);
          m_MET_multi[name_met] = std::move(v_MET_multi);

          //Register Kin histograms
          for(std::vector<TH1D*>::size_type i = 0; i < m_MET_Kinematic_pt[name_met].size(); ++i){          
            ATH_CHECK(regHist(m_MET_Kinematic_pt[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Kinematic_eta[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Kinematic_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_multi[name_met].at(i),m_dir_met[i],all));
          }
        // End of loop
        }
      }  
      
      //Now MET_Calo
  
      std::string name_met = "MET_Calo";
      std::string dir = "MET/" + name_met + "/";

      //Create and register Calo hists
      //these produce calo histos for for ET, Ex, Ey, phi, and sum
      ATH_CHECK(regHist(m_MET_Calo = new  TH1D("Calo", (name_met + " " + m_term_names["Calo"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_x = new  TH1D("Calo_x", (name_met + " " + m_term_names["Calo"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_y = new  TH1D("Calo_y", (name_met + " " + m_term_names["Calo"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_phi = new  TH1D("Calo_phi", (name_met + " " + m_term_names["Calo"] + " phi;  #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_sum = new  TH1D("Calo_sum", (name_met + " " + m_term_names["Calo"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), dir, all));
    }
  
    return StatusCode::SUCCESS;      
  }

  //Fill Histograms

  StatusCode PhysValMET::fillHistograms(const EventContext& ctx){
    //dont fill unbooked histograms!
    if (m_detailLevel < 10) {
      return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG ("Filling hists " << name() << "...");

    //If we're running over new AODs without MET containers, don't do anything!
    if(!m_inputIsDAOD && !m_doMETRefPlots)
      return StatusCode::SUCCESS;

    SG::ReadHandle<xAOD::EventInfo> eventInfoHandle(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfoHandle.isValid());
    const xAOD::EventInfo* eventInfo = eventInfoHandle.cptr();

    float weight = eventInfo->beamSpotWeight();

    //Retrieve MET Truth
    const xAOD::MissingETContainer* met_Truth = nullptr;
    if(m_doTruth) {
      ATH_CHECK( evtStore()->retrieve(met_Truth,"MET_Truth") );
      if (!met_Truth) {
        ATH_MSG_ERROR ( "Failed to retrieve MET_Truth. Exiting." );
        return StatusCode::FAILURE;
      }
    }

    ATH_MSG_DEBUG("Physics objects");

    //Set up Physics Objects
    //We will be grabbing all of the particles (electron, muon, gamma, tau, and jet) and then applying selections to them
    //To apply OR and JVT the particles will be fed into METMaker. Then OR will be applied to Tau and jets and JVT will be applied to jets
    //These particles will then be recieved from METMaker and used to fill the histograms

    //Muons 
    const xAOD::MuonContainer* muons = SG::get(m_muonColl);
    ConstDataVector<MuonContainer> basicSelectionMuons(SG::VIEW_ELEMENTS);
    for(const auto mu : *muons) {
      if(Accept(mu)) {
        basicSelectionMuons.push_back(mu);
      }
    }

    //Electrons
    const xAOD::ElectronContainer* electrons = SG::get(m_eleColl);
    ConstDataVector<ElectronContainer> basicSelectionElectrons(SG::VIEW_ELEMENTS);
    for(const auto el : *electrons) {
     if(Accept(el)) {
       basicSelectionElectrons.push_back(el);
     }
    }

    //Photons
    const xAOD::PhotonContainer* photons = SG::get(m_gammaColl);
    ConstDataVector<PhotonContainer> basicSelectionPhotons(SG::VIEW_ELEMENTS);
    for(const auto ph : *photons) {
      if(Accept(ph)) {
        basicSelectionPhotons.push_back(ph);
      }
    }

    //Tau
    const TauJetContainer* taus = SG::get(m_tauColl);
    ConstDataVector<TauJetContainer> basicSelectionTaus(SG::VIEW_ELEMENTS);
    for(const auto tau : *taus) {
      if(Accept(tau)) {
        basicSelectionTaus.push_back(tau);
      }
    }

    //Jets
    for (std::size_t i = 0; i < m_jet_types.size(); ++i) {
      const auto& jet_type = m_jet_types.at(i);
      const auto& jetKey = m_jetKeys.at(i);
      const xAOD::JetContainer* jets = SG::get(jetKey);

      std::string met_name = "MET_Rebuilt_"+jet_type;
      //Prepare Rebuilding MET
      ATH_MSG_DEBUG( "  Rebuilding MET_" << jet_type );

      auto met = std::make_unique<xAOD::MissingETContainer>();
      auto met_Aux = std::make_unique<xAOD::MissingETAuxContainer>();

      met->setStore(met_Aux.get()); //MET Reb container for METMaker

      //define map and core name for METMaker
      m_mapname = "METAssoc_"+jet_type;
      m_corename = "MET_Core_"+jet_type;
      const MissingETAssociationMap* metMap = nullptr;
      //check is can retrieve Map and Container
      if( evtStore()->retrieve(metMap, m_mapname).isFailure() ) {
        ATH_MSG_ERROR("Unable to retrieve MissingETAssociationMap: " << m_mapname);
        return StatusCode::FAILURE;
      }
      MissingETAssociationHelper metHelper(metMap);
      
      const xAOD::MissingETContainer* coreMet = nullptr;
      for (const auto& coreKey : m_coreMetKeys) {
        if (coreKey.key() == m_corename) {
          coreMet = SG::get(coreKey, ctx);
          break;
        }
      }

      if (!coreMet) {
        ATH_MSG_ERROR("Unable to retrieve MissingETContainer: " << m_corename);
        return StatusCode::FAILURE;
      }

      ATH_MSG_INFO("Retrieved core MET container: " << m_corename);

      for (const xAOD::MissingET* term : *coreMet) {
        ATH_MSG_INFO("  Core MET term: " << term->name());
      }

      //Start for MET Rebuilt
      ATH_MSG_DEBUG( "  MET_Rebuilt_" << jet_type << ":" );

      //Select and flag objects for final MET building ***************************
      if( jet_type.find("PFlow") != std::string::npos) m_metmaker = &m_metmakerPFlow;
      else m_metmaker = &m_metmakerTopo;

      

      //See if we can build terms. This will also add the particles into METMaker
      //METMaker will use the particles to apply OR and JVT
      ATH_MSG_INFO("Starting electron MET rebuild");
      // Electrons
      if( (*m_metmaker)->rebuildMET("RefEle", xAOD::Type::Electron, met.get(), basicSelectionElectrons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_ERROR("Failed to build electron term.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Starting Photon MET rebuild");
      // Photons
      if( (*m_metmaker)->rebuildMET("RefGamma", xAOD::Type::Photon, met.get(), basicSelectionPhotons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_ERROR("Failed to build photon term.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Starting taus MET rebuild");
      // Taus
      if( (*m_metmaker)->rebuildMET("RefTau", xAOD::Type::Tau, met.get(),basicSelectionTaus.asDataVector(),metHelper).isFailure() ) {
        ATH_MSG_ERROR("Failed to build tau term.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Starting muon MET rebuild");
      // Muons
      if( (*m_metmaker)->rebuildMET("RefMuons", xAOD::Type::Muon, met.get(), basicSelectionMuons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_ERROR("Failed to build muon term.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Starting jets MET rebuild");
      // Jets
      if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met.get(), jets, coreMet, metHelper, true).isFailure() ) {
        ATH_MSG_ERROR("Failed to build jet and soft terms.");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("done particles for  MET rebuild");

      // Setting up other values
      MissingETBase::Types::bitmask_t trksource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track);

      ATH_MSG_INFO("starting building FnalTrk for  MET rebuild");

      if((*met)["PVSoftTrk"]) trksource = (*met)["PVSoftTrk"]->source();

      if( met::buildMETSum("FinalTrk", met.get(), trksource).isFailure() ) {
        ATH_MSG_ERROR("Building MET FinalTrk sum failed.");
        return StatusCode::FAILURE;
      }
      MissingETBase::Types::bitmask_t clsource;
      if (jet_type == "AntiKt4EMTopo") clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::EMTopo);
      else clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::UnknownSignal);

      ATH_MSG_INFO("Starting SoftClus for  MET rebuild");

      if((*met)["SoftClus"]) clsource = (*met)["SoftClus"]->source();
      if( met::buildMETSum("FinalClus", met.get(), clsource).isFailure() )
      {
        ATH_MSG_ERROR("Building MET FinalClus sum failed.");
        return StatusCode::FAILURE;
      }

      ATH_MSG_INFO("done Trk and Softclus for  MET rebuild");

      // Get elements with OR and JVT applied
      // We are getting the particles out of METMaker
      std::vector<const xAOD::Electron*> el_elems = met::getMETElements<xAOD::Electron>(*(*met)["RefEle"]);
      std::vector<const xAOD::Photon*> ph_elems = met::getMETElements<xAOD::Photon>(*(*met)["RefGamma"]);
      std::vector<const xAOD::TauJet*> ta_elems = met::getMETElements<xAOD::TauJet>(*(*met)["RefTau"]);
      std::vector<const xAOD::Muon*> mu_elems = met::getMETElements<xAOD::Muon>(*(*met)["RefMuons"]);
      std::vector<const xAOD::Jet*> jet_elems = met::getMETElements<xAOD::Jet>(*(*met)["RefJet"]);

      //For difference histograms we need the jets to have JVT applied without OR.
      //To do this a jets will be fed into a new METMaker with no other particles. This will apply JVT without considering OR with other particles.
      //This produced a new group of jets that can be used for the difference histos

      auto met_jetonly = std::make_unique<xAOD::MissingETContainer>();
      auto aux_jetonly = std::make_unique<xAOD::MissingETAuxContainer>();
      met_jetonly->setStore(aux_jetonly.get());
      MissingETAssociationHelper metHelperJetOnly(metMap);

      ATH_MSG_INFO("Start jetonly mu for  MET rebuild"); 
      //MetMaker needs a muon term for the rebuildJetMET. Since we want this to only have jets I will produce a an empty Muon container to pass
      ConstDataVector<MuonContainer> muons_empty(SG::VIEW_ELEMENTS);
      // Build the muon term required by the METMaker correction
      if ((*m_metmaker)->rebuildMET("RefMuons", xAOD::Type::Muon, met_jetonly.get(), muons_empty.asDataVector(), metHelperJetOnly).isFailure()) {
        ATH_MSG_ERROR("Failed to build empty muon term for jet-only MET");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("Start jetonly jet for  MET rebuild");
      if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_jetonly.get(), jets, coreMet, metHelperJetOnly, true).isFailure()){
        ATH_MSG_ERROR("Failed to build jet and soft terms for second METMaker");
        return StatusCode::FAILURE;
      }
      ATH_MSG_INFO("done jetonly for  MET rebuild");
      std::vector<const xAOD::Jet*> only_jet_elems = met::getMETElements<xAOD::Jet>(*(*met_jetonly)["RefJet"]);

      //Sum up the pT's of the objects
      //Getting tlv, sum, and checking is has particle in event for Diff histos
      const auto [el_tlv, sum_el] = calculateParticleSums(basicSelectionElectrons);
      const auto [mu_tlv, sum_mu] = calculateParticleSums(basicSelectionMuons);
      const auto [tau_tlv, sum_tau] = calculateParticleSums(basicSelectionTaus);
      const auto [photon_tlv, sum_photon] = calculateParticleSums(basicSelectionPhotons);

      bool has_electron = !basicSelectionElectrons.empty();
      bool has_muon = !basicSelectionMuons.empty();
      bool has_tau = !basicSelectionTaus.empty();
      bool has_photon = !basicSelectionPhotons.empty();

      //Start filling histograms

      // Fill MET_Reb Term histograms
      // For each section fill with MET, x, y, phi, and sum for each section

      for(const auto it : *met) {
        std::string name = it->name();
        ATH_MSG_INFO("Looking up MET term: " << name);
        auto it_index = map_index.find(name);
        if (it_index == map_index.end()) {
          ATH_MSG_DEBUG("Skipping unbooked MET term: " << name);
          continue;
        }
        int index = it_index->second;
        
        (m_MET[met_name]).at(index)->Fill((*met)[name.c_str()]->met()/1000., weight);
        (m_MET_x[met_name]).at(index)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
        (m_MET_y[met_name]).at(index)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
        (m_MET_phi[met_name]).at(index)->Fill((*met)[name.c_str()]->phi(), weight);
        (m_MET_sum[met_name]).at(index)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
      }

      ATH_MSG_DEBUG( "  MET_Angles :" );

      double leadPt = 0., subleadPt = 0., leadPhi = 0., subleadPhi = 0.;

      //for Jets find leading and subleading jet
      for (auto jet_itr = jets->begin(); jet_itr != jets->end(); ++jet_itr) {
        if ((*jet_itr)->pt() > leadPt) {
          subleadPt = leadPt;
          subleadPhi = leadPhi;
          leadPt = (*jet_itr)->pt();
          leadPhi = (*jet_itr)->phi();
        }
        else if ((*jet_itr)->pt() > subleadPt) {
          subleadPt = (*jet_itr)->pt();
          subleadPhi = (*jet_itr)->phi();
        }
      }

      //Fill dPhi for Met Reb
      //For leading and subleading jets
      (m_MET_dPhi[met_name]).at(0)->Fill( P4Helpers::deltaPhi( (*met)["FinalClus"]->phi(),leadPhi ), weight );
      (m_MET_dPhi[met_name]).at(1)->Fill( P4Helpers::deltaPhi( (*met)["FinalClus"]->phi(), subleadPhi ), weight );
      (m_MET_dPhi[met_name]).at(3)->Fill( P4Helpers::deltaPhi( (*met)["FinalTrk"]->phi(), leadPhi ), weight );
      (m_MET_dPhi[met_name]).at(4)->Fill( P4Helpers::deltaPhi( (*met)["FinalTrk"]->phi(), subleadPhi ), weight );
  
      leadPt = 0.; leadPhi = 0.;

      xAOD::MuonContainer::const_iterator muon_itr = muons->begin();
      xAOD::MuonContainer::const_iterator muon_end = muons->end();

      for( ; muon_itr != muon_end; ++muon_itr ) {
        if((*muon_itr)->pt() > leadPt) {
          leadPt = (*muon_itr)->pt();
          leadPhi = (*muon_itr)->phi();
        }
      }

      xAOD::ElectronContainer::const_iterator electron_itr = electrons->begin();
      xAOD::ElectronContainer::const_iterator electron_end = electrons->end();

      for( ; electron_itr != electron_end; ++electron_itr ) {
        if((*electron_itr)->pt() > leadPt) {
          leadPt = (*electron_itr)->pt();
          leadPhi = (*electron_itr)->phi();
        }
      }

      //Fill dPhi for MET Reb
      //for leading lepton
      (m_MET_dPhi[met_name]).at(2)->Fill( -remainder( leadPhi - (*met)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi[met_name]).at(5)->Fill( -remainder( leadPhi - (*met)["FinalTrk"]->phi(), 2*M_PI ), weight );

      //Fill Correlation Histos
      //for each fill for MET of FinalTrk/FinalClus vs particle (muon, electron, gamma, tau, jet)
      for(const auto it : *met) {
        std::string name = it->name();

        if (name == "FinalTrk" || name == "FinalClus") {
          continue;
        }

        auto it_index = map_index.find(name);
        if (it_index == map_index.end()) {
          ATH_MSG_DEBUG("Skipping unbooked correlation term: " << name);
          continue;
        }
        int index = it_index->second;

        if(name == "PVSoftTrk")
          (m_MET_CorrFinalTrk[met_name]).at(5)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
        else if(name == "SoftClus")
          (m_MET_CorrFinalClus[met_name]).at(5)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        else {
          (m_MET_CorrFinalTrk[met_name]).at(index)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus[met_name]).at(index)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
      }

      // Fill Resolution histos
      if(m_doTruth){
        ATH_MSG_DEBUG( "  Resolution:" );
        //Fill Rebuilt plots
        (m_MET_Resolution[met_name]).at(0)->Fill(((*met)["FinalClus"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution[met_name]).at(1)->Fill(((*met)["FinalClus"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
        (m_MET_Resolution[met_name]).at(2)->Fill(((*met)["FinalTrk"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution[met_name]).at(3)->Fill(((*met)["FinalTrk"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
      }

      //Fill significance
      if( (*met)["FinalClus"]->sumet() != 0) (m_MET_Significance[met_name]).at(0)->Fill((*met)["FinalClus"]->met()/sqrt((*met)["FinalClus"]->sumet()*1000.), weight);
      if( (*met)["FinalTrk"]->sumet() != 0) (m_MET_Significance[met_name]).at(1)->Fill((*met)["FinalTrk"]->met()/sqrt((*met)["FinalTrk"]->sumet()*1000.), weight);

      TLorentzVector target_tlv;

      // Collecting using Jets with JVT and OR without other elements
      // this is collecting for the calculations for the difference histograms
      const auto [jet_tlv, sum_jet] = calculateParticleSums(only_jet_elems);
      bool has_jet = !only_jet_elems.empty();

      //Fill MET Diff histos
      //for each histo the difference is the Particle information with OR (METMaker) vs particle without OR (no METMaker)
      //this allows the identification of the impact of putting the particles into METMaker
      std::vector<std::uint8_t> has_object = {has_electron, has_photon, has_tau, has_muon, has_jet};

      std::vector<TLorentzVector> obj_tlv = {el_tlv, photon_tlv, tau_tlv, mu_tlv, jet_tlv};
      std::vector<double> obj_sum = {sum_el, sum_photon, sum_tau, sum_mu, sum_jet};

      for(const auto it : *met) {
        auto it_index = map_index.find(it->name());
        if (it_index == map_index.end()) {
          ATH_MSG_DEBUG("Skipping unbooked difference term: " << it->name());
          continue;
        }
        int index = it_index->second;
        if (index >= std::ssize(has_object)) continue;

        if(has_object[index] or (it->sumet() > 0)){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff[met_name]).at(index)->Fill((target_tlv.Pt() - obj_tlv[index].Pt())/1000., weight);
          (m_MET_Diff_x[met_name]).at(index)->Fill((target_tlv.Px() - obj_tlv[index].Px())/1000., weight);
          (m_MET_Diff_y[met_name]).at(index)->Fill((target_tlv.Py() - obj_tlv[index].Py())/1000., weight);
          (m_MET_Diff_phi[met_name]).at(index)->Fill(obj_tlv[index].DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum[met_name]).at(index)->Fill((it->sumet() - obj_sum[index])/1000., weight);
        }
      }

      //Fill MET Kinematic Histos
      //for each particle we are filling with the pT, eta, and phi
      auto toIParticles = [](const auto& particles) {
        return std::vector<const xAOD::IParticle*>(particles.begin(), particles.end());
      };
      std::vector<std::vector<const xAOD::IParticle*>> particle_groups = {
        toIParticles(el_elems), toIParticles(ph_elems), toIParticles(ta_elems), toIParticles(mu_elems), toIParticles(jet_elems) };
      for(unsigned int index = 0; index<5; index++){
        for(const auto p : particle_groups[index]){
          (m_MET_Kinematic_pt[met_name]).at(index)->Fill((p->pt())/1000., weight);
          (m_MET_Kinematic_eta[met_name]).at(index)->Fill(p->eta(), weight);
          (m_MET_Kinematic_phi[met_name]).at(index)->Fill(p->phi(), weight);
        }

        //Fill Multiplicity Histos for Kinematic histos
        //these are the number of each particle in the event saved in the histo
        (m_MET_multi[met_name]).at(index)->Fill(particle_groups[index].size(), weight);
      }

      //EMTopo
      if (jet_type == "AntiKt4EMTopo") {
        // Calo MET
        SG::WriteHandle<xAOD::MissingETContainer> metCaloHandle(m_metCaloKey, ctx);

        auto met_Calo = std::make_unique<xAOD::MissingETContainer>();
        auto met_CaloAux = std::make_unique<xAOD::MissingETAuxContainer>();
        met_Calo->setStore(met_CaloAux.get());

        ATH_CHECK(metCaloHandle.record(std::move(met_Calo),std::move(met_CaloAux)));

        // Use the recorded container through the WriteHandle
        xAOD::MissingETContainer* metCalo = metCaloHandle.ptr();

        ConstDataVector<JetContainer> metJetsEmpty(SG::VIEW_ELEMENTS);

        MissingETAssociationHelper metHelper(metMap);

        if ((*m_metmaker)->rebuildJetMET("RefJet","SoftClus","PVSoftTrk",metCalo,metJetsEmpty.asDataVector(),coreMet,metHelper,true).isFailure()) {
          ATH_MSG_ERROR("Failed to build jet and soft terms.");
          return StatusCode::FAILURE;
        }

        if ((*metCalo)["SoftClus"]) {
          clsource = (*metCalo)["SoftClus"]->source();
        }

        if (met::buildMETSum("FinalClus", metCalo, clsource).isFailure()) {
          ATH_MSG_ERROR("Building MET FinalClus sum failed.");
          return StatusCode::FAILURE;
        }

        auto* finalClus = (*metCalo)["FinalClus"];
        if (finalClus) {
          m_MET_Calo->Fill(finalClus->met() / 1000., weight);
          m_MET_Calo_x->Fill(finalClus->mpx() / 1000., weight);
          m_MET_Calo_y->Fill(finalClus->mpy() / 1000., weight);
          m_MET_Calo_phi->Fill(finalClus->phi(), weight);
          m_MET_Calo_sum->Fill(finalClus->sumet() / 1000., weight);
        }
      }

    }

   return StatusCode::SUCCESS;
  }
  
  // Proc Hists
  StatusCode PhysValMET::procHistograms()
  {
   
    if (m_detailLevel < 10){ //don't do anything if histograms not booked & filled
      return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG ("Finalising hists " << name() << "...");
    //loop over jet types
    for (const auto& jet_type : m_jet_types){
      std::string met_name = "MET_Rebuilt_"+jet_type;
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET[met_name]).size(); ++i) {
        //Term hists
        (m_MET[met_name]).at(i)->Sumw2();
        (m_MET_x[met_name]).at(i)->Sumw2();
        (m_MET_y[met_name]).at(i)->Sumw2();
        (m_MET_phi[met_name]).at(i)->Sumw2();
        (m_MET_sum[met_name]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Diff[met_name]).size(); ++i) {
        //Diff hists
        (m_MET_Diff[met_name]).at(i)->Sumw2();
        (m_MET_Diff_x[met_name]).at(i)->Sumw2();
        (m_MET_Diff_y[met_name]).at(i)->Sumw2();
        (m_MET_Diff_phi[met_name]).at(i)->Sumw2();
        (m_MET_Diff_sum[met_name]).at(i)->Sumw2();
      }

      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalTrk[met_name]).size(); ++i) {
        //Get for CorFinalTrk
        (m_MET_CorrFinalTrk[met_name]).at(i)->Sumw2();
      }
  
      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalClus[met_name]).size(); ++i) {
        //get for CorrFinalClus
        (m_MET_CorrFinalClus[met_name]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Significance[met_name]).size(); ++i) {
        //For Significance
        (m_MET_Significance[met_name]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Resolution[met_name]).size(); ++i) { 
        //for Resolution
      	(m_MET_Resolution[met_name]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_dPhi[met_name]).size(); ++i) {
        //for dPhi
        (m_MET_dPhi[met_name]).at(i)->Sumw2();
      }
  
      int nBins = (m_MET[met_name]).at(7)->GetNbinsX();
      for(int i=1;i<=nBins;i++){
        //For MET Cumu
        double err;
        (m_MET_Cumu[met_name]).at(0)->SetBinContent(i, (m_MET[met_name]).at(8)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu[met_name]).at(0)->SetBinError(i, err);
        (m_MET_Cumu[met_name]).at(1)->SetBinContent(i, (m_MET[met_name]).at(7)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu[met_name]).at(1)->SetBinError(i, err);
      }
      //For MET Cumu
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Cumu[met_name]).size(); ++i) {
        m_MET_Cumu[met_name].at(i)->Scale(1./(m_MET_Cumu[met_name]).at(i)->GetBinContent(1));
      }
  
    }
  
    //Get Sumw2 for Calo
    m_MET_Calo->Sumw2();
    m_MET_Calo_x->Sumw2();
    m_MET_Calo_y->Sumw2();
    m_MET_Calo_phi->Sumw2();
    m_MET_Calo_sum->Sumw2();
 
    return StatusCode::SUCCESS;
  }
  
  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 
  
  //Muon Selection
  bool PhysValMET::Accept(const xAOD::Muon* mu)
  {
    if( mu->pt()<2.5e3 || mu->pt()/std::cosh(mu->eta())<4e3 ) return false;
    return static_cast<bool> (m_muonSelTool->accept(*mu));
  }
  //Electron selection
  bool PhysValMET::Accept(const xAOD::Electron* el)
  {
    if( fabs(el->eta())>2.47 || el->pt()<10e3 ) return false;
    return static_cast<bool> (m_elecSelLHTool->accept(el));
  }
  //Photon selection
  bool PhysValMET::Accept(const xAOD::Photon* ph)
  {
    if( !(ph->author()&20) || std::abs(ph->eta())>2.47 || ph->pt()<10e3 ) return false;
    return static_cast<bool>(m_photonSelIsEMTool->accept(ph));
  }
  //Tau selection
  bool PhysValMET::Accept(const xAOD::TauJet* tau)
  { return static_cast<bool> (m_tauSelTool->accept( *tau )); }

}

//  LocalWords:  str
 
