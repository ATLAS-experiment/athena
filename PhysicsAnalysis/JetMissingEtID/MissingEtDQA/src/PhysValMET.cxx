///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// PhysValMET.cxx 
// Implementation file for class PhysValMET
// Author: Daniel Buescher <daniel.buescher@cern.ch>, Philipp Mogg <philipp.mogg@cern.ch>
/////////////////////////////////////////////////////////////////// 
// new version by Owen Darragh  2026
///////////////////////////////////////////////////////////////////

// PhysVal includes
#include "PhysValMET.h"

// STL includes
#include <cmath>
#include <map>
#include <vector>

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

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackParticle.h"
#include "PATCore/AcceptData.h"
#include "METUtilities/METHelpers.h"
#include "AthContainers/Decorator.h"

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
    declareProperty( "DoTruth", m_doTruth = false );
    declareProperty( "InputIsDAOD",    m_inputIsDAOD = false              );
    declareProperty( "DoMETRefPlots",  m_doMETRefPlots = false           );
    declareProperty( "METMapName",     m_mapname   = "METAssoc"          );
    declareProperty( "METCoreName",    m_corename  = "MET_Core"          );
  }
  
  // Destructor
  ///////////////
  PhysValMET::~PhysValMET()
  {
    m_names.clear();
    m_types.clear();
    m_terms.clear();
    m_MET.clear();
    m_MET_x.clear();
    m_MET_y.clear();
    m_MET_phi.clear();
    m_MET_sum.clear();
    m_MET_Diff.clear();
    m_MET_Diff_x.clear();
    m_MET_Diff_y.clear();
    m_MET_Diff_phi.clear();
    m_MET_Diff_sum.clear();
    m_MET_Cumu.clear();
    m_MET_Resolution.clear();
    m_MET_Significance.clear();
    m_MET_dPhi.clear();
    m_MET_CorrFinalTrk.clear();
    m_MET_CorrFinalClus.clear();
    m_MET_Kine_pt.clear();
    m_MET_Kine_eta.clear();
    m_MET_Kine_phi.clear();
    m_MET_multi.clear();
  }
   
  // Athena algtool's Hooks
  ////////////////////////////

  //initialize
  StatusCode PhysValMET::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");    
    ATH_CHECK(ManagedMonitorToolBase::initialize());

    //setup names
    m_names.clear();
    m_names["RefEle"] = "Electron term";
    m_names["RefGamma"] = "Photon term";
    m_names["RefTau"] = "Tau term";
    m_names["RefMuons"] = "Muon term";
    m_names["RefJet"] = "Jet term";
    m_names["SoftClus"] = "Cluster-based soft term";
    m_names["PVSoftTrk"] = "Track-based soft term (PV-matched)";
    m_names["FinalTrk"] = "Total MET with TST";
    m_names["FinalClus"] = "Total MET with CST";
    m_names["Track"] = "Track MET, loose selection";
    m_names["PVTrack_Nominal"] = "Track MET for highest sum p_{T}^{2} PV";
    m_names["PVTrack_Pileup"] = "Track MET for each pileup vertex";

    //define the Jet types
    m_types.clear();
    m_types.emplace_back("AntiKt4EMTopo");
    m_types.emplace_back("AntiKt4EMPFlow");

    //setup terms
    m_terms.clear();
    m_terms.emplace_back("RefEle");
    m_terms.emplace_back("RefGamma");
    m_terms.emplace_back("RefTau");
    m_terms.emplace_back("RefMuons");
    m_terms.emplace_back("RefJet");
    m_terms.emplace_back("SoftClus");
    m_terms.emplace_back("PVSoftTrk");
    m_terms.emplace_back("FinalTrk");
    m_terms.emplace_back("FinalClus");

    ATH_MSG_INFO("Retrieving tools...");

    //retrieve tools
    ATH_CHECK( m_metmakerTopo.retrieve() ); 
    ATH_CHECK( m_metmakerPFlow.retrieve() );
    ATH_CHECK( m_muonSelTool.retrieve() );
    ATH_CHECK( m_elecSelLHTool.retrieve() );
    ATH_CHECK( m_photonSelIsEMTool.retrieve() );
    ATH_CHECK( m_tauSelTool.retrieve() );
    ATH_CHECK( m_jvtToolEM.retrieve() );
    ATH_CHECK( m_jvtToolPFlow.retrieve() );

    //Clearing vector
    m_MET.clear();
    m_MET_x.clear();
    m_MET_y.clear();
    m_MET_phi.clear();
    m_MET_sum.clear();
    m_MET_Diff.clear();
    m_MET_Diff_x.clear();
    m_MET_Diff_y.clear();
    m_MET_Diff_phi.clear();
    m_MET_Diff_sum.clear();
    m_MET_Cumu.clear();
    m_MET_Resolution.clear();
    m_MET_Significance.clear();
    m_MET_dPhi.clear();
    m_MET_CorrFinalTrk.clear();
    m_MET_CorrFinalClus.clear();
    m_MET_Kine_pt.clear();
    m_MET_Kine_eta.clear();
    m_MET_Kine_phi.clear();
    m_MET_multi.clear(); 
   
    return StatusCode::SUCCESS;
  }
  
  //Book histograms
  StatusCode PhysValMET::bookHistograms()
  { 

    ATH_MSG_INFO ("Booking hists " << name() << "...");
      
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
    if (m_detailLevel >= 10) {

      //loop through jet types
      for (const auto& jet_type : m_types){
        //define variables
        std::string name_met;
        std::string name_sub;
        std::vector<std::string> corrClus_names;
        std::vector<std::string> corrTrk_names;
        std::vector<std::string> sum_names;
        std::string dir;

        corrClus_names.emplace_back("RefEle");
        corrClus_names.emplace_back("RefGamma");
        corrClus_names.emplace_back("RefTau");
        corrClus_names.emplace_back("RefMuons");
        corrClus_names.emplace_back("RefJet");
        corrClus_names.emplace_back("SoftClus");
        
        corrTrk_names.emplace_back("RefEle");
        corrTrk_names.emplace_back("RefGamma");
        corrTrk_names.emplace_back("RefTau");
        corrTrk_names.emplace_back("RefMuons");
        corrTrk_names.emplace_back("RefJet");
        corrTrk_names.emplace_back("PVSoftTrk");

        sum_names.emplace_back("RefEle");
        sum_names.emplace_back("RefGamma");
        sum_names.emplace_back("RefTau");
        sum_names.emplace_back("RefMuons");
        sum_names.emplace_back("RefJet");

        //Create and Register histograms for and Rebuilt
        std::vector <std::string> met_type = {"MET_Rebuilt_"};
        ATH_MSG_INFO("****STARTING****");

        //loop for rebuilt
        for (const auto& type : met_type){
          //define variables 
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
          std::vector<TH1D*> v_MET_Kine_pt;
          std::vector<TH1D*> v_MET_Kine_eta;
          std::vector<TH1D*> v_MET_Kine_phi;
          std::vector<TH1D*> v_MET_multi;

          //Create histograms

          //Setup Term histograms
          for(const auto& term : m_terms){
            v_MET.push_back( new  TH1D((name_met + "_" + term).c_str(), (name_met + " " + m_names[term] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
            v_MET_x.push_back( new  TH1D((name_met + "_" + term +"_x").c_str(), (name_met + " " + m_names[term] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
            v_MET_y.push_back( new  TH1D((name_met + "_" + term + "_y").c_str(), (name_met + " " + m_names[term] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
            v_MET_phi.push_back( new  TH1D((name_met + "_" + term + "_phi").c_str(), (name_met + " " + m_names[term] + " phi; #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
            v_MET_sum.push_back( new  TH1D((name_met + "_" + term + "_sum").c_str(), (name_met + " " + m_names[term] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET) );
            m_dir_met.push_back("MET/" + name_met + "/Terms/" + term + "/");
          }
        
          m_MET[name_met] = v_MET;
          m_MET_x[name_met] = v_MET_x;
          m_MET_y[name_met] = v_MET_y;
          m_MET_phi[name_met] = v_MET_phi;
          m_MET_sum[name_met] = v_MET_sum;

          //Register Term histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET.size(); ++i) {
            ATH_CHECK(regHist(m_MET[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_sum[name_met].at(i),m_dir_met[i],all));
          }
 
          //Setup cumulative hists
          name_sub = name_met + "/Cumulative";
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalClus").c_str(), (name_met + " CST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalTrk").c_str(), (name_met + " TST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
        
          m_MET_Cumu[name_met] = v_MET_Cumu;
        
          //Register cumulative hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Cumu.size(); ++i) {
            ATH_CHECK(regHist(m_MET_Cumu[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          //Setup Residual histograms        
          name_sub = name_met + "/Residuals";
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_x").c_str(), ("x-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_y").c_str(), ("y-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_x").c_str(), ("x-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_y").c_str(), ("y-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
        
          m_MET_Resolution[name_met] = v_MET_Resolution;
        
          //register Residual histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Resolution.size(); ++i){
            ATH_CHECK(regHist(m_MET_Resolution[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //Setup Significance hists        
          name_sub = name_met + "/Significance";
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalClus").c_str(), ("MET / sqrt(sumet) for " + name_met + " CST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalTrk").c_str(), ("MET / sqrt(sumet) for " + name_met + " TST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
        
          m_MET_Significance[name_met] = v_MET_Significance;
        
          //Register Significance hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Significance.size(); ++i) {
            ATH_CHECK(regHist(m_MET_Significance[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //Setup dPhi hists        
          name_sub = name_met + "/dPhi";
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalClus").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " CST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalClus").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " CST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalClus").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " CST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " TST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " TST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalTrk").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " TST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
        
          m_MET_dPhi[name_met] = v_MET_dPhi;
        
          //Register dPhi hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_dPhi.size(); ++i) {
            ATH_CHECK(regHist(m_MET_dPhi[name_met].at(i),"MET/" + name_sub + "/",all));
          }
         
          //Setup Correlation hists
          name_sub = name_met + "/Correlations";

          v_MET_CorrFinalClus.reserve(corrClus_names.size());

          for(const auto& it : corrClus_names) {
            v_MET_CorrFinalClus.push_back( new  TH2D((name_met + "_" + it + "_FinalClus").c_str(), (name_met + " " + m_names[it] + " vs. CST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,CST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }
          v_MET_CorrFinalTrk.reserve(corrTrk_names.size());

          for(const auto& it : corrTrk_names) {
            v_MET_CorrFinalTrk.push_back( new  TH2D((name_met + "_" + it + "_FinalTrk").c_str(), (name_met + " " + m_names[it] + " vs. TST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,TST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }

          m_MET_CorrFinalClus[name_met] = v_MET_CorrFinalClus;
          m_MET_CorrFinalTrk[name_met] = v_MET_CorrFinalTrk;

          //Register Correlation hists
          for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalTrk.size(); ++i) {
            ATH_CHECK(regHist(m_MET_CorrFinalTrk[name_met].at(i),"MET/" + name_sub + "/",all));
          }
          for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalClus.size(); ++i) {
            ATH_CHECK(regHist(m_MET_CorrFinalClus[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          m_dir_met.clear();
         
          //Setup Diff histograms
          for(const auto& it : sum_names) {
            v_MET_Diff.push_back( new  TH1D((name_met + "_Diff_" + it).c_str(), ("MET_Diff " + m_names[it] + " in " + name_met +"; #Sigma p_{T}^{Val} - #Sigma p_{T}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200));
            v_MET_Diff_x.push_back( new  TH1D((name_met + "_Diff_" + it +"_x").c_str(), ("MET_Diff x " + m_names[it] + " in " + name_met +"; #Sigma p_{x}^{Val} - #Sigma p_{x}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200) );
            v_MET_Diff_y.push_back( new  TH1D((name_met + "_Diff_" + it +"_y").c_str(), ("MET_Diff y " + m_names[it] + " in " + name_met +"; #Sigma p_{y}^{Val} - #Sigma p_{y}^{No Val} [GeV]; Entries / 4 GeV").c_str(), nbinpxy, -200, 200) );
            v_MET_Diff_phi.push_back( new  TH1D((name_met + "_Diff_" + it +"_phi").c_str(), ("MET_Diff phi " + m_names[it] + " in " + name_met +"; #Delta#Phi(#Sigma p_{T}^{Val},#Sigma p_{T}^{No Val}); Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
            v_MET_Diff_sum.push_back( new  TH1D((name_met + "_Diff_" + it +"_sum").c_str(), ("MET_Diff sumet " + m_names[it] + " in " + name_met +"; E_{T}^{sum Val} - #Sigma |p_{T}^{No Val}| [GeV]; Entries / 6 GeV").c_str(), nbinpxy, -300, 300) );
            m_dir_met.push_back("MET/" + name_met + "/Differences/" + it + "/");
          }
        
          m_MET_Diff[name_met] = v_MET_Diff;
          m_MET_Diff_x[name_met] = v_MET_Diff_x;
          m_MET_Diff_y[name_met] = v_MET_Diff_y;
          m_MET_Diff_phi[name_met] = v_MET_Diff_phi;
          m_MET_Diff_sum[name_met] = v_MET_Diff_sum;
        
          //Register Diff histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Diff.size(); ++i) {
            ATH_CHECK(regHist(m_MET_Diff[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_sum[name_met].at(i),m_dir_met[i],all));
          }
          m_dir_met.clear();

          //Setup Kin histos
          for(const auto& it : sum_names){
            v_MET_Kine_pt.push_back( new  TH1D((name_met + "_Kine_" + it+"_pt").c_str(), ("MET_Kine pt " + m_names[it] + " in " + name_met +"; p_{T} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, 0, 300));
            v_MET_Kine_eta.push_back( new  TH1D((name_met + "_Kine_" + it +"_eta").c_str(), ("MET_Kine eta " + m_names[it] + " in " + name_met +"; #eta ; Entries / 0.1").c_str(), 100, -5, 5) );
            v_MET_Kine_phi.push_back( new  TH1D((name_met + "_Kine_" + it +"_phi").c_str(), ("MET_Kine phi " + m_names[it] + " in " + name_met +"; #Phi ; Entries / 0.1").c_str(), nbinphi, -binphi, binphi) );
            v_MET_multi.push_back( new  TH1D((name_met + "_multi_" + it).c_str(), ("MET_multi " + m_names[it] + " in " + name_met +"; Multiplicity; Entries").c_str(), 20,-0.5,20.5) );
            m_dir_met.push_back("MET/" + name_met + "/Kinematics/" + it + "/");                                      
          }
          m_MET_Kine_pt[name_met] = v_MET_Kine_pt;
          m_MET_Kine_eta[name_met] = v_MET_Kine_eta;
          m_MET_Kine_phi[name_met] = v_MET_Kine_phi;
          m_MET_multi[name_met] = v_MET_multi;

          //Register Kin histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Kine_pt.size(); ++i){
            ATH_CHECK(regHist(m_MET_Kine_pt[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Kine_eta[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Kine_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_multi[name_met].at(i),m_dir_met[i],all));
          }
        // End of loop
        }
      }  
      
      //Now MET_Calo
  
      //variables
      std::string name_met = "MET_Calo";
      std::string dir = "MET/" + name_met + "/";

      //Create and register Calo hists
      ATH_CHECK(regHist(m_MET_Calo = new  TH1D("Calo", (name_met + " " + m_names["Calo"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_x = new  TH1D("Calo_x", (name_met + " " + m_names["Calo"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_y = new  TH1D("Calo_y", (name_met + " " + m_names["Calo"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_phi = new  TH1D("Calo_phi", (name_met + " " + m_names["Calo"] + " phi;  #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_sum = new  TH1D("Calo_sum", (name_met + " " + m_names["Calo"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), dir, all));
    }
  
    return StatusCode::SUCCESS;      
  }

  //Fill Histograms
  StatusCode PhysValMET::fillHistograms(const EventContext& /*ctx*/){
    ATH_MSG_DEBUG ("Filling hists " << name() << "...");

    
    const xAOD::EventInfo* eventInfo(nullptr);
    ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));

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

    ATH_MSG_INFO("Physics objects");

    //Set up Physics Objects

    //Muons 
    const xAOD::MuonContainer* muons = nullptr;
    ATH_CHECK( evtStore()->retrieve(muons,m_muonColl) );
    if (!muons) {
      ATH_MSG_ERROR ( "Failed to retrieve Muon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<MuonContainer> BasicSelectionMuons(SG::VIEW_ELEMENTS);
    for(const auto mu : *muons) {
      if(Accept(mu)) {
        BasicSelectionMuons.push_back(mu);
      }
    }

    //Electrons
    const xAOD::ElectronContainer* electrons = nullptr;
    ATH_CHECK( evtStore()->retrieve(electrons,m_eleColl) );
    if (!electrons) {
      ATH_MSG_ERROR ( "Failed to retrieve Electron container. Exiting." );
      return StatusCode::FAILURE;
    }
   ConstDataVector<ElectronContainer> BasicSelectionElectrons(SG::VIEW_ELEMENTS);
   for(const auto el : *electrons) {
     if(Accept(el)) {
       BasicSelectionElectrons.push_back(el);
     }
   }

    //Photons
    const xAOD::PhotonContainer* photons = nullptr;
    ATH_CHECK( evtStore()->retrieve(photons,m_gammaColl) );
    if (!electrons){
      ATH_MSG_ERROR ( "Failed to retrieve Photon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<PhotonContainer> BasicSelectionPhotons(SG::VIEW_ELEMENTS);
    for(const auto ph : *photons) {
      if(Accept(ph)) {
        BasicSelectionPhotons.push_back(ph);
      }
    }

    //Tau
    const TauJetContainer* taus = nullptr;
    ATH_CHECK( evtStore()->retrieve(taus, m_tauColl) );
    if(!taus) {
      ATH_MSG_ERROR("Failed to retrieve TauJet container: " << m_tauColl);
      return StatusCode::SUCCESS;
    }
    ConstDataVector<TauJetContainer> BasicSelectionTaus(SG::VIEW_ELEMENTS);
    for(const auto tau : *taus) {
      if(Accept(tau)) {
        BasicSelectionTaus.push_back(tau);
      }
    }

    //Jets
    for (const auto& jet_type : m_types){
      std::string name_jet = jet_type + "Jets";
      const xAOD::JetContainer* jets = nullptr;
      ATH_CHECK( evtStore()->retrieve(jets,name_jet) );
      if (!jets) {
        ATH_MSG_ERROR ( "Failed to retrieve Jet container: " << name_jet << ". Exiting." );
        return StatusCode::FAILURE;
      }
      ConstDataVector<JetContainer> BasicSelectionJets(SG::VIEW_ELEMENTS);
      for(const auto jet : *jets){ //for jets assign jets
        BasicSelectionJets.push_back(jet);
      }

      //Prepare Rebuilding MET
      ATH_MSG_INFO( "  Rebuilding MET_" << jet_type );
      MissingETContainer* met = new MissingETContainer(); //Define MET Container
      if( evtStore()->record(met,("MET_Rebuilt_"+jet_type).c_str()).isFailure() ) {
        ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Rebuilt_" << jet_type);
        return StatusCode::FAILURE;
      }
      MissingETAuxContainer* met_Aux = new MissingETAuxContainer(); //Define MET Aux container
      if( evtStore()->record(met_Aux,("MET_Rebuilt_"+jet_type+"Aux").c_str()).isFailure() ) {
        ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Rebuilt_" << jet_type);
        return StatusCode::FAILURE;
      }

      met->setStore(met_Aux); //MET Reb container for METMaker

      //define map and core name for METMaker
      m_mapname = "METAssoc_"+jet_type;
      m_corename = "MET_Core_"+jet_type;
      const MissingETAssociationMap* metMap = nullptr;
      //check is can retrieve Map and Container
      if( evtStore()->retrieve(metMap, m_mapname).isFailure() ) {
        ATH_MSG_WARNING("Unable to retrieve MissingETAssociationMap: " << m_mapname);
        return StatusCode::SUCCESS;
      }
      MissingETAssociationHelper metHelper(metMap);
      const MissingETContainer* coreMet(nullptr);
      if( evtStore()->retrieve(coreMet, m_corename).isFailure() ) 
      {
        ATH_MSG_WARNING("Unable to retrieve MissingETContainer: " << m_corename);
        return StatusCode::SUCCESS;
      }

      //Start for MET Rebuilt
      ATH_MSG_INFO( "  MET_Rebuilt_" << jet_type << ":" );

      //Select and flag objects for final MET building ***************************
      if( jet_type.find("PFlow") != std::string::npos) m_metmaker = &m_metmakerPFlow;
      else m_metmaker = &m_metmakerTopo;

      auto m_metmaker2 = m_metmaker;

      //See if we can build terms. This will also add the particles into METMaker
      // Electrons
      if( (*m_metmaker)->rebuildMET("RefEle", xAOD::Type::Electron, met, BasicSelectionElectrons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_WARNING("Failed to build electron term.");
      }
      // Photons
      if( (*m_metmaker)->rebuildMET("RefGamma", xAOD::Type::Photon, met, BasicSelectionPhotons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_WARNING("Failed to build photon term.");
      }
      // Taus
      if( (*m_metmaker)->rebuildMET("RefTau", xAOD::Type::Tau, met,BasicSelectionTaus.asDataVector(),metHelper).isFailure() ) {
        ATH_MSG_WARNING("Failed to build tau term.");
      }
      // Muons
      if( (*m_metmaker)->rebuildMET("RefMuons", xAOD::Type::Muon, met, BasicSelectionMuons.asDataVector(), metHelper).isFailure() ) {
        ATH_MSG_WARNING("Failed to build muon term.");
      }
      // Jets
      if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met, jets, coreMet, metHelper, true).isFailure() ) {
        ATH_MSG_WARNING("Failed to build jet and soft terms.");
      }

      // Setting up other values
      MissingETBase::Types::bitmask_t trksource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track);
      if((*met)["PVSoftTrk"]) trksource = (*met)["PVSoftTrk"]->source();
      if( met::buildMETSum("FinalTrk", met, trksource).isFailure() ) {
        ATH_MSG_WARNING("Building MET FinalTrk sum failed.");
      }
      MissingETBase::Types::bitmask_t clsource;
      if (jet_type == "AntiKt4EMTopo") clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::EMTopo);
      else clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::UnknownSignal);
      std::cout<<"___SoftClus___"<<std::endl;
      if((*met)["SoftClus"]) clsource = (*met)["SoftClus"]->source();
      if( met::buildMETSum("FinalClus", met, clsource).isFailure() )
      {
        ATH_MSG_WARNING("Building MET FinalClus sum failed.");
      }

      // Get elements with OR and JVT applied
      std::vector<const xAOD::Electron*> el_elems = met::getMETElements<xAOD::Electron>(*(*met)["RefEle"]);
      std::vector<const xAOD::Photon*> ph_elems = met::getMETElements<xAOD::Photon>(*(*met)["RefGamma"]);
      std::vector<const xAOD::TauJet*> ta_elems = met::getMETElements<xAOD::TauJet>(*(*met)["RefTau"]);
      std::vector<const xAOD::Muon*> mu_elems = met::getMETElements<xAOD::Muon>(*(*met)["RefMuons"]);
      std::vector<const xAOD::Jet*> jet_elems = met::getMETElements<xAOD::Jet>(*(*met)["RefJet"]);

      //Getting jets with JVT and OR without other particles applied. This is used for Jet Diff histos
      auto met_jetonly = std::make_unique<xAOD::MissingETContainer>();
      auto aux_jetonly = std::make_unique<xAOD::MissingETAuxContainer>();
      met_jetonly->setStore(aux_jetonly.get());
      (*m_metmaker2)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_jetonly.get(), jets, coreMet, metHelper, true);
      std::vector<const xAOD::Jet*> only_jet_elems = met::getMETElements<xAOD::Jet>(*(*met_jetonly)[str_jet]);

      //Sum up the pT's of the objects
      bool has_muon = 0, has_electron = 0, has_photon = 0, has_tau = 0, has_jet = 0;

      //Getting tlv, sum, and checking is has particle in event for Diff histos
      //electron
      TLorentzVector el_tlv;
      double sum_el = 0;
      for(const auto p : BasicSelectionElectrons){
        el_tlv += p->p4();
        sum_el += p->pt();
        has_electron = 1;
      }
      //muon
      TLorentzVector mu_tlv;
      double sum_mu = 0;
      for(const auto p : BasicSelectionMuons){
        mu_tlv += p->p4();
        sum_mu += p->pt();
        has_muon = 1;
      }
      //Tau
      TLorentzVector tau_tlv;
      double sum_tau = 0;
      for(const auto p : BasicSelectionTaus){
        tau_tlv += p->p4();
        sum_tau += p->pt();
        has_tau = 1;
      }
      //photon
      TLorentzVector photon_tlv;
      double sum_photon = 0;
      for(const auto p : BasicSelectionPhotons){
        photon_tlv += p->p4();
        sum_photon += p->pt();
        has_photon = 1;
      }

      //Start filling histograms

      std::cout<<"___Fill MET Reb___"<<std::endl;
      // Fill MET_Reb Term histograms
      for(const auto it : *met) {
        std::string name = it->name();
        std::cout<< name <<std::endl;
        if(name == "RefEle"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefGamma"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefTau"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefMuons"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefJet"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "SoftClus"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "PVSoftTrk"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "FinalTrk"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "FinalClus"){
          (m_MET["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met)[name.c_str()]->sumet()/1000., weight);
        }
      }

      //Fill MET Angles
      ATH_MSG_INFO( "  MET_Angles :" );

      //define vars
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
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(0)->Fill( -remainder( leadPhi - (*met)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(1)->Fill( -remainder( subleadPhi - (*met)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(3)->Fill( -remainder( leadPhi - (*met)["FinalTrk"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(4)->Fill( -remainder( subleadPhi - (*met)["FinalTrk"]->phi(), 2*M_PI ), weight );
  
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

      //Fill dPhi for MET Rebuilt Final Clus and Final trk
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(2)->Fill( -remainder( leadPhi - (*met)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(5)->Fill( -remainder( leadPhi - (*met)["FinalTrk"]->phi(), 2*M_PI ), weight );

      //Fill Correlation Histos
      for(const auto it : *met) {
        std::string name = it->name();
        if(name == "RefEle"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefGamma"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefTau"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefMuons"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefJet"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
        if(name == "PVSoftTrk"){
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalTrk"]->met()/1000., weight);
        }
        if(name == "SoftClus"){
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met)[name.c_str()]->met()/1000.,(*met)["FinalClus"]->met()/1000., weight);
        }
      }

      // Fill Resolution histos
      if(m_doTruth){
        ATH_MSG_INFO( "  Resolution:" );
        //Fill Rebuilt plots
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(0)->Fill(((*met)["FinalClus"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(1)->Fill(((*met)["FinalClus"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(2)->Fill(((*met)["FinalTrk"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(3)->Fill(((*met)["FinalTrk"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
      }

      //Fill significance
      if( (*met)["FinalClus"]->sumet() != 0) (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met)["FinalClus"]->met()/sqrt((*met)["FinalClus"]->sumet()*1000.), weight);
      if( (*met)["FinalTrk"]->sumet() != 0) (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met)["FinalTrk"]->met()/sqrt((*met)["FinalTrk"]->sumet()*1000.), weight);

      TLorentzVector target_tlv;

      // Collecting using Jets with JVT and OR without other elements
      TLorentzVector jet_tlv;
      double sum_jet = 0;
      for(const auto jet : only_jet_elems) {
        jet_tlv += jet->p4();
        sum_jet += jet->pt();
        has_jet = 1;
      }

      //Fill MET Diff histos
      for(const auto it : *met) {
        if(it->name() == "RefEle" && (has_electron or (it->sumet() > 0))){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Pt() - el_tlv.Pt())/1000., weight);
          (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Px() - el_tlv.Px())/1000., weight);
          (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Py() - el_tlv.Py())/1000., weight);
          (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(0)->Fill(el_tlv.DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(0)->Fill((it->sumet() - sum_el)/1000., weight);
        }
        if(it->name() == "RefGamma" && (has_photon or (it->sumet() > 0))){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Pt() - photon_tlv.Pt())/1000., weight);
          (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Px() - photon_tlv.Px())/1000., weight);
          (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Py() - photon_tlv.Py())/1000., weight);
          (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(1)->Fill(photon_tlv.DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(1)->Fill((it->sumet() - sum_photon)/1000., weight);
        }
        if(it->name() == "RefTau" && (has_tau or (it->sumet() > 0))){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Pt() - tau_tlv.Pt())/1000., weight);
          (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Px() - tau_tlv.Px())/1000., weight);
          (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Py() - tau_tlv.Py())/1000., weight);
          (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(2)->Fill(tau_tlv.DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(2)->Fill((it->sumet() - sum_tau)/1000., weight);
        }
        if(it->name() == "RefMuons" && (has_muon or (it->sumet() > 0))){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Pt() - mu_tlv.Pt())/1000., weight);
          (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Px() - mu_tlv.Px())/1000., weight);
          (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Py() - mu_tlv.Py())/1000., weight);
          (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(3)->Fill(mu_tlv.DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(3)->Fill((it->sumet() - sum_mu)/1000., weight);
        }
        if(it->name() == "RefJet" && (has_jet or (it->sumet() > 0))){
          target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
          (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Pt() - jet_tlv.Pt())/1000., weight);
          (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Px() - jet_tlv.Px())/1000., weight);
          (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Py() - jet_tlv.Py())/1000., weight);
          (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(4)->Fill(jet_tlv.DeltaPhi(target_tlv), weight);
          (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(4)->Fill((it->sumet() - sum_jet)/1000., weight);
        }
      }

      //Fill Kin Histos
      for(const auto p : el_elems){
          (m_MET_Kine_pt["MET_Rebuilt_"+jet_type]).at(0)->Fill((p->pt())/1000., weight);
          (m_MET_Kine_eta["MET_Rebuilt_"+jet_type]).at(0)->Fill(p->eta(), weight);
          (m_MET_Kine_phi["MET_Rebuilt_"+jet_type]).at(0)->Fill(p->phi(), weight);
      }
      for(const auto p : ph_elems){
          (m_MET_Kine_pt["MET_Rebuilt_"+jet_type]).at(1)->Fill((p->pt())/1000., weight);
          (m_MET_Kine_eta["MET_Rebuilt_"+jet_type]).at(1)->Fill(p->eta(), weight);
          (m_MET_Kine_phi["MET_Rebuilt_"+jet_type]).at(1)->Fill(p->phi(), weight);
      }
      for(const auto p : ta_elems){
          (m_MET_Kine_pt["MET_Rebuilt_"+jet_type]).at(2)->Fill((p->pt())/1000., weight);
          (m_MET_Kine_eta["MET_Rebuilt_"+jet_type]).at(2)->Fill(p->eta(), weight);
          (m_MET_Kine_phi["MET_Rebuilt_"+jet_type]).at(2)->Fill(p->phi(), weight);
      }
      for(const auto p : mu_elems){
          (m_MET_Kine_pt["MET_Rebuilt_"+jet_type]).at(3)->Fill((p->pt())/1000., weight);
          (m_MET_Kine_eta["MET_Rebuilt_"+jet_type]).at(3)->Fill(p->eta(), weight);
          (m_MET_Kine_phi["MET_Rebuilt_"+jet_type]).at(3)->Fill(p->phi(), weight);
      }
      for(const auto p : jet_elems){
          (m_MET_Kine_pt["MET_Rebuilt_"+jet_type]).at(4)->Fill((p->pt())/1000., weight);
          (m_MET_Kine_eta["MET_Rebuilt_"+jet_type]).at(4)->Fill(p->eta(), weight);
          (m_MET_Kine_phi["MET_Rebuilt_"+jet_type]).at(4)->Fill(p->phi(), weight);
      }
      //Fill Multiplicity Histos
      (m_MET_multi["MET_Rebuilt_"+jet_type]).at(0)->Fill(el_elems.size(), weight);
      (m_MET_multi["MET_Rebuilt_"+jet_type]).at(1)->Fill(ph_elems.size(), weight);
      (m_MET_multi["MET_Rebuilt_"+jet_type]).at(2)->Fill(ta_elems.size(), weight);
      (m_MET_multi["MET_Rebuilt_"+jet_type]).at(3)->Fill(mu_elems.size(), weight);
      (m_MET_multi["MET_Rebuilt_"+jet_type]).at(4)->Fill(jet_elems.size(), weight);


      //EMTopo
      if(jet_type == "AntiKt4EMTopo") {
        //Calo MET
        //const xAOD::JetContainer* emptyjets = 0;
        ConstDataVector<JetContainer> metJetsEmpty(SG::VIEW_ELEMENTS);
        MissingETContainer* met_Calo = new MissingETContainer();
        if( evtStore()->record(met_Calo,("MET_Calo"+jet_type).c_str()).isFailure() ) {
          ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Calo_" << jet_type);
          return StatusCode::FAILURE;
        }
        MissingETAuxContainer* met_CaloAux = new MissingETAuxContainer();
        if( evtStore()->record(met_CaloAux,("MET_Calo"+jet_type+"Aux").c_str()).isFailure() ) {
          ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Calo" << jet_type);
          return StatusCode::FAILURE;
        }
        met_Calo->setStore(met_CaloAux);
        MissingETAssociationHelper metHelper(metMap);
        if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_Calo, metJetsEmpty.asDataVector(), coreMet, metHelper, true).isFailure() ) {
          ATH_MSG_WARNING("Failed to build jet and soft terms.");
        }
        
        if((*met_Calo)["SoftClus"]) clsource = (*met_Calo)["SoftClus"]->source();
        if( met::buildMETSum("FinalClus", met_Calo, clsource).isFailure() ) {
          ATH_MSG_WARNING("Building MET FinalClus sum failed.");
        }
        
        //fills MET calo histos
        m_MET_Calo->Fill((*met_Calo)["FinalClus"]->met()/1000., weight);
        m_MET_Calo_x->Fill((*met_Calo)["FinalClus"]->mpx()/1000., weight);
        m_MET_Calo_y->Fill((*met_Calo)["FinalClus"]->mpy()/1000., weight);
        m_MET_Calo_phi->Fill((*met_Calo)["FinalClus"]->phi(), weight);
        m_MET_Calo_sum->Fill((*met_Calo)["FinalClus"]->sumet()/1000., weight);
      }
    }

   return StatusCode::SUCCESS;
  }
  
  // Proc Hists
  StatusCode PhysValMET::procHistograms()
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");
  
    //loop over jet types
    for (const auto& jet_type : m_types){
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET["MET_Rebuilt_"+jet_type]).size(); ++i) {
        //Term hists
        (m_MET["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_x["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_y["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_phi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_sum["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Diff["MET_Rebuilt_"+jet_type]).size(); ++i) {
        //Diff hists
        (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }

      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).size(); ++i) {
        //Get for CorFinalTrk
        (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).size(); ++i) {
        //get for CorrFinalClus
        (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Significance["MET_Rebuilt_"+jet_type]).size(); ++i) {
        //For Significance
        (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Resolution["MET_Rebuilt_"+jet_type]).size(); ++i) { 
        //for Resolution
      	(m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_dPhi["MET_Reference_"+jet_type]).size(); ++i) {
        //for dPhi
        (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      int nBins = (m_MET["MET_Rebuilt_"+jet_type]).at(7)->GetNbinsX();
      for(int i=1;i<=nBins;i++){
        //For MET Cumu
        double err;
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(0)->SetBinContent(i, (m_MET["MET_Rebuilt_"+jet_type]).at(8)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(0)->SetBinError(i, err);
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(1)->SetBinContent(i, (m_MET["MET_Rebuilt_"+jet_type]).at(7)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(1)->SetBinError(i, err);
      }
      //For MET Cumu
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Cumu["MET_Reference_"+jet_type]).size(); ++i) {
        m_MET_Cumu["MET_Rebuilt_"+jet_type].at(i)->Scale(1./(m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(i)->GetBinContent(1));
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
    if( mu->pt()<2.5e3 || mu->pt()/cosh(mu->eta())<4e3 ) return false;
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
    if( !(ph->author()&20) || fabs(ph->eta())>2.47 || ph->pt()<10e3 ) return false;
    return static_cast<bool>(m_photonSelIsEMTool->accept(ph));
  }
  //Tau selection
  bool PhysValMET::Accept(const xAOD::TauJet* tau)
  { return static_cast<bool> (m_tauSelTool->accept( *tau )); }

  /////////////////////////////////////////////////////////////////// 
  // Protected methods: 
  /////////////////////////////////////////////////////////////////// 

  /////////////////////////////////////////////////////////////////// 
  // Const methods: 
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////// 
  // Non-const methods: 
  /////////////////////////////////////////////////////////////////// 

}

//  LocalWords:  str
 
