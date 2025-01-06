/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"
#include "TruthUtils/HepMCHelpers.h"

namespace DerivationFramework{

  HadronOriginClassifier::HadronOriginClassifier(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p),
    m_mcName("TruthEvents"),
    m_HadronPtMinCut(0),
    m_HadronEtaMaxCut(0),
    m_DSID(0)
    {
      declareInterface<DerivationFramework::HadronOriginClassifier>(this);

      declareProperty("MCCollectionName",m_mcName="TruthEvents");
      declareProperty("HadronpTMinCut",m_HadronPtMinCut=5000.); /// MeV
      declareProperty("HadronetaMaxCut",m_HadronEtaMaxCut=2.5);
      declareProperty("DSID",m_DSID=410000);
    }

  HadronOriginClassifier::~HadronOriginClassifier(){}

  StatusCode HadronOriginClassifier::initialize() {
    ATH_MSG_INFO("Initialize " );
    ATH_MSG_INFO("DSID " << m_DSID );
    // all Herwig++/Herwig7 showered samples
    if( m_DSID==410003 || m_DSID == 410008 //aMC@NLO+Hpp
          || m_DSID == 410004 || m_DSID == 410163 //Powheg+Hpp
          || m_DSID == 410232 //first attempt for Powheg+H7
          || m_DSID == 410233 //first attempt for aMC@NLO+H7
          || (m_DSID>=410525 && m_DSID<=410530) //New Powheg+H7 samples
          || (m_DSID>=407037 && m_DSID<=407040) //Powheg+Hpp MET/HT sliced
          || m_DSID ==410536 || m_DSID == 410537
          || m_DSID == 410245 //aMC@NLO+H++ , ttbb
          || (m_DSID>=410557 && m_DSID<=410559) // new Powheg+H7, mc16
          || (m_DSID>=411082 && m_DSID<=411090) //Powheg+H7 HF-filtered
          || (m_DSID>=407354 && m_DSID<=407356) //Powheg+H7 ttbar HT-filtered
          || m_DSID ==411233 || m_DSID==411234 //Powheg+H7.1.3 ttbar 
          || m_DSID == 411316 //Powheg+H7 allhad ttbar
          || (m_DSID>=411329 && m_DSID<=411334) //Powheg+H7.1.3 ttbar HF-filtered
          || (m_DSID>=411335 && m_DSID<=411337) //Powheg+H7.1.3 ttbar HT-filtered
          || m_DSID ==412116 || m_DSID == 412117 //amc@NLO+H7.1.3 ttbar 
          || m_DSID ==504329 || m_DSID == 504333 || m_DSID == 504341 //amc@NLO+H7.2.1 refined ttZ
          || (m_DSID >= 601239 && m_DSID <= 601240)
          || m_DSID == 601668 || m_DSID == 603905 || m_DSID == 603906 // ttbb Powheg+H7 dilep, ljet, allhad
          ){
      m_GenUsed=HerwigPP;
      if (m_DSID==410245 || (m_DSID >= 601239 && m_DSID <= 601240)
          || m_DSID == 601668 || m_DSID == 603905 || m_DSID == 603906
          ){
        m_ttbb=true;
      }
    }
    // all Pythia8 showered samples
    else if( m_DSID==410006 //Powheg+P8 old main31
          || m_DSID==410500 //Powheg+P8 new main31, hdamp=mt
          || (m_DSID>=410501 && m_DSID<=410508) //Powheg+P8 new main31, hdamp=1.5m // Boosted samples are included 410507 410508
          || (m_DSID>=410511 && m_DSID<=410524) //Powheg+P8 new main31, hdamp=1.5mt, radiation systematics
          || (m_DSID>=410531 && m_DSID<=410535) //Powheg+P8 allhad samples
          || (m_DSID>=346343 && m_DSID<=346345) //Powheg+P8 ttH
          || m_DSID==412123 // MG+P8 ttW
          || m_DSID==410155 // aMC@NlO+P8 ttW
          || m_DSID==410159 || m_DSID==410160 //aMC@NLO+P8, old settings
          || (m_DSID>=410218 && m_DSID<=410220) // aMC@NlO+P8 ttZ
          || (m_DSID>=410276 && m_DSID<=410278) // aMC@NlO+P8 ttZ_lowMass
          || (m_DSID>=410225 && m_DSID<=410227) || m_DSID==410274 || m_DSID==410275 //aMC@NLO+P8, new settings
          || m_DSID==410568 || m_DSID==410569 // nonallhad boosted c-filtered
          || m_DSID==410244 //aMC@NLO+P8, ttbb (old)
          || m_DSID==410441 || m_DSID==410442 //new aMC@NLO+P8 mc16, new shower starting scale
          || (m_DSID>=410464 && m_DSID<=410466) //new aMC@NLO+P8 mc16, new shower starting scale, no shower weights
          || (m_DSID>=410470 && m_DSID<=410472) || (m_DSID>=410480 && m_DSID<=410482) //new Powheg+P8 mc16
          || m_DSID==410452 //new aMC@NLO+P8 FxFx mc16
          || (m_DSID>=411073 && m_DSID<=411081) //Powheg+P8 HF-filtered
          || (m_DSID>=412066 && m_DSID<=412074) //aMC@NLO+P8 HF-filtered
          || (m_DSID>=411068 && m_DSID<=411070) //Powheg+P8 ttbb
          || (m_DSID>=410265 && m_DSID<=410267) //aMC@NLO+P8 ttbb
          || (m_DSID>=411178 && m_DSID<=411180) || (m_DSID==411275) //Powheg+P8 ttbb OTF production - ATLMCPROD-7240
          || (m_DSID>=600791 && m_DSID<=600792) //Powheg+P8 ttbb - ATLMCPROD-9179
          || (m_DSID>=600737 && m_DSID<=600738) //Powheg+P8 ttbb - ATLMCPROD-9179
          || (m_DSID>=601226 && m_DSID<=601227) // Powheg+P8 ttbb bornzerodamp cut 5, ATLMCPROD-9694
          || (m_DSID>=407342 && m_DSID<=407344) //Powheg+P8 ttbar HT-filtered
          || (m_DSID>=407345 && m_DSID<=407347) //Powheg+P8 ttbar MET-filtered
          || (m_DSID>=407348 && m_DSID<=407350) //aMC@NLO+P8 ttbar HT-filtered
          ||  m_DSID==504330 || m_DSID==504331 || m_DSID==504332 || m_DSID==504334 || m_DSID==504335 || m_DSID==504336 || m_DSID==504338 || m_DSID==504342 || m_DSID==504343 || m_DSID==504344 || m_DSID==504346//aMC@NLO+P8 refined ttZ
          ||  m_DSID==601491 || m_DSID==601492  //Pow+Py8 ttbar pTHard variations - ATLMCPROD-10168
          || (m_DSID>=601495 && m_DSID<=601498) //Pow+Py8 ttbar pTHard variations - ATLMCPROD-10168
          || (m_DSID>=601229 && m_DSID<=601230) // mc23 ttbar dilep, singlelep
          || m_DSID==601237 // mc23 ttbar allhad       
          || (m_DSID>=601398 && m_DSID<=601399) // mc23 ttbar dilep, singlelep hdamp517p5
          || m_DSID==601491 || m_DSID==601495 || m_DSID==601497 // mc23 ttbar pThard variations, dilep, singlelep, allhad    
          || (m_DSID>=601783 && m_DSID<=601784) // Powheg+P8 ttbb bornzerodamp cut 5 pThard variations - ATLMCPROD-10527
          || (m_DSID>=603003 && m_DSID<=603004) // Powheg+P8 ttbb nominal and pthard1 allhad
          || (m_DSID>=603190 && m_DSID<=603193) // Powheg+P8 ttbb nominal and pthard1 dilep, ljet
          ){
      m_GenUsed=Pythia8;
      if ( m_DSID==410244 //aMC@NLO+P8, ttbb (old)
          || (m_DSID>=411068 && m_DSID<=411070) //Powheg+P8 ttbb
          || (m_DSID>=410265 && m_DSID<=410267) //aMC@NLO+P8 ttbb
          || (m_DSID>=411178 && m_DSID<=411180) || (m_DSID==411275) //Powheg+P8 ttbb OTF production - ATLMCPROD-7240
          || (m_DSID>=600791 && m_DSID<=600792) // Powheg+P8 ttbb
          || (m_DSID>=600737 && m_DSID<=600738) // Powheg+P8 ttbb dipole recoil
          || (m_DSID>=601226 && m_DSID<=601227) // Powheg+P8 ttbb bornzerodamp cut 5
          || (m_DSID>=601783 && m_DSID<=601784) // Powheg+P8 ttbb bornzerodamp cut 5 pThard variations - ATLMCPROD-10527
          || (m_DSID>=603003 && m_DSID<=603004) // Powheg+P8 ttbb nominal and pthard1 allhad
          || (m_DSID>=603190 && m_DSID<=603193) // Powheg+P8 ttbb nominal and pthard1 dilep, ljet
      ){
        m_ttbb=true;
      }
    }
    // all Sherpa showered samples
    else if( (m_DSID>=410186 && m_DSID<=410189) //Sherpa 2.2.0
          || (m_DSID>=410249 && m_DSID<=410252) //Sherpa 2.2.1
          || (m_DSID>=410342 && m_DSID<=410347) //Sherpa 2.2.1 sys
          || (m_DSID>=410350 && m_DSID<=410355) //Sherpa 2.2.1 sys
          || (m_DSID>=410357 && m_DSID<=410359) //Sherpa 2.2.1 sys
          || (m_DSID>=410361 && m_DSID<=410367) //Sherpa 2.2.1 sys
          || (m_DSID>=410281 && m_DSID<=410283) //Sherpa BFilter
          || m_DSID==410051 //Sherpa ttbb (ICHEP sample)
          || (m_DSID>=410323 && m_DSID<=410325) || (m_DSID==410369) //New Sherpa 2.2.1 ttbb
          || (m_DSID>=364345 && m_DSID<=364348) //Sherpa 2.2.4 (test)
          || (m_DSID>=410424 && m_DSID<=410427) //Sherpa 2.2.4
          || (m_DSID>=410661 && m_DSID<=410664) //Sherpa 2.2.4 ttbb
          || (m_DSID>=421152 && m_DSID<=421158) //Sherpa2.2.8 ttbar
          ||  m_DSID==413023 // sherpa 2.2.1 ttZ 
          ||  m_DSID==700000 // Sherpa 2.2.8 ttW
          ||  m_DSID==700168 // Sherpa 2.2.10 ttW
          ||  m_DSID==700205 // Sherpa 2.2.10 ttW EWK
          ||  m_DSID==700309 // Sherpa 2.2.11 ttZ
          || (m_DSID>=700051 && m_DSID<=700054) //Sherpa2.2.8 ttbb
          || (m_DSID>=700121 && m_DSID<=700124) //Sherpa2.2.10 ttbar
          || (m_DSID>=700164 && m_DSID<=700167) //Sherpa2.2.10 ttbb
          || (m_DSID>=700807 && m_DSID<=700809) //Sherpa2.2.14 ttbar
          ){
      m_GenUsed=Sherpa;
      if( m_DSID==410051
          || (m_DSID>=410323 && m_DSID<=410325) || (m_DSID==410369)
          || (m_DSID>=410661 && m_DSID<=410664)
          || (m_DSID>=700051 && m_DSID<=700054)
          || (m_DSID>=700164 && m_DSID<=700167)
        ){
        m_ttbb=true;
      }
    }
    // the default is Pythia6, so no need to list the Pythia6 showered samples
    // these are:
    // 410000-410002
    // 410007, 410009,  410120-410121
    // 301528-301532
    // 303722-303726
    // 407009-407012
    // 407029-407036
    // 410120
    // 426090-426097
    // 429007
    else{
      m_GenUsed=Pythia6;
    }
    return StatusCode::SUCCESS;
  }

  /*
  --------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------------- Hadron Map -------------------------------------------------------------
  --------------------------------------------------------------------------------------------------------------------------------------
  */

  // Define the function GetOriginMap that determines the origin of the hadrons.
  std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id> HadronOriginClassifier::GetOriginMap() const {
    // Create a set of maps to store the information about the hadrons and the partons 
    std::map<const xAOD::TruthParticle*, int> mainHadronMap;                         // Map with main hadrons and their flavor.
    std::map<const xAOD::TruthParticle*, HF_id> partonsOrigin;                       // Map with partons and their category (from top, W, H, MPI, FSR, extra).
    std::map<const xAOD::TruthParticle*, const xAOD::TruthParticle*> hadronsPartons; // Map with hadrons and their matched parton.
    std::map<const xAOD::TruthParticle*, HF_id> hadronsOrigin;                       // Map with hadrons and their category (from top, W, H, MPI, FSR, extra)
    // Fill the maps mainHadronMap and partonsOrigin
    buildPartonsHadronsMaps(mainHadronMap, partonsOrigin);
    // Create two maps to know which partons and hadrons have already been matched.
    std::vector<const xAOD::TruthParticle*> matched_partons;
    std::vector<const xAOD::TruthParticle*> matched_hadrons;
    // Use a while to go through the HF hadrons in mainHadronMap and partons in partonsOrigin.
    while (matched_partons.size()<partonsOrigin.size() && matched_hadrons.size()<mainHadronMap.size()){
      // Create a float variable to store the DeltaR between a parton and the closest hadron.
      float dR=999.;
      // Create two pointers for TruthParticle type to go through the partons and hadrons.
      const xAOD::TruthParticle* hadron=nullptr;
      const xAOD::TruthParticle* parton=nullptr;
      // Use a for to go through the partonsOrigin.
      for(std::map<const xAOD::TruthParticle*, HF_id>::iterator itr = partonsOrigin.begin(); itr!=partonsOrigin.end(); ++itr){
        // Check if the parton has already been matched to an hadron.
        if(std::find(matched_partons.begin(), matched_partons.end(), (*itr).first) != matched_partons.end()) continue;
        // Extract the pt of the parton.
        TVector3 v, vtmp;
        if ((*itr).first->pt()>0.)
          v.SetPtEtaPhi((*itr).first->pt(),(*itr).first->eta(),(*itr).first->phi());
        else // Protection against FPE from eta and phi calculation
          v.SetXYZ(0.,0.,(*itr).first->pz());
        // Use a for to go through the HF hadrons in mainHadronMap.
        for(std::map<const xAOD::TruthParticle*, int>::iterator it = mainHadronMap.begin(); it!=mainHadronMap.end(); ++it){
          // Check if the hadron has already been matched to a parton.
          if(std::find(matched_hadrons.begin(), matched_hadrons.end(), (*it).first) != matched_hadrons.end()) continue;
          // Check if the hadron's flavour mathces the one of the parton.
          if((*it).second != abs((*itr).first->pdgId()) ) continue;
          // Extract the pt of the hadron.
          vtmp.SetPtEtaPhi((*it).first->pt(),(*it).first->eta(),(*it).first->phi());
          // Compute Delta R between hadron and parton and store in dR if it is smaller than the current value.
          // Also store the parton and hadron in the pointers that have been previous created.
          if(vtmp.DeltaR(v) < dR){
            dR = vtmp.DeltaR(v);
            hadron = (*it).first;
            parton = (*itr).first;
          }
        }//loop hadrons
      }//loop partons
      // Add the matched part-hadron pair in the corresponding maps.
      matched_partons.push_back(parton);
      matched_hadrons.push_back(hadron);
      hadronsPartons[ hadron ] = parton;
    }

    // Use a for to go through the HF hadrons in mainHadronMap.
    for(std::map<const xAOD::TruthParticle*, int>::iterator it = mainHadronMap.begin(); it!=mainHadronMap.end(); ++it){
      // Extract the current hadron.
      const xAOD::TruthParticle* hadron = (*it).first;
      // Check if the hadron has been matched to a parton.
      // If it has been matched to any hadron, use it to determine the origin.
      // Otherwise, the hadron is considered extra.
      if(hadronsPartons.find(hadron)!=hadronsPartons.end()){
        hadronsOrigin[hadron] = partonsOrigin[ hadronsPartons[hadron] ];
      } else{
        hadronsOrigin[hadron] = extrajet;
      }
    }
    return hadronsOrigin;
  }

  // Define the function buildPartonsHadronsMaps that determines the flavour of the hadrons and the origin of the partons.
  void HadronOriginClassifier::buildPartonsHadronsMaps(std::map<const xAOD::TruthParticle*,int>& mainHadronMap, std::map<const xAOD::TruthParticle*,HF_id>& partonsOrigin) const {
    // Extract the TruthParticles container.
    const xAOD::TruthEventContainer* xTruthEventContainer = nullptr;
    if (evtStore()->retrieve(xTruthEventContainer,m_mcName).isFailure()) {
      ATH_MSG_WARNING("could not retrieve TruthEventContainer " <<m_mcName);
    }
    // Create a container with TruthParticles to store the hadrons that has already been saved.
    std::set<const xAOD::TruthParticle*> usedHadron;
    for ( const auto* truthevent : *xTruthEventContainer ) {
      // Use a for to go through the TruthParticles.
      for(unsigned int i = 0; i < truthevent->nTruthParticles(); i++){
        // Extract the i-th particle.
        const xAOD::TruthParticle* part = truthevent->truthParticle(i);
        if(!part) continue;
        // Simulated particles are not considered.
        if(HepMC::is_simulation_particle(part)) break;
        // Create a set of boolean variables to indicate the type of particle.
        bool isbquark   = false; // The particle is a b-quark.
        bool iscquark   = false; // The particle is a c-quark.
        bool isHFhadron = false; // The particle is a HF hadron.
        // Extract the pdgid of the particle and use it to determine the type of particle.
        int pdgid = abs(part->pdgId());
        if( MC::isBottom(pdgid) ){
          isbquark=true;
        }
        else if( MC::isCharm(pdgid) ){
          iscquark=true;
        }
        else if(MC::isBottomHadron(part) || MC::isCharmHadron(part)){
          isHFhadron=true;
        }
        else{
          continue;
        }
        // For HF quarks (b or c), check their category.
        // The category is determined looking for the parents.
        if(isbquark){
          // In this case, the parton is a b-quark.
          // Create a boolean that indicates when to stop to look for parents.
          bool islooping = isLooping(part);
          // Check the category of the b-quark.
          if(isDirectlyFromWTop(part, islooping)){
            partonsOrigin[ part ] = b_from_W; 
          }
          else if(isDirectlyFromTop(part, islooping)){
            partonsOrigin[ part ] = b_from_top;
          }
          else if(!IsTtBb()&&(IsHerwigPP()||IsSherpa())&&isDirectlyFSR(part,islooping)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyFSRPythia8(part,islooping)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyFSRPythia6(part,islooping)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyMPIPythia6(part, islooping)){
            partonsOrigin[ part ] = b_MPI;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyMPIPythia8(part, islooping)){
            partonsOrigin[ part ] = b_MPI;
          }
          else if(!IsTtBb()&&IsSherpa()&&isDirectlyMPISherpa(part)){
            partonsOrigin[ part ] = b_MPI;
          }
        }
        if(iscquark){
          // In this case, the parton is a c-quark.
          // Create a boolean that indicates when to stop to look for parents.
          bool islooping = isLooping(part);
          // Check the category of the b-quark.
          if(isDirectlyFromWTop(part, islooping)){
            partonsOrigin[ part ] = c_from_W;
          }
          else if(isDirectlyFromTop(part, islooping)){
            partonsOrigin[ part ] = c_from_top;
          }
          else if(!IsTtBb()&&(IsHerwigPP()&&IsSherpa())&&isDirectlyFSR(part,islooping)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyFSRPythia8(part,islooping)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyFSRPythia6(part,islooping)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(!IsTtBb()&&IsPythia6()&&isDirectlyMPIPythia6(part, islooping)){
            partonsOrigin[ part ] = c_MPI;
          }
          else if(!IsTtBb()&&IsPythia8()&&isDirectlyMPIPythia8(part, islooping)){
            partonsOrigin[ part ] = c_MPI;
          }
          else if(!IsTtBb()&&IsSherpa()&&isDirectlyMPISherpa(part)){
            partonsOrigin[ part ] = c_MPI;
          }
        }
        // The HF hadrons are stored in the map mainHadronMap if they are not repeated.
        if(isHFhadron && !isCHadronFromB(part)){
          // In this case, the particle is a HF hadron but not a C-Hadron from a B-hadron.
          // If the hadron is not in usedHadron, then add it in mainHadronMap with fillHadronMap function.
          if(usedHadron.insert(part).second) {
            fillHadronMap(usedHadron, mainHadronMap,part,part);
          }
        }
      }//loop on particles
    }//loop on truthevent container
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ------------------------------------------------------------ Particle Type ------------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */
  bool HadronOriginClassifier::isCHadronFromB(const xAOD::TruthParticle* part, std::shared_ptr<std::set<const xAOD::TruthParticle*>> checked ) const{
    if(!MC::isCharmHadron(part)) return false;
    if (!checked) checked  = std::make_shared<std::set<const xAOD::TruthParticle*>>();
    checked ->insert(part);

    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(checked->count(parent)) continue;
      checked->insert(parent);
      if( MC::isBottomHadron(parent) ){
        return true;
      }
      if(MC::isCharmHadron(parent)){
        if(isCHadronFromB(parent))return true;
      }
    }

    return false;
  }

  // Define the function fillHadronMap that fills the map of hadrons with their flavour.
  void HadronOriginClassifier::fillHadronMap(std::set<const xAOD::TruthParticle*>& usedHadron, std::map<const xAOD::TruthParticle*,int>& mainHadronMap, const xAOD::TruthParticle* mainhad, const xAOD::TruthParticle* ihad, bool decayed) const {
    // Fist, check that the consdired hadron has a non-null pointer 
    if (!ihad) return;
    usedHadron.insert(ihad);
    // Create two variables to indicate the flavour of the parents and childrens particles that will be considered.
    // Create a boolean to indicate if the particles considered are from the final state.
    int parent_flav,child_flav;
    bool isFinal = true;
    // Check if the considered hadron has children.
    if(!ihad->nChildren()) return;
    // Use a for to go through the children.
    for(unsigned int j=0; j<ihad->nChildren(); ++j){
      // Extract the j-th children.
      const xAOD::TruthParticle* child = ihad->child(j);
      if(!child) continue;
      if(decayed){
        fillHadronMap(usedHadron, mainHadronMap,mainhad,child,true);
        isFinal=false;
      }
      else{
        child_flav = std::abs(MC::leadingQuark(child));
        if(child_flav!=4 && child_flav!=5) continue;
        parent_flav = std::abs(MC::leadingQuark(mainhad));
        if(child_flav!=parent_flav) continue;
        fillHadronMap(usedHadron, mainHadronMap,mainhad,child);
        isFinal=false;
      }
    }

    if(isFinal && !decayed){
      mainHadronMap[mainhad]=std::abs(MC::leadingQuark(mainhad));
      for(unsigned int j=0; j<ihad->nChildren(); ++j){
        const xAOD::TruthParticle* child = ihad->child(j);
        if(!child) continue;
        fillHadronMap(usedHadron, mainHadronMap,mainhad,child,true);
      }
    }
  }

  /*
  ---------------------------------------------------------------------------------------------------------------------------------------
  ----------------------------------------------------------- Particle Origin -----------------------------------------------------------
  ---------------------------------------------------------------------------------------------------------------------------------------
  */

  // Define the function isFromTop that indicates if a particle comes from top.

  bool HadronOriginClassifier::isFromTop(const xAOD::TruthParticle* part, bool looping) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    // Check if this parent comes from the top with function isDirectlyFromTop.
    return isDirectlyFromTop(initpart, looping);
  }

  // Define the function isDirectlyFromTop that indicates if a particle comes from the direct decay of top.
  bool HadronOriginClassifier::isDirectlyFromTop(const xAOD::TruthParticle* part, bool looping) {
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part || !part->nParents()) return false;
    // Go through the parents of the particle.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(looping) continue;
      // If the i-th parent is a top, then return true
      if( MC::isTop(parent) ) return true;
    }
    // If a top is no the parent, then return false.
    return false;
  }

  // Define the function isFromWTop that indicates if a particle comes from the decay chain t->Wb.

  bool HadronOriginClassifier::isFromWTop(const xAOD::TruthParticle* part, bool looping) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromWTop(initpart, looping);
  }

  // Define the function isDirectlyFromWTop that indicates if a particle comes from the direct decay of a W from a top.
  bool HadronOriginClassifier::isDirectlyFromWTop(const xAOD::TruthParticle * part, bool looping) const{
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part || !part->nParents()) return false;
    // Use a for to go though the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Get the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(looping) continue;
      if( MC::isW(parent)){
        if( isFromTop(parent, looping) ) return true;
      }
    }
    // In this case, none of the parents of the particle is a W from top.
    // Hence, return false.
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFromGluonQuark(const xAOD::TruthParticle* part, bool looping) {
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( looping ) continue;
      if( MC::isPhoton(parent) || abs(parent->pdgId())<MC::BQUARK ) return true;
    }
    return false;
  }

  bool HadronOriginClassifier::isFromGluonQuark(const xAOD::TruthParticle* part, bool looping) const{
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromGluonQuark(initpart, looping);
  }

  bool HadronOriginClassifier::isDirectlyFSRPythia6(const xAOD::TruthParticle * part, bool looping) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(looping ) continue;
      if(!MC::isW(parent)) continue;
      if(MC::isCharm(part)){
        //trick to get at least 50% of PowhegPythia c from FSR
        if(part->pdgId()==-(parent->pdgId())/6){
          if( isFromGluonQuark(parent, looping) ) return true;
        }
      }
      else{
        if( isFromGluonQuark(parent, looping) ) return true;
      }
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFSR(const xAOD::TruthParticle * part, bool looping) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( looping ) continue;
      if( MC::isPhoton(parent) || MC::isGluon(parent) ){
        if( isFromQuarkTop( parent,looping ) ) return true;
      }
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( looping ) continue;
      if( abs(parent->pdgId())<MC::TQUARK ) {

        if(isFromTop(parent,looping)){
          return true;
        }
        else if(isFromWTop(parent,looping)){
          return true;
        }
      }
    }

    return false;
  }

  bool HadronOriginClassifier::isFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const{
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return isDirectlyFromQuarkTop(initpart, looping);
  }

  // Define the function isDirectlyFSRPythia8 that indicates if a particle comes from Final State Radiation in samples generated with Pythia8.

  bool HadronOriginClassifier::isDirectlyFSRPythia8(const xAOD::TruthParticle * part, bool looping) const{
    // First, check if the particle has parents and return false if it does not.
    if(!part->nParents()) return false;
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){

      // Extract the i-th parent.

      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( looping ) continue;
      if( MC::isPhoton(parent) || MC::isGluon(parent) ){
        if( isFromQuarkTopPythia8( parent,looping ) ) return true;
      }
    }
    // In this case, no parent from the particle is a gluon or a photon coming from a top
    // Hence, the particle is not from FSR and false is not returned.
    return false;
  }

  // Define the function isDirectlyFromQuarkTopPythia8 that indicates if a particle comes from direct decay of the top in samples generated with Pythia8.
  bool HadronOriginClassifier::isDirectlyFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const{
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part->nParents()) return false;
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(looping ) continue;
      // Check if the parent is a quark different from the top.
      if( abs(parent->pdgId())<MC::TQUARK ) {
        // In this case, the parent is a quark different from top.
        // Check if it comes from the decay chain of the t->Wb.
        // If it is the case, return true.
        if(isFromWTop(parent,looping)){
          return true;
        }
      }
    }
    // In this case, any of the parents of the particle comes from t->Wb chaing.
    // Hence, the particle does not come from the top directly and false is returned.
    return false;
  }

  // Define the function isFromQuarkTopPythia8 that indicates if a particle comes from top in samples generated with Pythia8.
  bool HadronOriginClassifier::isFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    // Check if this parent comes from the top with function isDirectlyFromQuarkTopPythia8.
    return isDirectlyFromQuarkTopPythia8(initpart, looping);
  }

  bool HadronOriginClassifier::isDirectlyMPIPythia6(const xAOD::TruthParticle * part, bool looping) {
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( looping ) continue;
      if( abs(parent->pdgId()) == MC::PROTON && MC::isPhysical(part) ) return true;
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyMPIPythia8(const xAOD::TruthParticle * part, bool looping) const{
    const xAOD::TruthParticle* initpart = findInitial(part, looping);
    return MC::Pythia8::isConditionC(initpart);
  }

  bool HadronOriginClassifier::isDirectlyMPISherpa(const xAOD::TruthParticle * part) {
    if(!part->hasProdVtx()) return false;
    const xAOD::TruthVertex* vertex = part->prodVtx();
    return HepMC::status(vertex) == 2;
  }

  /*
  --------------------------------------------------------------------------------------------------------------------------------------
  ---------------------------------------------------------- Particle Parents ----------------------------------------------------------
  --------------------------------------------------------------------------------------------------------------------------------------
  */

  // Define the function isLooping that determines when to stop to look at the parents of a particle.
  bool HadronOriginClassifier::isLooping(const xAOD::TruthParticle* part, std::shared_ptr<std::set<const xAOD::TruthParticle*>> init_part) const{
    // First, check if the particle has parents and return false if it does not.
    if(!part->nParents()) return false;
    // In this case, the particle has parents.
    // Store the particle in the container init_part.
    if (!init_part) init_part = std::make_shared<std::set<const xAOD::TruthParticle*>>();
    init_part->insert(part);
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Get the i-th parent and check if it is in the container init_part.
      // If it is not, return true because the parent need to be checked.
      // Otherwise, check the parent of the parent and keep going until there is a parent to check or all parents are checked.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( init_part->count(parent)) return true;
      if( isLooping(parent, init_part) ) return true;
    }
    // If this point is reached, then it means that no parent needs to be checked.
    // Hence, return false.
    return false;
  }

  // Define the function findInitial which finds the first parent of a particle that is not the particle itself.
  const xAOD::TruthParticle*  HadronOriginClassifier::findInitial(const xAOD::TruthParticle* part, bool looping,  std::shared_ptr<std::set<const xAOD::TruthParticle*>> checked ) const{
    // If the particle has no parent, return the particle.
    if(!part->nParents()) return part;
    if (!checked) checked = std::make_shared<std::set<const xAOD::TruthParticle*>>();
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(checked->count(parent) && looping) continue;
      checked->insert(parent);
      // If the parent has the same pdgId as the particle, then it means that the parent is the same as the considered particle.
      // This happens if the particle irradiates for example.
      // In this case, try to look for the first parent of i-th parent that is being considered.
      // Repeat the process until you find a particle different from the considred one or that has no parent.

      if( part->pdgId() == parent->pdgId() ){
        return findInitial(parent, looping, checked);
      }
    }
    // In this case, no parent different from the considered particle has been found.
    // Hence, return the particle.
    return part;
  }

}//namespace
