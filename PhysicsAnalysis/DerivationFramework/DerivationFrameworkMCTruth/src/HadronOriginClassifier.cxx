/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"
#include "TruthUtils/HepMCHelpers.h"
#include "StoreGate/ReadHandle.h"

namespace {
  /// Helper class to store sample properties
  struct Sample {
    using GEN_id = DerivationFramework::HadronOriginClassifier::GEN_id;

    /// Range of samples with low <= id <= high
    Sample(int low, int high, GEN_id gen) :
      low(low), high(high), gen(gen) {}

    /// Single sample with `id`
    Sample(int id, GEN_id gen) :
      Sample(id, id, gen) {}

    int low{};
    int high{};
    GEN_id gen{GEN_id::Pythia6};
  };
}

namespace DerivationFramework{

  HadronOriginClassifier::HadronOriginClassifier(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p)
    {
    }

  HadronOriginClassifier::~HadronOriginClassifier(){}

  StatusCode HadronOriginClassifier::initialize() {
    ATH_MSG_INFO("Initialize " );
    ATH_MSG_INFO("DSID " << m_DSID );

    ATH_CHECK(m_mcName.initialize());
    
    static const std::vector<Sample> samples = {
      // all Herwig++/Herwig7 showered samples
      {346346, 346348, GEN_id::HerwigPP},
      {410003, GEN_id::HerwigPP}, {410008, GEN_id::HerwigPP}, //aMC@NLO+Hpp
      {410004, GEN_id::HerwigPP}, {410163, GEN_id::HerwigPP}, //Powheg+Hpp
      {410232, 410233, GEN_id::HerwigPP}, //first attempt for Powheg+H7 / aMC@NLO+H7
      {410525, 410530, GEN_id::HerwigPP}, //New Powheg+H7 samples
      {407037, 407040, GEN_id::HerwigPP}, //Powheg+Hpp MET/HT sliced
      {410536, 410537, GEN_id::HerwigPP}, {410245, GEN_id::HerwigPP}, //aMC@NLO+H++ , ttbb
      {410557, 410559, GEN_id::HerwigPP}, // new Powheg+H7, mc16
      {411082, 411090, GEN_id::HerwigPP}, //Powheg+H7 HF-filtered
      {407354, 407356, GEN_id::HerwigPP}, //Powheg+H7 ttbar HT-filtered
      {411233, 411234, GEN_id::HerwigPP}, //Powheg+H7.1.3 ttbar
      {411316, GEN_id::HerwigPP}, //Powheg+H7 allhad ttbar
      {411329, 411334, GEN_id::HerwigPP}, //Powheg+H7.1.3 ttbar HF-filtered
      {411335, 411337, GEN_id::HerwigPP}, //Powheg+H7.1.3 ttbar HT-filtered
      {412116, 412117, GEN_id::HerwigPP}, //amc@NLO+H7.1.3 ttbar
      {504329, GEN_id::HerwigPP}, {504333, GEN_id::HerwigPP}, {504341, GEN_id::HerwigPP}, //amc@NLO+H7.2.1 refined ttZ
      {601239, 601240, GEN_id::HerwigPP},
      {601668, GEN_id::HerwigPP},
      {603905, 603906, GEN_id::HerwigPP}, // ttbb Powheg+H7 dilep, ljet, allhad
      {504337, GEN_id::HerwigPP}, {504345, GEN_id::HerwigPP}, // aMC@NLO+H7 ttZ
      {526034, GEN_id::HerwigPP}, // aMC@NLO+H7 4tops
      {600666, 600667, GEN_id::HerwigPP}, // Powheg+H7 ttbar H7UE
      {601414, 601415, GEN_id::HerwigPP}, // Powheg+H7 ttbar A14
      {602635, 602635, GEN_id::HerwigPP}, // Powheg+H7 ttH PDF4LHC21
      {602846, 602849, GEN_id::HerwigPP}, // Powheg+H7 ttW NNPDF30NLO EW

      // all Pythia8 showered samples
      {304014, GEN_id::Pythia8}, // amc@NLO+P8 3top
      {346229, 346234, GEN_id::Pythia8}, // amc@NLO+P8 tHjb
      {410006, GEN_id::Pythia8}, //Powheg+P8 old main31
      {410081, GEN_id::Pythia8}, //amc@NLO+P8 ttV
      {410500, GEN_id::Pythia8}, //Powheg+P8 new main31, hdamp=mt
      {410501, 410508, GEN_id::Pythia8}, //Powheg+P8 new main31, hdamp=1.5m // Boosted samples are included 410507 410508
      {410511, 410524, GEN_id::Pythia8}, //Powheg+P8 new main31, hdamp=1.5mt, radiation systematics
      {410531, 410535, GEN_id::Pythia8}, //Powheg+P8 allhad samples
      {346343, 346345, GEN_id::Pythia8}, //Powheg+P8 ttH
      {412123, GEN_id::Pythia8}, // MG+P8 ttW
      {410155, GEN_id::Pythia8}, // aMC@NlO+P8 ttW
      {410159, 410160, GEN_id::Pythia8}, //aMC@NLO+P8, old settings
      {410218, 410220, GEN_id::Pythia8}, // aMC@NlO+P8 ttZ
      {410276, 410278, GEN_id::Pythia8}, // aMC@NlO+P8 ttZ_lowMass
      {410225, 410227, GEN_id::Pythia8}, {410274, 410275, GEN_id::Pythia8}, //aMC@NLO+P8, new settings
      {410568, 410569, GEN_id::Pythia8}, // nonallhad boosted c-filtered
      {410244, GEN_id::Pythia8}, //aMC@NLO+P8, ttbb (old)
      {410441, 410442, GEN_id::Pythia8}, //new aMC@NLO+P8 mc16, new shower starting scale
      {410464, 410466, GEN_id::Pythia8}, //new aMC@NLO+P8 mc16, new shower starting scale, no shower weights
      {410470, 410472, GEN_id::Pythia8}, {410480, 410482, GEN_id::Pythia8}, //new Powheg+P8 mc16
      {410452, GEN_id::Pythia8}, //new aMC@NLO+P8 FxFx mc16
      {411073, 411081, GEN_id::Pythia8}, //Powheg+P8 HF-filtered
      {412043, 412044, GEN_id::Pythia8}, {500326, GEN_id::Pythia8}, //aMC@NLO+P8 4top
      {412066, 412074, GEN_id::Pythia8}, //aMC@NLO+P8 HF-filtered
      {411068, 411070, GEN_id::Pythia8}, //Powheg+P8 ttbb
      {410265, 410267, GEN_id::Pythia8}, //aMC@NLO+P8 ttbb
      {411178, 411180, GEN_id::Pythia8}, {411275, GEN_id::Pythia8}, //Powheg+P8 ttbb OTF production - ATLMCPROD-7240
      {501720, GEN_id::Pythia8}, // aMC@NLO+P8 FxFx ttW
      {600791, 600792, GEN_id::Pythia8}, //Powheg+P8 ttbb - ATLMCPROD-9179
      {600737, 600738, GEN_id::Pythia8}, //Powheg+P8 ttbb - ATLMCPROD-9179
      {601226, 601228, GEN_id::Pythia8}, // Powheg+P8 ttbb bornzerodamp cut 5, ATLMCPROD-9694
      {407342, 407344, GEN_id::Pythia8}, {411391, GEN_id::Pythia8}, //Powheg+P8 ttbar HT-filtered
      {407345, 407347, GEN_id::Pythia8}, //Powheg+P8 ttbar MET-filtered
      {407348, 407350, GEN_id::Pythia8}, //aMC@NLO+P8 ttbar HT-filtered
      {504330, 504332, GEN_id::Pythia8}, {504334, 504336, GEN_id::Pythia8}, {504338, GEN_id::Pythia8}, {504342, 504344, GEN_id::Pythia8}, {504346, GEN_id::Pythia8}, //aMC@NLO+P8 refined ttZ
      {601491, 601492, GEN_id::Pythia8}, //Pow+Py8 ttbar pTHard variations - ATLMCPROD-10168
      {601495, 601498, GEN_id::Pythia8}, //Pow+Py8 ttbar pTHard variations - ATLMCPROD-10168
      {601229, 601230, GEN_id::Pythia8}, // mc23 ttbar dilep, singlelep
      {601237, GEN_id::Pythia8}, // mc23 ttbar allhad
      {601398, 601399, GEN_id::Pythia8}, // mc23 ttbar dilep, singlelep hdamp517p5
      {601491, GEN_id::Pythia8}, {601495, GEN_id::Pythia8}, {601497, GEN_id::Pythia8}, // mc23 ttbar pThard variations, dilep, singlelep, allhad
      {601783, 601784, GEN_id::Pythia8}, // Powheg+P8 ttbb bornzerodamp cut 5 pThard variations - ATLMCPROD-10527
      {603003, 603004, GEN_id::Pythia8}, // Powheg+P8 ttbb nominal and pthard1 allhad
      {603190, 603193, GEN_id::Pythia8}, // Powheg+P8 ttbb nominal and pthard1 dilep, ljet
      {604482, 604483, GEN_id::Pythia8}, // mc23 Powheg+P8 ttbar recoilToTop
      {411369, 411374, GEN_id::Pythia8}, // PP8 ttbar Var1
      {411290, GEN_id::Pythia8}, // PP8 ttbar rb1p05
      {508792, GEN_id::Pythia8}, // aMC@NLO+P8 ttbar smeftsim
      {510203, GEN_id::Pythia8}, // aMC@NLO+P8 4tops
      {522035, 522038, GEN_id::Pythia8}, // aMC@NLO+P8 ttZqq
      {523243, GEN_id::Pythia8}, // aMC@NLO+P8 4tops
      {601356, 601357, GEN_id::Pythia8}, // PP8 ttbar Trec
      {602067, 602072, GEN_id::Pythia8}, // PP8 ttH pthard1/2
      {602637, GEN_id::Pythia8}, // PP8 ttH PDF4LHC21
      {602852, 602853, GEN_id::Pythia8}, // PP8 ttH PDF4LHC21 pthard1
      {602886, 602889, GEN_id::Pythia8}, // PP8 ttW NNPDF23
      {603851, 603854, GEN_id::Pythia8}, // PP8 ggH
      {604224, GEN_id::Pythia8}, // PP8 ttH allhad HTop

      // all Sherpa showered samples
      {410186, 410189, GEN_id::Sherpa}, //Sherpa 2.2.0
      {410249, 410252, GEN_id::Sherpa}, //Sherpa 2.2.1
      {410342, 410347, GEN_id::Sherpa}, //Sherpa 2.2.1 sys
      {410350, 410355, GEN_id::Sherpa}, //Sherpa 2.2.1 sys
      {410357, 410359, GEN_id::Sherpa}, //Sherpa 2.2.1 sys
      {410361, 410367, GEN_id::Sherpa}, //Sherpa 2.2.1 sys
      {410281, 410283, GEN_id::Sherpa}, //Sherpa BFilter
      {410051, GEN_id::Sherpa}, //Sherpa ttbb (ICHEP sample)
      {410323, 410325, GEN_id::Sherpa}, {410369, GEN_id::Sherpa}, //New Sherpa 2.2.1 ttbb
      {364345, 364348, GEN_id::Sherpa}, //Sherpa 2.2.4 (test)
      {410424, 410427, GEN_id::Sherpa}, //Sherpa 2.2.4
      {410661, 410664, GEN_id::Sherpa}, //Sherpa 2.2.4 ttbb
      {421152, 421158, GEN_id::Sherpa}, //Sherpa2.2.8 ttbar
      {413023, GEN_id::Sherpa}, // sherpa 2.2.1 ttZ
      {700000, GEN_id::Sherpa}, // Sherpa 2.2.8 ttW
      {700168, GEN_id::Sherpa}, // Sherpa 2.2.10 ttW
      {700205, GEN_id::Sherpa}, // Sherpa 2.2.10 ttW EWK
      {700309, GEN_id::Sherpa}, // Sherpa 2.2.11 ttZ
      {700051, 700054, GEN_id::Sherpa}, //Sherpa2.2.8 ttbb
      {700121, 700124, GEN_id::Sherpa}, //Sherpa2.2.10 ttbar
      {700164, 700167, GEN_id::Sherpa}, //Sherpa2.2.10 ttbb
      {700807, 700809, GEN_id::Sherpa}, //Sherpa2.2.14 ttbar
      {700659, 700662, GEN_id::Sherpa}, // Sherpa 2.2.12 ttbar maxHTavrgTopPT
      {700712, GEN_id::Sherpa}, // Sherpa 2.2.14 4tops muQHT2
      {700986, 700997, GEN_id::Sherpa}, // Sherpa 2.2.14 ttW
      {701251, 701259, GEN_id::Sherpa}, // Sherpa ttW

    };

    // Linear search for sample and assign properties:
    for (const auto& s : samples) {
      if (m_DSID>=s.low && m_DSID<=s.high) {
        m_GenUsed = s.gen;
        return StatusCode::SUCCESS;
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
    m_GenUsed = GEN_id::Pythia6;

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
          // Check if the hadron's flavour matches the one of the parton.
          if((*it).second != (*itr).first->absPdgId()) continue;
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
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::ReadHandle<xAOD::TruthEventContainer> xTruthEventContainer(m_mcName, ctx);
    if (!xTruthEventContainer.isValid()) {
      ATH_MSG_WARNING("Could not retrieve " <<m_mcName);
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
        int pdgid = part->absPdgId();
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
          // Check the category of the b-quark.
          if(isDirectlyFromWTop(part)){
            partonsOrigin[ part ] = b_from_W; 
          }
          else if(isDirectlyFromTop(part)){
            partonsOrigin[ part ] = b_from_top;
          }
          else if((IsHerwigPP()||IsSherpa())&&isDirectlyFSR(part)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(IsPythia8()&&isDirectlyFSRPythia8(part)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(IsPythia6()&&isDirectlyFSRPythia6(part)){
            partonsOrigin[ part ] = b_FSR;
          }
          else if(IsPythia6()&&isDirectlyMPIPythia6(part)){
            partonsOrigin[ part ] = b_MPI;
          }
          else if(IsPythia8()&&isDirectlyMPIPythia8(part)){
            partonsOrigin[ part ] = b_MPI;
          }
          else if(IsSherpa()&&isDirectlyMPISherpa(part)){
            partonsOrigin[ part ] = b_MPI;
          }
        }
        if(iscquark){
          // In this case, the parton is a c-quark.
          // Check the category of the b-quark.
          if(isDirectlyFromWTop(part)){
            partonsOrigin[ part ] = c_from_W;
          }
          else if(isDirectlyFromTop(part)){
            partonsOrigin[ part ] = c_from_top;
          }
          else if((IsHerwigPP()&&IsSherpa())&&isDirectlyFSR(part)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(IsPythia8()&&isDirectlyFSRPythia8(part)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(IsPythia6()&&isDirectlyFSRPythia6(part)){
            partonsOrigin[ part ] = c_FSR;
          }
          else if(IsPythia6()&&isDirectlyMPIPythia6(part)){
            partonsOrigin[ part ] = c_MPI;
          }
          else if(IsPythia8()&&isDirectlyMPIPythia8(part)){
            partonsOrigin[ part ] = c_MPI;
          }
          else if(IsSherpa()&&isDirectlyMPISherpa(part)){
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

  bool HadronOriginClassifier::isFromTop(const xAOD::TruthParticle* part) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part);
    // Check if this parent comes from the top with function isDirectlyFromTop.
    return isDirectlyFromTop(initpart);
  }

  // Define the function isDirectlyFromTop that indicates if a particle comes from the direct decay of top.
  bool HadronOriginClassifier::isDirectlyFromTop(const xAOD::TruthParticle* part) {
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part || !part->nParents()) return false;
    // Go through the parents of the particle.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      // If the i-th parent is a top, then return true
      if( MC::isTop(parent) ) return true;
    }
    // If a top is no the parent, then return false.
    return false;
  }

  // Define the function isFromWTop that indicates if a particle comes from the decay chain t->Wb.

  bool HadronOriginClassifier::isFromWTop(const xAOD::TruthParticle* part) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part);
    return isDirectlyFromWTop(initpart);
  }

  // Define the function isDirectlyFromWTop that indicates if a particle comes from the direct decay of a W from a top.
  bool HadronOriginClassifier::isDirectlyFromWTop(const xAOD::TruthParticle * part) const{
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part || !part->nParents()) return false;
    // Use a for to go though the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Get the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( MC::isW(parent)){
        if( isFromTop(parent) ) return true;
      }
    }
    // In this case, none of the parents of the particle is a W from top.
    // Hence, return false.
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFromGluonQuark(const xAOD::TruthParticle* part) {
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( MC::isPhoton(parent) || parent->absPdgId()<MC::BQUARK ) return true;
    }
    return false;
  }

  bool HadronOriginClassifier::isFromGluonQuark(const xAOD::TruthParticle* part) const{
    const xAOD::TruthParticle* initpart = findInitial(part);
    return isDirectlyFromGluonQuark(initpart);
  }

  bool HadronOriginClassifier::isDirectlyFSRPythia6(const xAOD::TruthParticle * part) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(!MC::isW(parent)) continue;
      if(MC::isCharm(part)){
        //trick to get at least 50% of PowhegPythia c from FSR
        if(part->pdgId()==-(parent->pdgId())/6){
          if( isFromGluonQuark(parent) ) return true;
        }
      }
      else{
        if( isFromGluonQuark(parent) ) return true;
      }
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFSR(const xAOD::TruthParticle * part) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( MC::isPhoton(parent) || MC::isGluon(parent) ){
        if( isFromQuarkTop( parent ) ) return true;
      }
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyFromQuarkTop(const xAOD::TruthParticle* part) const{
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( parent->absPdgId()<MC::TQUARK ) {
        if(isFromTop(parent)){
          return true;
        }
        else if(isFromWTop(parent)){
          return true;
        }
      }
    }

    return false;
  }

  bool HadronOriginClassifier::isFromQuarkTop(const xAOD::TruthParticle* part) const{
    const xAOD::TruthParticle* initpart = findInitial(part);
    return isDirectlyFromQuarkTop(initpart);
  }

  // Define the function isDirectlyFSRPythia8 that indicates if a particle comes from Final State Radiation in samples generated with Pythia8.

  bool HadronOriginClassifier::isDirectlyFSRPythia8(const xAOD::TruthParticle * part) const{
    // First, check if the particle has parents and return false if it does not.
    if(!part->nParents()) return false;
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){

      // Extract the i-th parent.

      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( MC::isPhoton(parent) || MC::isGluon(parent) ){
        if( isFromQuarkTopPythia8( parent ) ) return true;
      }
    }
    // In this case, no parent from the particle is a gluon or a photon coming from a top
    // Hence, the particle is not from FSR and false is not returned.
    return false;
  }

  // Define the function isDirectlyFromQuarkTopPythia8 that indicates if a particle comes from direct decay of the top in samples generated with Pythia8.
  bool HadronOriginClassifier::isDirectlyFromQuarkTopPythia8(const xAOD::TruthParticle* part) const{
    // First, make sure the consdired particle has a non-null pointer and it has parents.
    // Otherwise, return false.
    if(!part->nParents()) return false;
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      // Check if the parent is a quark different from the top.
      if( parent->absPdgId()<MC::TQUARK ) {
        // In this case, the parent is a quark different from top.
        // Check if it comes from the decay chain of the t->Wb.
        // If it is the case, return true.
        if(isFromWTop(parent)){
          return true;
        }
      }
    }
    // In this case, any of the parents of the particle comes from t->Wb chaing.
    // Hence, the particle does not come from the top directly and false is returned.
    return false;
  }

  // Define the function isFromQuarkTopPythia8 that indicates if a particle comes from top in samples generated with Pythia8.
  bool HadronOriginClassifier::isFromQuarkTopPythia8(const xAOD::TruthParticle* part) const{
    // Find the first parent of the considered particle that is different from the particle.
    const xAOD::TruthParticle* initpart = findInitial(part);
    // Check if this parent comes from the top with function isDirectlyFromQuarkTopPythia8.
    return isDirectlyFromQuarkTopPythia8(initpart);
  }

  bool HadronOriginClassifier::isDirectlyMPIPythia6(const xAOD::TruthParticle * part) {
    if(!part->nParents()) return false;
    for(unsigned int i=0; i<part->nParents(); ++i){
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if( parent->absPdgId() == MC::PROTON && MC::isPhysical(part) ) return true;
    }
    return false;
  }

  bool HadronOriginClassifier::isDirectlyMPIPythia8(const xAOD::TruthParticle * part) const{
    const xAOD::TruthParticle* initpart = findInitial(part);
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

  // Define the function findInitial which finds the first parent of a particle that is not the particle itself.
  const xAOD::TruthParticle*  HadronOriginClassifier::findInitial(const xAOD::TruthParticle* part,  std::shared_ptr<std::set<const xAOD::TruthParticle*>> checked ) const{
    // If the particle has no parent, return the particle.
    if(!part->nParents()) return part;
    if (!checked) checked = std::make_shared<std::set<const xAOD::TruthParticle*>>();
    // Use a for to go through the parents.
    for(unsigned int i=0; i<part->nParents(); ++i){
      // Extract the i-th parent.
      const xAOD::TruthParticle* parent = part->parent(i);
      if(!parent) continue;
      if(checked->count(parent)) continue;
      checked->insert(parent);
      // If the parent has the same pdgId as the particle, then it means that the parent is the same as the considered particle.
      // This happens if the particle irradiates for example.
      // In this case, try to look for the first parent of i-th parent that is being considered.
      // Repeat the process until you find a particle different from the considred one or that has no parent.

      if( part->pdgId() == parent->pdgId() ){
        return findInitial(parent, checked);
      }
    }
    // In this case, no parent different from the considered particle has been found.
    // Hence, return the particle.
    return part;
  }

}//namespace
