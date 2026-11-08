/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_TRUTHCLASSES_H
#define TRUTHUTILS_TRUTHCLASSES_H
#include <vector>
#include <string>
namespace MCTruthPartClassifier {
  /**
   * @brief Type of a truth particle, as assigned by MCTruthClassifier::particleTruthClassifier
   * (the first member of the returned pair).
   *
   * Lepton and photon types come in four flavours, decided from the particle's ParticleOrigin
   * (see defTypeOfElectron, defTypeOfMuon, defTypeOfTau, defTypeOfPhoton in TruthClassifiers.h):
   *  - Unknown<X>: the origin could not be determined (ParticleOrigin::NonDefined);
   *  - Iso<X>:     isolated/prompt, i.e. from the hard process (W, Z, top, Higgs, SUSY, other BSM, ...),
   *                or a single particle gun;
   *  - NonIso<X>:  non-isolated, from the decay of a heavy-flavour hadron, a tau or a muon,
   *                or from a quark weak decay;
   *  - Bkg<X>:     background, i.e. everything else (conversions, Dalitz decays, material
   *                interactions, light hadron decays, ...).
   * Particles for which the classifier does not even try to find an origin get one of the
   * generic types (Hadron ... UnknownJet), or Unknown.
   */
  // cppcheck-suppress syntaxError; cppcheck 2.16 bug (https://sourceforge.net/p/cppcheck/discussion/general/thread/44654d46c3)
  enum ParticleType : unsigned int {
    Unknown           =  0, ///< Not classified: not a lepton, photon, neutrino or hadron (e.g. a quark, a gluon, a boson), or no result
    UnknownElectron   =  1, ///< Electron with undefined origin (no production vertex, or origin could not be traced)
    IsoElectron       =  2, ///< Isolated (prompt) electron, e.g. from W/Z/H/top/BSM decay, or a single-electron particle gun
    NonIsoElectron    =  3, ///< Non-isolated electron from J/psi, B/D hadron, tau, muon or quark weak decay
    BkgElectron       =  4, ///< Background electron: photon conversion, Dalitz decay, material interaction, light-hadron decay, ...
    UnknownMuon       =  5, ///< Muon with undefined origin
    IsoMuon           =  6, ///< Isolated (prompt) muon
    NonIsoMuon        =  7, ///< Non-isolated muon from J/psi, B/D hadron, tau or quark weak decay
    BkgMuon           =  8, ///< Background muon: pion/kaon decay in flight and other remaining cases
    UnknownTau        =  9, ///< Tau with undefined origin
    IsoTau            =  10, ///< Isolated (prompt) tau
    NonIsoTau         =  11, ///< Non-isolated tau from J/psi, B/D hadron or quark weak decay
    BkgTau            =  12, ///< Background tau (remaining cases)
    UnknownPhoton     =  13, ///< Photon with undefined origin
    IsoPhoton         =  14, ///< Isolated (prompt) photon: from W/Z/H/BSM decay, a hard-process prompt photon, or a single-photon gun
    NonIsoPhoton      =  15, ///< Non-isolated photon: initial/final-state radiation, or from tau, muon or right-handed neutrino/W_R decay
    BkgPhoton         =  16, ///< Background photon: bremsstrahlung, pi0 decay, hadron decay, nuclear reaction, underlying-event photon, ...
    Hadron            =  17, ///< Any hadron (neither the beam nor a nuclear fragment); the origin is not studied further
    Neutrino          =  18, ///< Neutrino (an origin is determined for it, see ParticleOrigin)
    NuclFrag          =  19, ///< Nuclear fragment: a nucleus other than a proton, produced in a material interaction
    NonPrimary        =  20, ///< Not a primary particle: a copy produced by the simulation (e.g. Geant4) with no production vertex, or the re-generated copy of its own mother
    GenParticle       =  21, ///< Generator-level intermediate particle (decayed, or otherwise not a final-state candidate) that is not classified further
    SUSYParticle      =  22, ///< Stable SUSY particle (e.g. the LSP)
    OtherBSMParticle  =  39, ///< Stable non-SUSY BSM particle (added later, hence the out-of-order value)
    BBbarMesonPart    =  23, ///< Hidden-bottom meson (e.g. Upsilon, chi_b)
    BottomMesonPart   =  24, ///< Open-bottom meson (B, B_s, B_c)
    CCbarMesonPart    =  25, ///< Hidden-charm meson (e.g. J/psi, chi_c)
    CharmedMesonPart  =  26, ///< Open-charm meson (D, D_s)
    BottomBaryonPart  =  27, ///< Baryon containing a b quark
    CharmedBaryonPart =  28, ///< Baryon containing a c quark (and no b)
    StrangeBaryonPart =  29, ///< Baryon containing an s quark (and no c, b)
    LightBaryonPart   =  30, ///< Baryon made of u and d quarks only
    StrangeMesonPart  =  31, ///< Meson containing an s quark (and no c, b), e.g. a kaon
    LightMesonPart    =  32, ///< Meson made of u and d quarks only, e.g. a pion
    BJet              =  33, ///< Jet containing a b-hadron (jet overload of the classifier; DR-matched or constituent truth particles)
    CJet              =  34, ///< Jet with a c-hadron and no b-hadron (jet overload of the classifier)
    LJet              =  35, ///< Jet with only light/strange hadrons (jet overload of the classifier)
    GJet              =  36, ///< UNUSED: never assigned or checked anywhere in Athena (gluon jet)
    TauJet            =  37, ///< UNUSED: never assigned or checked anywhere in Athena (hadronic tau jet)
    UnknownJet        =  38  ///< Jet with no classifiable hadron among its constituents (also the initial value of the jet classifier)
  };

  /**
   * @brief Origin of a truth particle, as assigned by MCTruthClassifier::particleTruthClassifier
   * (the second member of the returned pair).
   *
   * For leptons and photons it names the process or the particle whose decay produced them,
   * found by walking up the history to the first ancestor that is not a copy of the same
   * particle (and skipping through intermediate W, tau and muon decays). Where an
   * origin is a hadron class (LightMeson ... BBbarMeson), it is the class of that ancestor.
   * Sherpa and MadGraph records, which lack explicit intermediate bosons, are recognised
   * from the multiplicity of the hard-process vertex.
   */
  enum ParticleOrigin : unsigned int {
    NonDefined    = 0,  ///< Origin could not be determined (no production vertex, no ancestor, or particle not classified)

    SingleElec    = 1,  ///< Single-electron particle gun: production vertex with no mother and one outgoing particle
    SingleMuon    = 2,  ///< Single-muon particle gun
    SinglePhot    = 3,  ///< Single-photon particle gun
    SingleTau     = 4,  ///< Single-tau particle gun

    PhotonConv    = 5,  ///< Photon conversion (gamma -> e+ e-, or gamma -> mu+ mu- for muons)
    DalitzDec     = 6,  ///< Dalitz decay of a pi0 (pi0 -> gamma e+ e-, or pi0 -> e+ e- e+ e-)
    ElMagProc     = 7,  ///< Electromagnetic or nuclear material process: bremsstrahlung/pair production in the detector, Compton scattering, nuclear photo-fission, hadron-nucleus interaction
    Mu            = 8,  ///< Decay (or radiation) of a muon
    TauLep        = 9,  ///< Decay (or radiation) of a tau lepton that is not itself prompt
    top           = 10, ///< Decay of a top quark (including through W)
    QuarkWeakDec  = 11, ///< Weak decay of a quark: q -> q' l nu with a single other quark in the final state
    WBoson        = 12, ///< W boson decay (including Sherpa W records and W+gamma)
    ZBoson        = 13, ///< Z boson (or Z/gamma*) decay (including Sherpa Z records and Z+gamma)
    Higgs         = 14, ///< Higgs boson decay (SM Higgs, PDG 25)
    HiggsMSSM     = 15, ///< Decay of an extra (MSSM/2HDM) Higgs boson: H (35), A (36) or H+ (37)
    HeavyBoson    = 16, ///< Decay of a heavy gauge boson Z', Z'', W'+ (PDG 32, 33, 34), or a KK graviton (5100039) in the photon case
    WBosonLRSM    = 17, ///< Decay of the W_R boson of the left-right symmetric model (Pythia-specific PDG 9900024)
    NuREle        = 18, ///< Decay of a right-handed electron neutrino (Pythia-specific PDG 9900012), or of a W_R decaying to it
    NuRMu         = 19, ///< Decay of a right-handed muon neutrino (Pythia-specific PDG 9900014)
    NuRTau        = 20, ///< Decay of a right-handed tau neutrino (Pythia-specific PDG 9900016)
    LQ            = 21, ///< Leptoquark: the ancestor is a leptoquark, or the decay vertex contains one

    SUSY          = 22, ///< Decay of a SUSY particle
    OtherBSM      = 46, ///< Decay of a non-SUSY BSM particle (added later, hence the out-of-order value)
    LightMeson    = 23, ///< Decay of a light (u, d only) meson
    StrangeMeson  = 24, ///< Decay of a meson containing s quark (e.g. kaon)
    CharmedMeson  = 25, ///< Decay of an open-charm meson
    BottomMeson   = 26, ///< Decay of an open-bottom meson
    CCbarMeson    = 27, ///< Decay of a hidden-charm meson other than J/psi
    JPsi          = 28, ///< Decay of a J/psi
    BBbarMeson    = 29, ///< Decay of a hidden-bottom meson
    LightBaryon   = 30, ///< Decay of a light (u, d only) baryon
    StrangeBaryon = 31, ///< Decay of a strange baryon
    CharmedBaryon = 32, ///< Decay of a charmed baryon
    BottomBaryon  = 33, ///< Decay of a bottom baryon
    PionDecay     = 34, ///< Muon from pi -> mu nu in flight
    KaonDecay     = 35, ///< Muon from K -> mu nu in flight
    BremPhot      = 36, ///< Bremsstrahlung photon: the mother continues as a copy of itself plus this photon
    PromptPhot    = 37, ///< Prompt (direct) photon of the hard process, e.g. Pythia8 gamma+jet
    UndrPhot      = 38, ///< "Underlying" photon: radiated from a quark or gluon in the parton shower/underlying event
    ISRPhot       = 39, ///< Initial-state radiation photon (quark/gluon mother whose copy is a Herwig cluster)
    FSRPhot       = 40, ///< Final-state radiation photon, e.g. from the leptons of W/Z/H decays or from Photos
    NucReact      = 41, ///< Photon from a nuclear reaction in the material
    PiZero        = 42, ///< Photon from a pi0 decay
    DiBoson       = 43, ///< Two-boson (WW, WZ, ZZ) production recorded without explicit bosons: four leptons/neutrinos in the hard-process vertex
    ZorHeavyBoson = 44, ///< UNUSED: never returned by the classifier; only compared against in defTypeOfElectron/Muon/Tau
    MultiBoson    = 47, ///< Three or more bosons (VVV, 4-top with leptonic W decays): six or eight leptons/neutrinos in the hard-process vertex
    QCD           = 45  ///< Jet origin: a quark or gluon mother (see defJetOrig); only assigned by the jet classifier, never to leptons or photons
  };

  /**
   * @brief Outcome of a (simulated) truth particle: what happened to it after being produced,
   * determined from its decay or end vertex (second member of IMCTruthClassifier::Info).
   * The defOutComeOf<X> functions in TruthClassifiers.h assign it for electrons, muons, taus and photons.
   */
  enum ParticleOutCome : unsigned int {
    NonDefinedOutCome  =  0, ///< UNUSED: never assigned; the default of IMCTruthClassifier::Info::particleOutCome is UnknownOutCome
    UnknownOutCome     =  1, ///< End vertex exists, but its content matches none of the patterns below
    UnConverted        =  2, ///< Photon without an end vertex: it did not convert
    Converted          =  3, ///< Photon converted: end vertex has exactly one e+ and one e-
    NonInteract        =  4, ///< Particle without an end vertex (survived the detector) -- also used for neutrinos
    NuclInteraction    =  5, ///< Interaction with nuclei: the end vertex contains hadrons, nucleons or nuclear fragments
    ElectrMagInter     =  6, ///< Electron that only scattered electromagnetically: its end vertex has one electron and nothing else
    DecaytoElectron    =  7, ///< Muon decayed to e nu_e nu_mu, or tau decayed to e nu nu (with optional photons)
    DecaytoMuon        =  8, ///< Tau decayed to mu nu nu (with optional photons)
    OneProng           =  9, ///< Tau decayed hadronically with one charged pion or kaon
    ThreeProng         = 10, ///< Tau decayed hadronically with three charged pions or kaons
    FiveProng          = 11  ///< Tau decayed hadronically with five charged pions or kaons
  };



  /**
   * @brief Human-readable names for the values of ParticleType, ParticleOrigin and ParticleOutCome.
   *
   * Each vector is indexed by the numeric value of the corresponding enum, so
   * `sParticleType[IsoElectron]` is `"IsoElectron"`. The order of the strings must therefore follow
   * the enum values, not the order in which the enumerators are written above. Use it to print or
   * label classifier results, or to convert a name from a configuration string back to its value by
   * finding it in the vector (as ParticleLevelIsolationAlg does for sParticleType); there is no bounds checking, so an out-of-range value is undefined behaviour.
   */
  struct ParticleDef {
    /** Names of the ParticleType values, indexed by value (0 = "Unknown" ... 39 = "OtherBSMParticle").
     *  Used by ParticleLevelIsolationAlg (TruthParticleLevelAnalysisAlgorithms) to turn its `checkType`
     *  string property into a ParticleType, taking the index of the matching entry as the enum value.
     *  This relies on every entry sitting at the index of its enumerator, so keep them in sync. */
      std::vector<std::string> sParticleType{
      "Unknown",
      "UnknownElectron",
      "IsoElectron",
      "NonIsoElectron",
      "BkgElectron",
      "UnknownMuon",
      "IsoMuon",
      "NonIsoMuon",
      "BkgMuon",
      "UnknownTau",
      "IsoTau",
      "NonIsoTau",
      "BkgTau",
      "UnknownPhoton",
      "IsoPhoton",
      "NonIsoPhoton",
      "BkgPhoton",
      "Hadron",
      "Neutrino",
      "NuclFrag",
      "NonPrimary",
      "GenParticle",
      "SUSYParticle",
      "BBbarMesonPart",
      "BottomMesonPart",
      "CCbarMesonPart",
      "CharmedMesonPart",
      "BottomBaryonPart",
      "CharmedBaryonPart",
      "StrangeBaryonPart",
      "LightBaryonPart",
      "StrangeMesonPart",
      "LightMesonPart",
      "BJet",
      "CJet",
      "LJet",
      "GJet",
      "TauJet",
      "UnknownJet",
      "OtherBSMParticle"};

      /** Names of the ParticleOrigin values, indexed by value (0 = "NonDefined" ... 47 = "MultiBoson"). Not used anywhere else in Athena at the moment.
       Caution: entry 16 is "WZMSSM", while the enumerator with that value is HeavyBoson, so a name lookup
       as done for sParticleType would not find "HeavyBoson" and would map "WZMSSM" to HeavyBoson. */
      std::vector<std::string> sParticleOrigin{
      "NonDefined",
      "SingleElec",
      "SingleMuon",
      "SinglePhot",
      "SingleTau",
      "PhotonConv",
      "DalitzDec",
      "ElMagProc",
      "Mu",
      "TauLep",
      "top",
      "QuarkWeakDec",
      "WBoson",
      "ZBoson",
      "Higgs",
      "HiggsMSSM",
      "WZMSSM",
      "WBosonLRSM",
      "NuREle",
      "NuRMu",
      "NuRTau",
      "LQ",
      "SUSY",
      "LightMeson",
      "StrangeMeson",
      "CharmedMeson",
      "BottomMeson",
      "CCbarMeson",
      "JPsi",
      "BBbarMeson",
      "LightBaryon",
      "StrangeBaryon",
      "CharmedBaryon",
      "BottomBaryon",
      "PionDecay",
      "KaonDecay",
      "BremPhot",
      "PromptPhot",
      "UndrPhot",
      "ISRPhot",
      "FSRPhot",
      "NucReact",
      "PiZero",
      "DiBoson",
      "ZorHeavyBoson",
      "QCD",
      "OtherBSM",
      "MultiBoson"};
  };
}

#endif
