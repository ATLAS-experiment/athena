/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// EgammaPhysValMonitoringTool.cxx 
// Implementation file for class EgammaPhysValMonitoringTool
// Author:
/////////////////////////////////////////////////////////////////// 

// PhysVal includes
#include "EgammaPhysValMonitoringTool.h"

// STL includes
#include <vector>

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "xAODEgamma/EgammaEnums.h"
#include "xAODEgamma/EgammaDefs.h" 
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/PhotonxAODHelpers.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include <EgammaAnalysisHelpers/PhotonHelpers.h>

#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "MCTruthClassifier/MCTruthClassifier.h"

#include "StoreGate/ReadHandle.h"
#include "AthContainers/ConstAccessor.h"

#include "TruthUtils/HepMCHelpers.h"

using CLHEP::GeV;
using namespace std;
using namespace MCTruthPartClassifier;

namespace EgammaPhysValMonitoring {

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////

EgammaPhysValMonitoringTool::EgammaPhysValMonitoringTool( const std::string& type, 
							  const std::string& name, 
							  const IInterface* parent ):
  ManagedMonitorToolBase( type, name, parent ),
  m_oElectronValidationPlots(nullptr, "Electron/"),
  m_oPhotonValidationPlots(nullptr, "Photon/"),
  m_oLRTElectronValidationPlots(nullptr, "LRTElectron/"),
  m_acc_electronLLH_VeryLooseNoPix("DFCommonElectronsLHVeryLooseNoPix"),
  m_acc_electronLLH_LooseNoPix("DFCommonElectronsLHLooseNoPix"),
  m_acc_electronLLH_MediumNoPix("DFCommonElectronsLHMediumNoPix"),
  m_acc_electronLLH_TightNoPix("DFCommonElectronsLHTightNoPix")
{    
}

// Athena algtool's Hooks
////////////////////////////
StatusCode EgammaPhysValMonitoringTool::initialize()
{
  ATH_MSG_INFO("Initializing " << name() << "...");
  ATH_MSG_INFO("using OQ quality cuts: " << m_useOQQuality);
  ATH_CHECK(ManagedMonitorToolBase::initialize());
  ATH_CHECK(m_truthClassifier.retrieve()); 
  ATH_CHECK(m_Electron_VeryLooseNoPix_LLHTool.retrieve()); 
  ATH_CHECK(m_Electron_LooseNoPix_LLHTool.retrieve()); 
  ATH_CHECK(m_Electron_MediumNoPix_LLHTool.retrieve()); 
  ATH_CHECK(m_Electron_TightNoPix_LLHTool.retrieve()); 
  ATH_CHECK(m_EventInfoContainerKey.initialize());
  ATH_CHECK(m_photonContainerKey.initialize());
  ATH_CHECK(m_electronContainerKey.initialize());
  ATH_CHECK(m_lrtelectronContainerKey.initialize(!m_lrtelectronContainerKey.empty()));
  ATH_CHECK(m_electronContainerFrwdKey.initialize());
  ATH_CHECK(m_truthParticleContainerKey.initialize(m_isMC));
  ATH_CHECK(m_egammaTruthContainerKey.initialize(m_isMC));

  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::bookHistograms() 
{
  ATH_MSG_INFO ("Booking hists " << name() << "...");

  m_oElectronValidationPlots.initialize();
  std::vector<HistData> hists = m_oElectronValidationPlots.retrieveBookedHistograms();
  for (auto &hist : hists){
    ATH_MSG_DEBUG ("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }

  m_oPhotonValidationPlots.initialize();
  hists = m_oPhotonValidationPlots.retrieveBookedHistograms();
  for (auto &hist : hists){
    ATH_MSG_DEBUG ("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }

  m_oLRTElectronValidationPlots.initialize();
  hists = m_oLRTElectronValidationPlots.retrieveBookedHistograms();
  for (auto &hist : hists){
    ATH_MSG_DEBUG ("Initializing " << hist.first << " " << hist.first->GetName() << " " << hist.second << "...");
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }

  return StatusCode::SUCCESS;      
}


StatusCode EgammaPhysValMonitoringTool::fillHistograms()
{
  ATH_MSG_DEBUG("Filling hists " << name() << "...");

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_EventInfoContainerKey, ctx);
  ATH_CHECK(eventInfo.isValid());

  float weight = eventInfo->beamSpotWeight();

  if (m_isMC) {
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles =
      SG::ReadHandle<xAOD::TruthParticleContainer>(m_egammaTruthContainerKey, ctx);

    // filling truth iso (prompt) particles from egammaTruthParticles container
    // (containing only iso particles)
    // validity check is only really needed for serial running. Remove when MT
    // is only way.
    ATH_CHECK(truthParticles.isValid());

    for (const auto* const truthParticle : *truthParticles) {

      //--electrons
      if (std::abs(truthParticle->pdgId()) == 11 &&
          MC::isStable(truthParticle) && HepMC::generations(truthParticle) < 1) {
        m_oElectronValidationPlots.m_oTruthIsoPlots.fill(*truthParticle,
                                                         *eventInfo);
        m_oElectronValidationPlots.m_oTruthPromptElecPlots.fill(*truthParticle,
                                                           *eventInfo);
      } //-- end electrons

      //--photons
      if (std::abs(truthParticle->pdgId()) == 22 &&
          MC::isStable(truthParticle) && HepMC::generations(truthParticle) < 1) {
        m_oPhotonValidationPlots.m_oTruthIsoPlots.fill(*truthParticle,
                                                       *eventInfo);
        //-- filling conversions
        const xAOD::TruthParticle* tmp =
          xAOD::TruthHelpers::getTruthParticle(*truthParticle); // 20.7.0.1
        //      const xAOD::TruthParticle* tmp =
        //      xAOD::EgammaHelpers::getTruthParticle( truthParticle );
        bool isTrueConv = false;
        float trueR = -999;
        float truthEta = -999;
        if (tmp && tmp->hasDecayVtx()) {
          float x = tmp->decayVtx()->x();
          float y = tmp->decayVtx()->y();
          trueR = std::sqrt(x * x + y * y);
        }

        if (tmp != nullptr) {
          truthEta = tmp->eta();
          isTrueConv = xAOD::EgammaHelpers::isTrueConvertedPhoton(tmp); // rel20
        }

        m_oPhotonValidationPlots.convTruthR->Fill(trueR, weight);
        m_oPhotonValidationPlots.convTruthRvsEta->Fill(trueR, truthEta, weight);
        if (isTrueConv)
          m_oPhotonValidationPlots.m_oTruthIsoConvPlots.fill(*truthParticle,
                                                             *eventInfo);
        if (!isTrueConv)
          m_oPhotonValidationPlots.m_oTruthIsoUncPlots.fill(*truthParticle,
                                                            *eventInfo);

        const xAOD::Photon* recoPhoton =
          xAOD::EgammaHelpers::getRecoPhoton(truthParticle);
        if (recoPhoton) {
          m_oPhotonValidationPlots.convTruthMatchedR->Fill(trueR, weight);
          m_oPhotonValidationPlots.convTruthMatchedRvsEta->Fill(
            trueR, truthEta, weight);

          m_oPhotonValidationPlots.m_oTruthRecoPlots.fill(*truthParticle,
                                                          *eventInfo);
          if (isTrueConv) {
            m_oPhotonValidationPlots.m_oTruthRecoConvPlots.fill(*truthParticle,
                                                                *eventInfo);
          } else {
            m_oPhotonValidationPlots.m_oTruthRecoUncPlots.fill(*truthParticle,
                                                               *eventInfo);
          }
          bool val_loose = false;
          recoPhoton->passSelection(val_loose, "Loose");
          if (val_loose) {
            m_oPhotonValidationPlots.m_oTruthRecoLoosePlots.fill(*truthParticle,
                                                                 *eventInfo);
            if (isTrueConv) {
              m_oPhotonValidationPlots.m_oTruthRecoLooseConvPlots.fill(
                *truthParticle, *eventInfo);
            } else {
              m_oPhotonValidationPlots.m_oTruthRecoLooseUncPlots.fill(
                *truthParticle, *eventInfo);
            }
          } //--  end truth loose
          bool val_tight = false;
          recoPhoton->passSelection(val_tight, "Tight");
          if (val_tight) {
            m_oPhotonValidationPlots.m_oTruthRecoTightPlots.fill(*truthParticle,
                                                                 *eventInfo);
            if (isTrueConv) {
              m_oPhotonValidationPlots.m_oTruthRecoTightConvPlots.fill(
                *truthParticle, *eventInfo);
            } else {
              m_oPhotonValidationPlots.m_oTruthRecoTightUncPlots.fill(
                *truthParticle, *eventInfo);
            }
          } //--  end truth tight
        }   //--  end recoPhoton
      }     //-- end Photons
    }       // -- end fill histos iso particles
    // filling all truth particles from TruthParticles container (possibly will
    // be deleted, also possibly to fill only prompt particles)
    SG::ReadHandle<xAOD::TruthParticleContainer> truthallParticles(
      m_truthParticleContainerKey, ctx);
    ATH_CHECK(truthallParticles.isValid());

    MCTruthPartClassifier::Info info;
    bool elecPrompt = false;
    bool photonPrompt = false;

    for (const auto* const truthallParticle :
         *truthallParticles) { // Electrons and photons from standard
                               // TruthParticle container

      //--electrons
      if (std::abs(truthallParticle->pdgId()) == 11 &&
          MC::isStable(truthallParticle) &&
          HepMC::generations(truthallParticle) == 0) {

        auto type = m_truthClassifier->particleTruthClassifier(truthallParticle, &info);
        if (type.first == IsoElectron)
          elecPrompt = true;

        m_oElectronValidationPlots.m_oTruthAllPlots.fill(*truthallParticle,
                                                         *eventInfo);
          if (elecPrompt) {
              m_oElectronValidationPlots.m_oTruthAllIsoPlots.fill(*truthallParticle,
                                                                  *eventInfo);
              m_oElectronValidationPlots.m_oTruthAllPromptPlots.fill(*truthallParticle,
                                                                  *eventInfo);
          }
      } //-- end electrons

      //--photons
      if (std::abs(truthallParticle->pdgId()) == 22 &&
          MC::isStable(truthallParticle) &&
          HepMC::generations(truthallParticle) == 0) {

        auto type = m_truthClassifier->particleTruthClassifier(truthallParticle, &info);
        if (type.first == IsoPhoton)
          photonPrompt = true;


        m_oPhotonValidationPlots.m_oTruthAllPlots.fill(*truthallParticle,
                                                       *eventInfo);

        if (!photonPrompt)
          continue;
        if (truthallParticle->pt() / GeV > 20. &&
            fabs(truthallParticle->eta()) < 2.47) {
          m_oPhotonValidationPlots.m_oTruthAllIsoPlots.fill(*truthallParticle, *eventInfo);
          m_truthClassifier->particleTruthClassifier(truthallParticle, &info);
          ParticleOutCome photOutCome = info.particleOutCome;

          float convTruthR = 9999.;
          if (truthallParticle->decayVtx())
            convTruthR = truthallParticle->decayVtx()->perp();
          // std::cout<<"Truth Conversion R "<<convTruthR<<std::endl;
          // m_oPhotonValidationPlots.convTruthR->Fill(convTruthR);

          // fill only iso photon for conv and not converted
          if (photOutCome == Converted && convTruthR < 800.)
            m_oPhotonValidationPlots.m_oTruthAllIsoConvPlots.fill(
              *truthallParticle, *eventInfo);
          else
            m_oPhotonValidationPlots.m_oTruthAllIsoUncPlots.fill(
              *truthallParticle, *eventInfo);
        } // end cuts on truth
      }   // -- end photons
    }

    //---------Electrons----------------------
    if (!fillRecoElecHistograms(truthParticles.ptr(), eventInfo.ptr())) {
      ATH_MSG_ERROR("Filling reco elecectron hists  failed " << name()
                                                             << "...");
      return StatusCode::FAILURE;
    }

    //---------LRTElectrons----------------------
    if(!m_lrtelectronContainerKey.empty()){
      if (!fillLRTElecHistograms(truthParticles.ptr(), eventInfo.ptr())) {
        ATH_MSG_ERROR("Filling lrt elecectron hists  failed " << name() << "...");
        return StatusCode::FAILURE;
      }
    }

    //---------Frwd Electrons----------------------
    if (!fillRecoFrwdElecHistograms(truthParticles.ptr(), eventInfo.ptr())) {
      ATH_MSG_ERROR("Filling reco frwd elecectron hists  failed " << name()
                                                                  << "...");
      return StatusCode::FAILURE;
    }
    //----------Photons
    if (!fillRecoPhotHistograms(truthParticles.ptr(), eventInfo.ptr())) {
      ATH_MSG_ERROR("Filling reco photon  hists  failed " << name() << "...");
      return StatusCode::FAILURE;
    }
  } else // end is MC / code using truth particles
  {//---------Electrons----------------------
      if (!fillRecoElecHistograms(nullptr, eventInfo.ptr())) {
        ATH_MSG_ERROR("Filling reco elecectron hists  failed " << name()
                                                               << "...");
        return StatusCode::FAILURE;
      }
      //---------Frwd Electrons----------------------
      if (!fillRecoFrwdElecHistograms(nullptr, eventInfo.ptr())) {
        ATH_MSG_ERROR("Filling reco frwd elecectron hists  failed " << name()
                                                                    << "...");
        return StatusCode::FAILURE;
      }
      //----------Photons
      if (!fillRecoPhotHistograms(nullptr, eventInfo.ptr())) {
        ATH_MSG_ERROR("Filling reco photon  hists  failed " << name() << "...");
        return StatusCode::FAILURE;
      }
      
  }
    
    
    
  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::fillRecoElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo)
{
  ATH_MSG_DEBUG ("Filling reco electron hists " << name() << "...");
  
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::ElectronContainer> Electrons( m_electronContainerKey, ctx);
  ATH_CHECK(Electrons.isValid());
  
  int numofele=0;

  float weight = eventInfo->beamSpotWeight();
  
    for(const auto *const electron : *Electrons){
        bool isElecPrompt=false;
        
        if(!(electron->isGoodOQ (xAOD::EgammaParameters::BADCLUSELECTRON))) continue;
        
        if(electron->author()&xAOD::EgammaParameters::AuthorElectron||
           electron->author()&xAOD::EgammaParameters::AuthorAmbiguous)   numofele++;
        
        if(!m_isMC) m_oElectronValidationPlots.fill(*electron,*eventInfo,isElecPrompt);
        else {
            static const SG::ConstAccessor<int> truthTypeAcc ("truthType");
            if(truthTypeAcc.isAvailable (*electron)) {
                MCTruthPartClassifier::ParticleType type = (MCTruthPartClassifier::ParticleType) truthTypeAcc (*electron);
                if(type==MCTruthPartClassifier::IsoElectron) {
                    isElecPrompt=true;
                    //fill energy scale
                    const xAOD::TruthParticle* thePart = xAOD::TruthHelpers::getTruthParticle(*electron); // 20.7.X.Y.I
                    if(thePart) {
                        float EtLin = (electron->pt()-thePart->pt())/thePart->pt();
                        m_oElectronValidationPlots.res_et->Fill(thePart->pt()/GeV,EtLin,weight);
                        m_oElectronValidationPlots.res_eta->Fill(thePart->eta(),EtLin,weight);
                        if (thePart->pt()/GeV>20.) {
                            m_oElectronValidationPlots.res_et_cut->Fill(thePart->pt()/GeV,EtLin,weight);
                            m_oElectronValidationPlots.res_eta_cut->Fill(thePart->eta(),EtLin,weight);
                            m_oElectronValidationPlots.res_et_cut_pt_20->Fill(thePart->pt()/GeV,EtLin,weight);
                            m_oElectronValidationPlots.res_eta_cut_pt_20->Fill(thePart->eta(),EtLin,weight);
                        }
                        m_oElectronValidationPlots.matrix->Fill(electron->pt()/GeV,thePart->pt()/GeV);
                    } else {
                        ATH_MSG_INFO ("Truth particle associated not in egamma truth collection");
                    }
                }
                
            } else if(m_isMC){ if(Match(electron,11, truthParticles)!=nullptr ) isElecPrompt=true;}
            
            m_oElectronValidationPlots.fill(*electron,*eventInfo,isElecPrompt);
        }
    }
  m_oElectronValidationPlots.m_oCentralElecPlots.nParticles->Fill(numofele);
  m_oElectronValidationPlots.m_oCentralElecPlots.nParticles_weighted->Fill(numofele,weight);

  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::fillLRTElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo)
{
  ATH_MSG_DEBUG ("Filling lrt electron hists " << name() << "...");
  
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::ElectronContainer> LRTElectrons( m_lrtelectronContainerKey, ctx);
  ATH_CHECK(LRTElectrons.isValid());
  
  int numofele=0;

  float weight = eventInfo->beamSpotWeight();
  
  for(const auto *const electron : *LRTElectrons){
    bool isElecPrompt=false;

    if(!(electron->isGoodOQ (xAOD::EgammaParameters::BADCLUSELECTRON))) continue;

    // Retrieve electron ID, compute on-the-fly if decoration is missing (for AODs)
    bool pass_LHVeryLooseNoPix = false;
    if (m_acc_electronLLH_VeryLooseNoPix.isAvailable(*electron)) electron->passSelection(pass_LHVeryLooseNoPix, "DFCommonElectronsLHVeryLooseNoPix");
    else pass_LHVeryLooseNoPix = static_cast<bool>(m_Electron_VeryLooseNoPix_LLHTool->accept(electron));

    bool pass_LHLooseNoPix = false;
    if (m_acc_electronLLH_LooseNoPix.isAvailable(*electron)) electron->passSelection(pass_LHLooseNoPix, "DFCommonElectronsLHLooseNoPix");
    else pass_LHLooseNoPix = static_cast<bool>(m_Electron_LooseNoPix_LLHTool->accept(electron));

    bool pass_LHMediumNoPix = false;
    if (m_acc_electronLLH_MediumNoPix.isAvailable(*electron)) electron->passSelection(pass_LHMediumNoPix, "DFCommonElectronsLHMediumNoPix");
    else pass_LHMediumNoPix = static_cast<bool>(m_Electron_MediumNoPix_LLHTool->accept(electron));

    bool pass_LHTightNoPix = false;
    if (m_acc_electronLLH_TightNoPix.isAvailable(*electron)) electron->passSelection(pass_LHTightNoPix, "DFCommonElectronsLHTightNoPix");
    else pass_LHTightNoPix = static_cast<bool>(m_Electron_TightNoPix_LLHTool->accept(electron));

    static const SG::ConstAccessor<int> truthTypeAcc ("truthType");
    if(truthTypeAcc.isAvailable(*electron)) {
      MCTruthPartClassifier::ParticleType type = (MCTruthPartClassifier::ParticleType) truthTypeAcc(*electron);
      if(type==MCTruthPartClassifier::IsoElectron) {
        isElecPrompt=true;
	      //fill energy scale
	      const xAOD::TruthParticle* thePart = xAOD::TruthHelpers::getTruthParticle(*electron); // 20.7.X.Y.I
	      if(thePart) {
          float EtLin = (electron->pt()-thePart->pt())/thePart->pt();
	        m_oLRTElectronValidationPlots.res_et->Fill(thePart->pt()/GeV,EtLin,weight);
	        m_oLRTElectronValidationPlots.res_eta->Fill(thePart->eta(),EtLin,weight);
          if (thePart->pt()/GeV>20.) {
            m_oLRTElectronValidationPlots.res_et_cut->Fill(thePart->pt()/GeV,EtLin,weight);
            m_oLRTElectronValidationPlots.res_eta_cut->Fill(thePart->eta(),EtLin,weight);
            m_oLRTElectronValidationPlots.res_et_cut_pt_20->Fill(thePart->pt()/GeV,EtLin,weight);
            m_oLRTElectronValidationPlots.res_eta_cut_pt_20->Fill(thePart->eta(),EtLin,weight);
          }
	        m_oLRTElectronValidationPlots.matrix->Fill(electron->pt()/GeV,thePart->pt()/GeV);
        }else {
	        ATH_MSG_INFO ("Truth particle associated not in egamma truth collection");
        }
	    }  
    } else if(m_isMC){ if(Match(electron,11, truthParticles)!=nullptr ) isElecPrompt=true;}
    
    
    m_oLRTElectronValidationPlots.fill(*electron,*eventInfo, isElecPrompt, pass_LHVeryLooseNoPix, pass_LHLooseNoPix, pass_LHMediumNoPix, pass_LHTightNoPix);
    if(electron->author()&xAOD::EgammaParameters::AuthorElectron||
       electron->author()&xAOD::EgammaParameters::AuthorAmbiguous)   numofele++;
    
  } 
  
  m_oLRTElectronValidationPlots.m_oCentralElecPlots.nParticles->Fill(numofele);
  m_oLRTElectronValidationPlots.m_oCentralElecPlots.nParticles_weighted->Fill(numofele,weight);

  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::fillRecoFrwdElecHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo)
{
  ATH_MSG_DEBUG ("Filling reco frwd electron hists " << name() << "...");

  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::ElectronContainer> ElectronsFrwd( m_electronContainerFrwdKey, ctx);
  ATH_CHECK(ElectronsFrwd.isValid());
  
  int numoffrwdele=0;
  float weight = eventInfo->beamSpotWeight();
  
  for(const auto *const frwdelectron : *ElectronsFrwd){
    if(!(frwdelectron->isGoodOQ (xAOD::EgammaParameters::BADCLUSELECTRON))) continue;
    bool isElecPrompt=false;
    if (m_isMC) {
      if((Match(frwdelectron,11, truthParticles)!=nullptr )) isElecPrompt=true;
    }
    m_oElectronValidationPlots.fill(*frwdelectron,*eventInfo,isElecPrompt);
    numoffrwdele++;
    isElecPrompt=false;
  }
  
  m_oElectronValidationPlots.m_oFrwdElecPlots.nParticles->Fill(numoffrwdele);
  m_oElectronValidationPlots.m_oFrwdElecPlots.nParticles_weighted->Fill(numoffrwdele,weight);
  
  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::fillRecoPhotHistograms(const xAOD::TruthParticleContainer* truthParticles, const xAOD::EventInfo* eventInfo)
{
  ATH_MSG_DEBUG ("Filling reco photon  hists " << name() << "...");
 
  const EventContext& ctx = Gaudi::Hive::currentContext();
  SG::ReadHandle<xAOD::PhotonContainer> Photons( m_photonContainerKey, ctx);
  ATH_CHECK(Photons.isValid());
  
  int numofPhot=0;
  int numofAmb=0; 
  int numPhotAll=0; 
  int numofCnv=0;
  float weight = eventInfo->beamSpotWeight();

    for(const auto *photon : *Photons){
        bool isPhotPrompt=false;
        if (photon->author()&xAOD::EgammaParameters::AuthorCaloTopo35) continue;//21.0.>7
        if(!(photon->isGoodOQ (xAOD::EgammaParameters::BADCLUSPHOTON))) continue;
        if (m_useOQQuality) {
          if (!PhotonHelpers::passOQquality(*photon)) continue;
        }
        
        if(photon->author())           numofPhot++;
        else if(photon->author()&xAOD::EgammaParameters::AuthorAmbiguous)   numofAmb++;
        if(xAOD::EgammaHelpers::isConvertedPhoton(photon))                  numofCnv++;
        if(!m_isMC) m_oPhotonValidationPlots.fill(*photon,*eventInfo, isPhotPrompt);
        else {
            static const SG::ConstAccessor<int> truthTypeAcc ("truthType");
            if(truthTypeAcc.isAvailable(*photon)) {
                MCTruthPartClassifier::ParticleType type = (MCTruthPartClassifier::ParticleType) truthTypeAcc(*photon);
                if(type==MCTruthPartClassifier::IsoPhoton) {
                    isPhotPrompt=true;	
                    //fill energy scale
                    const xAOD::TruthParticle* thePart = xAOD::TruthHelpers::getTruthParticle(*photon);//20.7.X.Y.I
                    //	const xAOD::TruthParticle* thePart = xAOD::EgammaHelpers::getTruthParticle(photon);
                    if(thePart&&thePart->pt()/GeV>20.) {
                        float EtLin = (photon->pt()-thePart->pt())/thePart->pt();
                        m_oPhotonValidationPlots.res_et->Fill(thePart->pt()/GeV,EtLin,weight);
                        m_oPhotonValidationPlots.res_eta->Fill(thePart->eta(),EtLin,weight);
                        if (std::abs(EtLin)<0.2){
                            m_oPhotonValidationPlots.res_et_cut->Fill(thePart->pt()/GeV,EtLin,weight);
                            m_oPhotonValidationPlots.res_eta_cut->Fill(thePart->eta(),EtLin,weight);
                        }
                    }else {
  		        ATH_MSG_INFO("Truth particle associated not in egamma truth collection");
                    }
                }
                
            } else if(m_isMC){if(Match(photon,22, truthParticles)!=nullptr ) isPhotPrompt=true;}    
            
            m_oPhotonValidationPlots.fill(*photon,*eventInfo, isPhotPrompt);
            
        }
    }
  numPhotAll = numofPhot+numofAmb;
  m_oPhotonValidationPlots.m_oAllPlots.m_nParticles->Fill(numPhotAll);
  m_oPhotonValidationPlots.m_oPhotPlots.m_nParticles->Fill(numofPhot);
  m_oPhotonValidationPlots.m_oAmbPhotPlots.m_nParticles->Fill(numofAmb);
  m_oPhotonValidationPlots.m_oConvPhotPlots.m_nParticles->Fill(numofCnv);

  m_oPhotonValidationPlots.m_oAllPlots.m_nParticles_weighted->Fill(numPhotAll,weight);
  m_oPhotonValidationPlots.m_oPhotPlots.m_nParticles_weighted->Fill(numofPhot,weight);
  m_oPhotonValidationPlots.m_oAmbPhotPlots.m_nParticles_weighted->Fill(numofAmb,weight);
  m_oPhotonValidationPlots.m_oConvPhotPlots.m_nParticles_weighted->Fill(numofCnv,weight);
 

  return StatusCode::SUCCESS;
}

StatusCode EgammaPhysValMonitoringTool::procHistograms() 
  {
    ATH_MSG_INFO ("Finalising hists " << name() << "...");
    m_oElectronValidationPlots.finalize();
    m_oPhotonValidationPlots.finalize();
    return StatusCode::SUCCESS;
  }

const xAOD::TruthParticle* EgammaPhysValMonitoringTool::Match(const xAOD::Egamma* particle,
							      int pdg,
							      const xAOD::TruthParticleContainer* truthParticles) {
  float currentdr = 0.05;
  const xAOD::TruthParticle* matchedTruthParticle = nullptr;
  if (truthParticles){
    for (const auto *truthParticle: *truthParticles){
      if (std::abs(truthParticle->pdgId()) != pdg || !MC::isStable(truthParticle)) continue;
      float dr = particle->p4().DeltaR(truthParticle->p4());
      if (dr < currentdr){
        currentdr = dr;
        matchedTruthParticle = truthParticle;
      }
    }
  }
  return matchedTruthParticle;
}


}

