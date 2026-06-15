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
namespace MissingEtDQA 
{

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
    m_names["Muons"] = "Muon term";
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
    m_terms.emplace_back("Muons");
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
    
    return StatusCode::SUCCESS;
  }
  
  //Book histograms
  StatusCode PhysValMET::bookHistograms()
  { 
    ATH_MSG_INFO ("Booking hists " << name() << "...");
      
    // Physics validation plots are level 10

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

    if (m_detailLevel >= 10) 
    {

      //loop through jet types
      for (const auto& jet_type : m_types)
      {
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
        corrClus_names.emplace_back("Muons");
        corrClus_names.emplace_back("RefJet");
        corrClus_names.emplace_back("SoftClus");
        
        corrTrk_names.emplace_back("RefEle");
        corrTrk_names.emplace_back("RefGamma");
        corrTrk_names.emplace_back("RefTau");
        corrTrk_names.emplace_back("Muons");
        corrTrk_names.emplace_back("RefJet");
        corrTrk_names.emplace_back("PVSoftTrk");

        sum_names.emplace_back("RefEle");
        sum_names.emplace_back("RefGamma");
        sum_names.emplace_back("RefTau");
        sum_names.emplace_back("Muons");
        sum_names.emplace_back("RefJet");

        //Create and Register histograms for and Rebuilt
        std::vector <std::string> met_type = {"MET_Rebuilt_"};
        ATH_MSG_INFO("****STARTING****");
        //loop for rebuilt
        for (const auto& type : met_type)
        {
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

          //Create histograms
          for(const auto& term : m_terms) 
          {
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

          //Register histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_sum[name_met].at(i),m_dir_met[i],all));
          }
 
          //Create cumulative hists
          name_sub = name_met + "/Cumulative";
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalClus").c_str(), (name_met + " CST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
          v_MET_Cumu.push_back( new  TH1D((name_met + "_Cumulative_FinalTrk").c_str(), (name_met + " TST MET cumulative; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi) );
        
          m_MET_Cumu[name_met] = v_MET_Cumu;
        
          //Regiser cumulative hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Cumu.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_Cumu[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          //create Residual histograms        
          name_sub = name_met + "/Residuals";
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_x").c_str(), ("x-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalClus_y").c_str(), ("y-Residual of CST MET in " + name_met + "; #Delta(E_{T,CST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_x").c_str(), ("x-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{x} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
          v_MET_Resolution.push_back(  new TH1D((name_met + "_Resolution_FinalTrk_y").c_str(), ("y-Residual of TST MET in " + name_met + "; #Delta(E_{T,TST}^{miss}, E_{T,truth}^{miss})_{y} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy) );
        
          m_MET_Resolution[name_met] = v_MET_Resolution;
        
          //register Residual histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Resolution.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_Resolution[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //create Significance hists        
          name_sub = name_met + "/Significance";
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalClus").c_str(), ("MET / sqrt(sumet) for " + name_met + " CST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
          v_MET_Significance.push_back(  new TH1D((name_met + "_Significance_FinalTrk").c_str(), ("MET / sqrt(sumet) for " + name_met + " TST; MET/#sqrt{SET} [#sqrt{GeV}]; Entries / 0.25 #sqrt{GeV}").c_str(), nbinp, 0., 25.) );
        
          m_MET_Significance[name_met] = v_MET_Significance;
        
          //Register Significance hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Significance.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_Significance[name_met].at(i),"MET/" + name_sub + "/",all));
          }
 
          //Create dPhi hists        
          name_sub = name_met + "/dPhi";
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalClus").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " CST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalClus").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " CST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalClus").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " CST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs leading jet for " + name_met + " TST; #Delta#Phi(leadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_subleadJetMET_FinalTrk").c_str(), ("MET deltaPhi vs subleading jet for " + name_met + " TST; #Delta#Phi(subleadJet, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
          v_MET_dPhi.push_back(  new TH1D((name_met + "_dPhi_leadLepMET_FinalTrk").c_str(), ("MET deltaPhi vs leading lepton for " + name_met + " TST; #Delta#Phi(leadLep, MET); Entries / 0.05").c_str(), nbinphi, 0., binphi) );
        
          m_MET_dPhi[name_met] = v_MET_dPhi;
        
          //Register dPhi hists
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_dPhi.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_dPhi[name_met].at(i),"MET/" + name_sub + "/",all));
          }
         
          //Create Correlation hists
          name_sub = name_met + "/Correlations";

          v_MET_CorrFinalClus.reserve(corrClus_names.size());

          for(const auto& it : corrClus_names) 
          {
            v_MET_CorrFinalClus.push_back( new  TH2D((name_met + "_" + it + "_FinalClus").c_str(), (name_met + " " + m_names[it] + " vs. CST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,CST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }
          v_MET_CorrFinalTrk.reserve(corrTrk_names.size());

          for(const auto& it : corrTrk_names) 
          {
            v_MET_CorrFinalTrk.push_back( new  TH2D((name_met + "_" + it + "_FinalTrk").c_str(), (name_met + " " + m_names[it] + " vs. TST MET; E_{T," + it + "}^{miss} [GeV]; E_{T,TST}^{miss} [GeV]; Entries").c_str(), nbinp, 0., suptmi, nbinp, 0., suptmi) );
          }

          m_MET_CorrFinalClus[name_met] = v_MET_CorrFinalClus;
          m_MET_CorrFinalTrk[name_met] = v_MET_CorrFinalTrk;

          //Register Correlation hists
          for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalTrk.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_CorrFinalTrk[name_met].at(i),"MET/" + name_sub + "/",all));
          }
          for(std::vector<TH2D*>::size_type i = 0; i < v_MET_CorrFinalClus.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_CorrFinalClus[name_met].at(i),"MET/" + name_sub + "/",all));
          }

          m_dir_met.clear();
         
          //Create Diff histograms
          for(const auto& it : sum_names) 
          {
            v_MET_Diff.push_back( new  TH1D((name_met + "_Diff_" + it).c_str(), ("MET_Diff " + m_names[it] + " in " + name_met +"; E_{T}^{miss} - #Sigma p_{T} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150));
            v_MET_Diff_x.push_back( new  TH1D((name_met + "_Diff_" + it +"_x").c_str(), ("MET_Diff x " + m_names[it] + " in " + name_met +"; E_{x}^{miss} - #Sigma p_{x} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
            v_MET_Diff_y.push_back( new  TH1D((name_met + "_Diff_" + it +"_y").c_str(), ("MET_Diff y " + m_names[it] + " in " + name_met +"; E_{y}^{miss} - #Sigma p_{y} [GeV]; Entries / 3 GeV").c_str(), nbinpxy, -150, 150) );
            v_MET_Diff_phi.push_back( new  TH1D((name_met + "_Diff_" + it +"_phi").c_str(), ("MET_Diff phi " + m_names[it] + " in " + name_met +"; #Delta#Phi(E_{T}^{miss},#Sigma p_{T}); Entries / 0.1").c_str(), nbinphi,-binphi,binphi) );
            v_MET_Diff_sum.push_back( new  TH1D((name_met + "_Diff_" + it +"_sum").c_str(), ("MET_Diff sumet " + m_names[it] + " in " + name_met +"; E_{T}^{sum} - #Sigma |p_{T}| [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -250, 250) );
            m_dir_met.push_back("MET/" + name_met + "/Differences/" + it + "/");
          }
        
          m_MET_Diff[name_met] = v_MET_Diff;
          m_MET_Diff_x[name_met] = v_MET_Diff_x;
          m_MET_Diff_y[name_met] = v_MET_Diff_y;
          m_MET_Diff_phi[name_met] = v_MET_Diff_phi;
          m_MET_Diff_sum[name_met] = v_MET_Diff_sum;
        
          //Register Diff histograms
          for(std::vector<TH1D*>::size_type i = 0; i < v_MET_Diff.size(); ++i) 
          {
            ATH_CHECK(regHist(m_MET_Diff[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_x[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_y[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_phi[name_met].at(i),m_dir_met[i],all));
            ATH_CHECK(regHist(m_MET_Diff_sum[name_met].at(i),m_dir_met[i],all));
          }
        // End of loop
        }
      }  
      //-------------------------------------------------------------------------------------
      // Now MET_Track (only built if METRef is too)
        
      //-------------------------------------------------------------------------------------
      //Now MET_Calo
  
      //variables
      std::string name_met = "MET_Calo";
      std::string dir = "MET/" + name_met + "/";

      //Create an register Calo hists
      ATH_CHECK(regHist(m_MET_Calo = new  TH1D("Calo", (name_met + " " + m_names["Calo"] + "; E_{T}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinp, 0., suptmi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_x = new  TH1D("Calo_x", (name_met + " " + m_names["Calo"] + " x; E_{x}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_y = new  TH1D("Calo_y", (name_met + " " + m_names["Calo"] + " y; E_{y}^{miss} [GeV]; Entries / 5 GeV").c_str(), nbinpxy, -suptmixy, suptmixy), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_phi = new  TH1D("Calo_phi", (name_met + " " + m_names["Calo"] + " phi;  #Phi; Entries / 0.1").c_str(), nbinphi,-binphi,binphi), dir, all));
      ATH_CHECK(regHist(m_MET_Calo_sum = new  TH1D("Calo_sum", (name_met + " " + m_names["Calo"] + " sum; E_{T}^{sum} [GeV]; Entries / 25 GeV").c_str(), nbinE, lowET, suET), dir, all));
    }
  
    return StatusCode::SUCCESS;      
  }

  //Fill Histograms
  StatusCode PhysValMET::fillHistograms(const EventContext& /*ctx*/)
  {
    ATH_MSG_DEBUG ("Filling hists " << name() << "...");

    //Beamspot weight
    const xAOD::EventInfo* eventInfo(nullptr);
    ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));

    float weight = eventInfo->beamSpotWeight();

    //Retrieve MET Truth
    const xAOD::MissingETContainer* met_Truth = nullptr;
    if(m_doTruth) 
    {
      ATH_CHECK( evtStore()->retrieve(met_Truth,"MET_Truth") );
      if (!met_Truth) 
      {
        ATH_MSG_ERROR ( "Failed to retrieve MET_Truth. Exiting." );
        return StatusCode::FAILURE;
      }
    }
     ATH_MSG_INFO("Physics objects");
    //Physics Objects

    //Muons
    const xAOD::MuonContainer* muons = nullptr;
    ATH_CHECK( evtStore()->retrieve(muons,m_muonColl) );
    if (!muons) 
    {
      ATH_MSG_ERROR ( "Failed to retrieve Muon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<MuonContainer> metMuons(SG::VIEW_ELEMENTS);
    bool is_muon = 0;
    for(const auto mu : *muons) 
    {
      if(Accept(mu)) 
      {
        metMuons.push_back(mu);
        is_muon = 1;
      }
    }

    //Electrons
    const xAOD::ElectronContainer* electrons = nullptr;
    ATH_CHECK( evtStore()->retrieve(electrons,m_eleColl) );
    if (!electrons) 
    {
      ATH_MSG_ERROR ( "Failed to retrieve Electron container. Exiting." );
      return StatusCode::FAILURE;
    }
   ConstDataVector<ElectronContainer> metElectrons(SG::VIEW_ELEMENTS);
   bool is_electron = 0;
   for(const auto el : *electrons) 
   {
     if(Accept(el)) 
     {
       metElectrons.push_back(el);
       is_electron = 1;
     }
   }

    //Photons
    const xAOD::PhotonContainer* photons = nullptr;
    ATH_CHECK( evtStore()->retrieve(photons,m_gammaColl) );
    if (!electrons)
    {
      ATH_MSG_ERROR ( "Failed to retrieve Photon container. Exiting." );
      return StatusCode::FAILURE;
    }
    ConstDataVector<PhotonContainer> metPhotons(SG::VIEW_ELEMENTS);
    for(const auto ph : *photons) 
    {
      if(Accept(ph)) 
      {
        metPhotons.push_back(ph);
      }
    }

    //Tau Jets
    const TauJetContainer* taus = nullptr;
    ATH_CHECK( evtStore()->retrieve(taus, m_tauColl) );
    if(!taus) 
    {
      ATH_MSG_ERROR("Failed to retrieve TauJet container: " << m_tauColl);
      return StatusCode::SUCCESS;
    }
    ConstDataVector<TauJetContainer> metTaus(SG::VIEW_ELEMENTS);
    for(const auto tau : *taus) 
    {
      if(Accept(tau)) 
      {
        metTaus.push_back(tau);
      }
    }
     ATH_MSG_INFO("OR");
//////////Overlap removal///////
    // Overlap removal

    ConstDataVector<PhotonContainer>::iterator pho_itr;
    ConstDataVector<ElectronContainer>::iterator ele_itr;
    ConstDataVector<TauJetContainer>::iterator taujet_itr;
    ConstDataVector<MuonContainer>::iterator mu_itr;
    ConstDataVector<JetContainer>::iterator jetc_itr;
 
    //Photons OR
    bool is_photon = 0;
    ConstDataVector<PhotonContainer> metPhotonsOR(SG::VIEW_ELEMENTS);
    for(pho_itr = metPhotons.begin(); pho_itr != metPhotons.end(); ++pho_itr ) 
    {
      TLorentzVector phtlv = (*pho_itr)->p4();
      bool passOR = 1;
      for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) 
      {
        if(phtlv.DeltaR((*ele_itr)->p4()) < 0.2) 
        {
          passOR = 0;
          break;
        }
      }
      if(passOR)
      {
        metPhotonsOR.push_back(*pho_itr);
        is_photon = 1;
      }
    }
     ATH_MSG_INFO("Tau OR");
    //TauJets OR
    ConstDataVector<TauJetContainer> metTausOR(SG::VIEW_ELEMENTS);
    bool is_tau = 0;
    for(taujet_itr = metTaus.begin(); taujet_itr != metTaus.end(); ++taujet_itr ) 
    {
      TLorentzVector tautlv = (*taujet_itr)->p4();
      bool passOR = 1;
      for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) 
      {
        if(tautlv.DeltaR((*ele_itr)->p4()) < 0.2) 
        {
          passOR = 0;
          break;
        }
      }
      for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr) 
      {
        if(tautlv.DeltaR((*pho_itr)->p4()) < 0.2) 
        {
          passOR = 0;
          break;
        }
      }
      if(passOR)
      {
        metTausOR.push_back(*taujet_itr);
        is_tau = 1;
      }
    }

    //Sum up the pT's of the objects
    
    //electron
    TLorentzVector el_tlv;
    double sum_el = 0;
    for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr ) 
    {
      el_tlv += (*ele_itr)->p4();
      sum_el += (*ele_itr)->pt();
    }
    
    //muon
    TLorentzVector mu_tlv;
    double sum_mu = 0;
    for(mu_itr = metMuons.begin(); mu_itr != metMuons.end(); ++mu_itr ) 
    {
      mu_tlv += (*mu_itr)->p4();
      sum_mu += (*mu_itr)->pt();
    }

    //Tau
    TLorentzVector tau_tlv;
    double sum_tau = 0;
    for(taujet_itr = metTausOR.begin(); taujet_itr != metTausOR.end(); ++taujet_itr ) 
    {
      tau_tlv += (*taujet_itr)->p4();
      sum_tau += (*taujet_itr)->pt();
    }
  
    //photon
    TLorentzVector photon_tlv;
    double sum_photon = 0;
    for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr ) 
    {
      photon_tlv += (*pho_itr)->p4();
      sum_photon += (*pho_itr)->pt();
    }
     ATH_MSG_INFO("JVT and OR for jets");
    //JVT and OR on jets
    for (const auto& jet_type : m_types)
    {
      ToolHandle<IJetUpdateJvt>* jvtTool(nullptr);
      double JvtCut = 0.59;
      //Get jvt cut and tool
      if (jet_type == "AntiKt4EMPFlow") //EMPFlow
      {
        JvtCut = 0.2;
        jvtTool = &m_jvtToolPFlow;
      }
      else if (jet_type == "AntiKt4EMTopo") //EMTopo
      {
        jvtTool = &m_jvtToolEM;
      }
    
      if(jvtTool == nullptr)
      {
        ATH_MSG_ERROR("Unrecognized jet container: " << jet_type << "Jets");
        return StatusCode::FAILURE;
      }

      // Retrieve Jets
      std::string name_jet = jet_type + "Jets";
      const xAOD::JetContainer* jets = nullptr;
      ATH_CHECK( evtStore()->retrieve(jets,name_jet) );
      if (!jets) 
      {
        ATH_MSG_ERROR ( "Failed to retrieve Jet container: " << name_jet << ". Exiting." );
        return StatusCode::FAILURE;
      }
      SG::Decorator<float> NewJvtDec("NewJvt");
      for(auto jet : *jets) //for Jets assing JVT decoration
      {
        float newjvt = (*jvtTool)->updateJvt(*jet); 
        NewJvtDec(*jet) = newjvt;
      }
      ConstDataVector<JetContainer> metJets(SG::VIEW_ELEMENTS);
      for(const auto jet : *jets) //for jets assign jets
      { 
        metJets.push_back(jet);
      }

      //Overlap Removal for jets
      ConstDataVector<JetContainer> metJetsOR(SG::VIEW_ELEMENTS);
      bool is_jet = 0;
      for(jetc_itr = metJets.begin(); jetc_itr != metJets.end(); ++jetc_itr ) 
      {
        TLorentzVector jettlv = (*jetc_itr)->p4();
        bool passOR = 1;
        for(ele_itr = metElectrons.begin(); ele_itr != metElectrons.end(); ++ele_itr) 
        {
          if(jettlv.DeltaR((*ele_itr)->p4()) < 0.2) 
          {
            passOR = 0;
            break;
          }
        }
        for(pho_itr = metPhotonsOR.begin(); pho_itr != metPhotonsOR.end(); ++pho_itr) 
        {
          if(jettlv.DeltaR((*pho_itr)->p4()) < 0.2) 
          {
            passOR = 0;
            break;
          }
        }
        for(taujet_itr = metTausOR.begin(); taujet_itr != metTausOR.end(); ++taujet_itr) 
        {
          if(jettlv.DeltaR((*taujet_itr)->p4()) < 0.2) 
          {
            passOR = 0;
            break;
          }
        }
        if(passOR)
        {
          metJetsOR.push_back(*jetc_itr);
          is_jet = 1;
        }
      }

      TLorentzVector jet_tlv; //Jet Transverse Lorentz Vector
      double sum_jet = 0;
      //loop over Jets that passed OR
      for(jetc_itr = metJetsOR.begin(); jetc_itr != metJetsOR.end(); ++jetc_itr ) 
      {
        jet_tlv += (*jetc_itr)->p4();
        sum_jet += (*jetc_itr)->pt();
      }

      //Prepare Rebuilding MET
      ATH_MSG_INFO( "  Rebuilding MET_" << jet_type );
      MissingETContainer* met_Reb = new MissingETContainer(); //Define MET Container
      if( evtStore()->record(met_Reb,("MET_Rebuilt_"+jet_type).c_str()).isFailure() ) 
      {
        ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Rebuilt_" << jet_type);
        return StatusCode::FAILURE;
      }
      MissingETAuxContainer* met_RebAux = new MissingETAuxContainer(); //Define MET Aux container
      if( evtStore()->record(met_RebAux,("MET_Rebuilt_"+jet_type+"Aux").c_str()).isFailure() ) 
      {
        ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Rebuilt_" << jet_type);
        return StatusCode::FAILURE;
      }

      met_Reb->setStore(met_RebAux); //MET Reb container

      //define map and core name
      m_mapname = "METAssoc_"+jet_type;
      m_corename = "MET_Core_"+jet_type;
      const MissingETAssociationMap* metMap = nullptr;
      //check is can retrieve Map and Container
      if( evtStore()->retrieve(metMap, m_mapname).isFailure() ) 
      {
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

       ATH_MSG_INFO("building terms");
      //See if we ca build terms

      // Electrons
      if( (*m_metmaker)->rebuildMET("RefEle", xAOD::Type::Electron, met_Reb, metElectrons.asDataVector(), metHelper).isFailure() ) 
      {
        ATH_MSG_WARNING("Failed to build electron term.");
      }
      // Photons
      if( (*m_metmaker)->rebuildMET("RefGamma", xAOD::Type::Photon, met_Reb, metPhotons.asDataVector(), metHelper).isFailure() ) 
      {
        ATH_MSG_WARNING("Failed to build photon term.");
      }
      // Taus
      if( (*m_metmaker)->rebuildMET("RefTau", xAOD::Type::Tau, met_Reb,metTaus.asDataVector(),metHelper).isFailure() )
      {
        ATH_MSG_WARNING("Failed to build tau term.");
      }
      // Muons
      if( (*m_metmaker)->rebuildMET("Muons", xAOD::Type::Muon, met_Reb, metMuons.asDataVector(), metHelper).isFailure() ) 
      {
        ATH_MSG_WARNING("Failed to build muon term.");
      }
      // Jets
      if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_Reb, jets, coreMet, metHelper, true).isFailure() ) 
      {
        ATH_MSG_WARNING("Failed to build jet and soft terms.");
      }

      MissingETBase::Types::bitmask_t trksource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track);
      if((*met_Reb)["PVSoftTrk"]) trksource = (*met_Reb)["PVSoftTrk"]->source();
      if( met::buildMETSum("FinalTrk", met_Reb, trksource).isFailure() )
      {
        ATH_MSG_WARNING("Building MET FinalTrk sum failed.");
      }
      MissingETBase::Types::bitmask_t clsource;
      if (jet_type == "AntiKt4EMTopo") clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::EMTopo);
      else clsource = static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::UnknownSignal);
      std::cout<<"___SoftClus___"<<std::endl;
      if((*met_Reb)["SoftClus"]) clsource = (*met_Reb)["SoftClus"]->source();
      if( met::buildMETSum("FinalClus", met_Reb, clsource).isFailure() ) 
      {
        ATH_MSG_WARNING("Building MET FinalClus sum failed.");
      }
       ATH_MSG_INFO("Particle test");

      ///////
      //Testing Particles
      //doing METMaker particles
      std::string str_ele = "RefEle"; 
      std::string str_gam = "RefGamma";
      std::string str_tau = "RefTau";
      std::string str_mu = "Muons";
      std::string str_jet = "RefJet";
      // This will be the output MET.
      auto met_MetMaker = std::make_unique<xAOD::MissingETContainer>();
      auto aux = std::make_unique<xAOD::MissingETAuxContainer>();
      met_MetMaker->setStore(aux.get());

      // Build the hard terms. The string argument is arbitrary, it's the name you're giving to the term for later lookup.
      ATH_CHECK((*m_metmaker)->rebuildMET(str_ele, xAOD::Type::Electron, met_MetMaker.get(), metElectrons.asDataVector(), metHelper));
      ATH_CHECK((*m_metmaker)->rebuildMET(str_gam, xAOD::Type::Photon, met_MetMaker.get(), metPhotons.asDataVector(), metHelper));
      ATH_CHECK((*m_metmaker)->rebuildMET(str_tau, xAOD::Type::Tau, met_MetMaker.get(), metTaus.asDataVector(),metHelper));
      ATH_CHECK((*m_metmaker)->rebuildMET(str_mu, xAOD::Type::Muon, met_MetMaker.get(), metMuons.asDataVector(), metHelper));
      ATH_CHECK((*m_metmaker)->rebuildJetMET(str_jet, "SoftClus", "PVSoftTrk", met_MetMaker.get(), jets, coreMet, metHelper, true));

      for (const xAOD::MissingET* met : *met_MetMaker) {
        ATH_MSG_INFO("_W_W_W_W_W_W_W_W_W_W_W_W_W_W");
        ATH_MSG_INFO( met->name());
      }

      // If the specific object interfaces are needed
      std::vector<const xAOD::Electron*> el_elems = met::getMETElements<xAOD::Electron>(*(*met_MetMaker)[str_ele]);
      std::vector<const xAOD::Photon*> ph_elems = met::getMETElements<xAOD::Photon>(*(*met_MetMaker)[str_gam]);
      std::vector<const xAOD::TauJet*> ta_elems = met::getMETElements<xAOD::TauJet>(*(*met_MetMaker)[str_tau]);
      std::vector<const xAOD::Muon*> mu_elems = met::getMETElements<xAOD::Muon>(*(*met_MetMaker)[str_mu]);
      std::vector<const xAOD::Jet*> jet_elems = met::getMETElements<xAOD::Jet>(*(*met_MetMaker)[str_jet]);
/*
      std::vector<const xAOD::Electron*> el_elems = met::getMETElements<xAOD::Electron>(*(*met_MetMaker)[str_ele]);
      std::vector<const xAOD::Photon*> ph_elems = met::getMETElements<xAOD::Photon>(*(*met_MetMaker)[str_gam]);
      std::vector<const xAOD::Tau*> ta_elems = met::getMETElements<xAOD::Tau>(*(*met_MetMaker)[str_tau]);
      >std::vector<const xAOD::Muon*> mu_elems = met::getMETElements<xAOD::Muon>(*(*met_MetMaker)[str_mu]);
      std::vector<const xAOD::JetContainer*> jet_elems = met::getMETElements<xAOD::JetContainer>(*(*met_MetMaker)[str_jet]);
*/
      ATH_MSG_INFO("Comparing particle numbers" );
      ATH_MSG_INFO("---METMaker---" );
      for(const auto p : el_elems){
        ATH_MSG_INFO("Electron " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : ph_elems){
        ATH_MSG_INFO("Photon " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : ta_elems){
        ATH_MSG_INFO("Tau " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : mu_elems){
        ATH_MSG_INFO("Muon " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : jet_elems){
        ATH_MSG_INFO("Jet " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      ATH_MSG_INFO("---MET PhysVal Default---" );
      for(const auto p : metElectrons){
        ATH_MSG_INFO("Electron " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : metPhotonsOR){
        ATH_MSG_INFO("Photon " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : metTausOR){
        ATH_MSG_INFO("Tau " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : metMuons){
        ATH_MSG_INFO("Muon " << p->pt() << " " << p->eta() << " " << p->phi());
      }
      for(const auto p : metJetsOR){
        if(Accept(p, JvtCut, jvtTool)){
          ATH_MSG_INFO("Jet " << p->pt() << " " << p->eta() << " " << p->phi());
        }
      }
      //////

      std::cout<<"___Fill MET Reb___"<<std::endl;
      // Fill MET_Reb hists
      for(const auto it : *met_Reb) 
      {
        std::string name = it->name();
        std::cout<< name <<std::endl;
        if(name == "RefEle")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefGamma")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefTau")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "Muons")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "RefJet")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "SoftClus")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "PVSoftTrk")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(6)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "FinalTrk")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(7)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
        if(name == "FinalClus")
        {
          (m_MET["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met_Reb)[name.c_str()]->met()/1000., weight);
          (m_MET_x["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met_Reb)[name.c_str()]->mpx()/1000., weight);
          (m_MET_y["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met_Reb)[name.c_str()]->mpy()/1000., weight);
          (m_MET_phi["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met_Reb)[name.c_str()]->phi(), weight);
          (m_MET_sum["MET_Rebuilt_"+jet_type]).at(8)->Fill((*met_Reb)[name.c_str()]->sumet()/1000., weight);
        }
      }

      //Fill MET Angles
      ATH_MSG_INFO( "  MET_Angles :" );

      //define vars
      double leadPt = 0., subleadPt = 0., leadPhi = 0., subleadPhi = 0.;

      //for Jets find leading and subleading jet
      for (auto jet_itr = jets->begin(); jet_itr != jets->end(); ++jet_itr) 
      {
        if ((*jet_itr)->pt() > leadPt && Accept(*jet_itr,JvtCut,jvtTool)) 
        {
          subleadPt = leadPt;
          subleadPhi = leadPhi;
          leadPt = (*jet_itr)->pt();
          leadPhi = (*jet_itr)->phi();
        }
        else if ((*jet_itr)->pt() > subleadPt && Accept(*jet_itr,JvtCut,jvtTool)) 
        {
          subleadPt = (*jet_itr)->pt();
          subleadPhi = (*jet_itr)->phi();
        }
      }

      //Fill dPhi for Met Reb
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(0)->Fill( -remainder( leadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(1)->Fill( -remainder( subleadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(3)->Fill( -remainder( leadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(4)->Fill( -remainder( subleadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );
  
      leadPt = 0.; leadPhi = 0.;

      xAOD::MuonContainer::const_iterator muon_itr = muons->begin();
      xAOD::MuonContainer::const_iterator muon_end = muons->end();

      for( ; muon_itr != muon_end; ++muon_itr ) 
      {
        if((*muon_itr)->pt() > leadPt) 
        {
          leadPt = (*muon_itr)->pt();
          leadPhi = (*muon_itr)->phi();
        }
      }

      xAOD::ElectronContainer::const_iterator electron_itr = electrons->begin();
      xAOD::ElectronContainer::const_iterator electron_end = electrons->end();

      for( ; electron_itr != electron_end; ++electron_itr ) 
      {
        if((*electron_itr)->pt() > leadPt) 
        {
          leadPt = (*electron_itr)->pt();
          leadPhi = (*electron_itr)->phi();
        }
      }

      //Fill dPhi for MET Rebuilt Final Clus and Final trk
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(2)->Fill( -remainder( leadPhi - (*met_Reb)["FinalClus"]->phi(), 2*M_PI ), weight );
      (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(5)->Fill( -remainder( leadPhi - (*met_Reb)["FinalTrk"]->phi(), 2*M_PI ), weight );

      //Rebuilt

      for(const auto it : *met_Reb) 
      {
        std::string name = it->name();
        if(name == "RefEle")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefGamma")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefTau")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(2)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
        if(name == "Muons")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(3)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
        if(name == "RefJet")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(4)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
        if(name == "PVSoftTrk")
        {
          (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalTrk"]->met()/1000., weight);
        }
        if(name == "SoftClus")
        {
          (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(5)->Fill((*met_Reb)[name.c_str()]->met()/1000.,(*met_Reb)["FinalClus"]->met()/1000., weight);
        }
      }

      // Fill Resolution
      if(m_doTruth)
      {
        ATH_MSG_INFO( "  Resolution:" );
        //Fill Rebuilt plots
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(0)->Fill(((*met_Reb)["FinalClus"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(1)->Fill(((*met_Reb)["FinalClus"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(2)->Fill(((*met_Reb)["FinalTrk"]->mpx()-(*met_Truth)["NonInt"]->mpx())/1000., weight);
        (m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(3)->Fill(((*met_Reb)["FinalTrk"]->mpy()-(*met_Truth)["NonInt"]->mpy())/1000., weight);
      }

      //Fill MET significance
      //fill Reb
      if( (*met_Reb)["FinalClus"]->sumet() != 0) (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(0)->Fill((*met_Reb)["FinalClus"]->met()/sqrt((*met_Reb)["FinalClus"]->sumet()*1000.), weight);
      if( (*met_Reb)["FinalTrk"]->sumet() != 0) (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(1)->Fill((*met_Reb)["FinalTrk"]->met()/sqrt((*met_Reb)["FinalTrk"]->sumet()*1000.), weight);

      TLorentzVector target_tlv;

      // For rebuilt MET add only jets with pT>20e3 and JVT cut
      TLorentzVector jetReb_tlv;
      double sum_jetReb = 0;
      for(const auto jet : metJetsOR) 
      {
        if(Accept(jet, JvtCut, jvtTool)) 
        {
          jetReb_tlv += jet->p4();
          sum_jetReb += jet->pt();
        }
      }

      for(const auto it : *met_Reb) 
      {
        //Fill MET Dif for Reb
        if(it->name() == "RefEle")
        {
          if(is_electron or (it->sumet() > 0))
          {
            target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
            (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Pt() - el_tlv.Pt())/1000., weight);
            (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Px() - el_tlv.Px())/1000., weight);
            (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(0)->Fill((target_tlv.Py() - el_tlv.Py())/1000., weight);
            (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(0)->Fill(el_tlv.DeltaPhi(target_tlv), weight);
            (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(0)->Fill((it->sumet() - sum_el)/1000., weight);
          }
        }
        if(it->name() == "RefGamma")
        {
          if(is_photon or (it->sumet() > 0))
          {
            target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
            (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Pt() - photon_tlv.Pt())/1000., weight);
            (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Px() - photon_tlv.Px())/1000., weight);
            (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(1)->Fill((target_tlv.Py() - photon_tlv.Py())/1000., weight);
            (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(1)->Fill(photon_tlv.DeltaPhi(target_tlv), weight);
            (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(1)->Fill((it->sumet() - sum_photon)/1000., weight);
          }
        }
        if(it->name() == "RefTau")
        {
          if(is_tau or (it->sumet() > 0))
          {
            target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
            (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Pt() - tau_tlv.Pt())/1000., weight);
            (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Px() - tau_tlv.Px())/1000., weight);
            (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(2)->Fill((target_tlv.Py() - tau_tlv.Py())/1000., weight);
            (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(2)->Fill(tau_tlv.DeltaPhi(target_tlv), weight);
            (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(2)->Fill((it->sumet() - sum_tau)/1000., weight);
          }
        }
        if(it->name() == "Muons")
        {
          if(is_muon or (it->sumet() > 0))
          {
            target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
            (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Pt() - mu_tlv.Pt())/1000., weight);
            (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Px() - mu_tlv.Px())/1000., weight);
            (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(3)->Fill((target_tlv.Py() - mu_tlv.Py())/1000., weight);
            (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(3)->Fill(mu_tlv.DeltaPhi(target_tlv), weight);
            (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(3)->Fill((it->sumet() - sum_mu)/1000., weight);
          }
        }
        if(it->name() == "RefJet")
        {
          if(is_jet or (it->sumet() > 0))
          {
            target_tlv.SetPxPyPzE(-it->mpx(), -it->mpy(), 0, it->met());
            (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Pt() - jetReb_tlv.Pt())/1000., weight);
            (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Px() - jetReb_tlv.Px())/1000., weight);
            (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(4)->Fill((target_tlv.Py() - jetReb_tlv.Py())/1000., weight);
            (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(4)->Fill(jetReb_tlv.DeltaPhi(target_tlv), weight);
            (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(4)->Fill((it->sumet() - sum_jetReb)/1000., weight);
          }
        }
      }

      //EMTopo
      if(jet_type == "AntiKt4EMTopo") 
      {
        //Calo MET
        //const xAOD::JetContainer* emptyjets = 0;
        ConstDataVector<JetContainer> metJetsEmpty(SG::VIEW_ELEMENTS);
        MissingETContainer* met_Calo = new MissingETContainer();
        if( evtStore()->record(met_Calo,("MET_Calo"+jet_type).c_str()).isFailure() ) 
        {
          ATH_MSG_WARNING("Unable to record MissingETContainer: MET_Calo_" << jet_type);
          return StatusCode::FAILURE;
        }
        MissingETAuxContainer* met_CaloAux = new MissingETAuxContainer();
        if( evtStore()->record(met_CaloAux,("MET_Calo"+jet_type+"Aux").c_str()).isFailure() ) 
        {
          ATH_MSG_WARNING("Unable to record MissingETAuxContainer: MET_Calo" << jet_type);
          return StatusCode::FAILURE;
        }
        met_Calo->setStore(met_CaloAux);
        MissingETAssociationHelper metHelper(metMap);
        if( (*m_metmaker)->rebuildJetMET("RefJet", "SoftClus", "PVSoftTrk", met_Calo, metJetsEmpty.asDataVector(), coreMet, metHelper, true).isFailure() ) 
        {
          ATH_MSG_WARNING("Failed to build jet and soft terms.");
        }
        
        if((*met_Calo)["SoftClus"]) clsource = (*met_Calo)["SoftClus"]->source();
        if( met::buildMETSum("FinalClus", met_Calo, clsource).isFailure() ) 
        {
          ATH_MSG_WARNING("Building MET FinalClus sum failed.");
        }
        
        //fills MET calo
        m_MET_Calo->Fill((*met_Calo)["FinalClus"]->met()/1000., weight);
        m_MET_Calo_x->Fill((*met_Calo)["FinalClus"]->mpx()/1000., weight);
        m_MET_Calo_y->Fill((*met_Calo)["FinalClus"]->mpy()/1000., weight);
        m_MET_Calo_phi->Fill((*met_Calo)["FinalClus"]->phi(), weight);
        m_MET_Calo_sum->Fill((*met_Calo)["FinalClus"]->sumet()/1000., weight);

      }

    }

   return StatusCode::SUCCESS;
   //return StatusCode::FAILURE;
  }
  
  // Proc Hists
  StatusCode PhysValMET::procHistograms()
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");
  
    //loop over jet types
    for (const auto& jet_type : m_types)
    {
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET["MET_Rebuilt_"+jet_type]).size(); ++i) 
      {
        //Reb hists get Sum w2
        (m_MET["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_x["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_y["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_phi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_sum["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Diff["MET_Rebuilt_"+jet_type]).size(); ++i) 
      {
        (m_MET_Diff["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_x["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_y["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_phi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
        (m_MET_Diff_sum["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }

      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).size(); ++i) 
      {
        //Get for CorFinalTrk
        (m_MET_CorrFinalTrk["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH2D*>::size_type i = 0; i < (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).size(); ++i) 
      {
        //get for CorrFinalClus
        (m_MET_CorrFinalClus["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Significance["MET_Rebuilt_"+jet_type]).size(); ++i) 
      {
        //For Significance
        (m_MET_Significance["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Resolution["MET_Rebuilt_"+jet_type]).size(); ++i) 
      { 
        //for Resolution
      	(m_MET_Resolution["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_dPhi["MET_Reference_"+jet_type]).size(); ++i) 
      {
        //for dPhi
        (m_MET_dPhi["MET_Rebuilt_"+jet_type]).at(i)->Sumw2();
      }
  
      int nBins = (m_MET["MET_Rebuilt_"+jet_type]).at(7)->GetNbinsX();
      for(int i=1;i<=nBins;i++)
      {
        //For MET add errors for Cumu
        double err;
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(0)->SetBinContent(i, (m_MET["MET_Rebuilt_"+jet_type]).at(8)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(0)->SetBinError(i, err);
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(1)->SetBinContent(i, (m_MET["MET_Rebuilt_"+jet_type]).at(7)->IntegralAndError(i,nBins+1,err));
        (m_MET_Cumu["MET_Rebuilt_"+jet_type]).at(1)->SetBinError(i, err);
      }
      //For MET Cumu
      for(std::vector<TH1D*>::size_type i = 0; i < (m_MET_Cumu["MET_Reference_"+jet_type]).size(); ++i) 
      {
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
  //Jet selection
  bool PhysValMET::Accept(const xAOD::Jet* jet, double JvtCut, ToolHandle<IJetUpdateJvt>* jvtTool)
  {
    if( jet->pt()<20e3 || jvtTool == nullptr) return false;
    return (fabs(jet->eta()) > 2.4 || jet->pt() > 60e3 || (*jvtTool)->updateJvt(*jet) > JvtCut);
  }

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
 
