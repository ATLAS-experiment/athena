/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRUTHUTILS_ATLASPID_H
#define TRUTHUTILS_ATLASPID_H
#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <cstdlib>
/** Implementation of classification functions according to PDG2022.
 *  https://pdg.lbl.gov/2023/reviews/rpp2022-rev-monte-carlo-numbering.pdf
 * This code is also available at https://gitlab.cern.ch/averbyts/atlaspid
 */

class DecodedPID: public std::pair<int,std::vector<int>> {
public:
  DecodedPID(int p);
  inline DecodedPID shift(const size_t n) const { return DecodedPID(this->first%int(std::pow(10,ndigits()-n)));}
  inline const int& operator()(const size_t n) const { return this->second.at(n);}
  inline const int& last() const { return this->second.back();}
  inline const int& pid() const { return this->first;}
  inline int max_digit(const  int m,const  int n) const { return *std::max_element(second.rbegin() + m, second.rbegin() + n);}
  inline int min_digit(const  int m,const  int n) const { return *std::min_element(second.rbegin() + m, second.rbegin() + n);}
  inline size_t ndigits() const { return this->second.size();}
};

static const int TABLESIZE = 100;
extern const std::array<int,TABLESIZE> triple_charge;

extern const std::array<int,TABLESIZE> double_spin;


static const int DQUARK = 1;
static const int UQUARK = 2;
static const int SQUARK = 3;
static const int CQUARK = 4;
static const int BQUARK = 5;
static const int TQUARK = 6;
static const int BPRIME = 7; // 4th Generation quark
static const int TPRIME = 8; // 4th Generation quark
static const int QUARK_LIMIT = BPRIME; // Quark pdg_ids less than this are considered in (R-)Hadrons and Diquarks

static const int ELECTRON = 11;
static const int POSITRON = -ELECTRON;
static const int NU_E = 12;
static const int MUON = 13;
static const int NU_MU = 14;
static const int TAU = 15;
static const int NU_TAU = 16;
static const int LPRIME = 17; // 4th Generation charged lepton
static const int NUPRIME = 18; // 4th Generation neutrino

static const int GLUON = 21;
// APID: 9 rather than 21 is used to denote a gluon/gluino in composite states. (From PDG 11g)
static const int COMPOSITEGLUON = 9;
static const int PHOTON = 22;
static const int Z0BOSON = 23;
static const int WPLUSBOSON = 24;
static const int HIGGSBOSON = 25;
static const int ZPRIME = 32; // Z′/Z^0_2
static const int ZDBLPRIME = 33; // Z′′/Z^0_3
static const int WPLUSPRIME = 34; // W ′/W^+_2
static const int HIGGS2 = 35; // H^0/H^0_2  FIXME Any better ideas?
static const int HIGGS3 = 36; // A^0/H^0_3 FIXME Any better ideas?
static const int HIGGSPLUS = 37; // H^+
static const int HIGGSPLUSPLUS = 38; // H^++
static const int GRAVITON = 39;
static const int HIGGS4 = 40; // a^0/H^0_4 FIXME Any better ideas?
static const int LEPTOQUARK = 42;

/// PDG Ids for Mavtop madgraph UFO model found under DarkX. The
/// mavtop is a vector-like top partner with coupling to a dark photon.
/// Theory paper: https://arxiv.org/abs/1904.05893
/// Pheno paper: https://arxiv.org/pdf/2112.08425
static const int DARKPHOTON = 60000;
static const int MAVTOP = 60001;

static const int PIPLUS = 211;
static const int PIMINUS = -PIPLUS;
static const int PI0 = 111;
static const int K0L = 130;

static const int K0S = 310;
static const int K0 = 311;
static const int KPLUS = 321;
static const int DPLUS = 411;
static const int DSTAR = 413;
static const int D0 = 421;
static const int DSPLUS = 431;
static const int JPSI = 443;
static const int B0 = 511;
static const int BCPLUS = 541;
static const int PROTON = 2212;
static const int NEUTRON = 2112;
static const int LAMBDA0 = 3122;
static const int LAMBDACPLUS = 4122;
static const int LAMBDAB0 = 5122;
static const int PSI2S = 20443;

/// PDG Rule 12:
/// Generator defined PDG ID values for right handed neutrinos and
/// corresponding W+ boson from a Left-Right symmetric Standard Model
/// extension. (Defined for some MadGraph+Pythia8 samples and
/// referenced in MCTruthClassifierGen.cxx)
static const int  RH_NU_E = 9900012;
static const int  RH_NU_MU = 9900014;
static const int  RH_NU_TAU = 9900016;
static const int  WBOSON_LRSM = 9900024;

static const int LEAD = 1000822080;
static const int OXYGEN = 1000080160;
static const int NEON = 1000100200;
static const int HELIUM =  1000020040;

/// PDG rule 8:
/// The pomeron and odderon trajectories and a generic reggeon trajectory
/// of states in QCD areassigned codes 990, 9990, and 110 respectively
static const int POMERON = 990;
static const int ODDERON = 9990;
static const int REGGEON = 110;

/// PDG rule 10:
/// Codes 81–100 are reserved for generator-specific pseudoparticles and concepts.
/// Codes 901–930, 1901–1930, 2901–2930, and 3901–3930 are for additional components
/// of Standard Modelparton distribution functions, where the latter three ranges are intended
/// to distinguish left/right/ longitudinal components. Codes 998 and 999 are reserved for GEANT tracking purposes.
static const int GEANTINOPLUS = 998;
static const int GEANTINO0 = 999;


/// PDG rule 2:
/// Quarks and leptons are numbered consecutively starting from 1 and 11
/// respectively; to do this they are first ordered by family and within
/// families by weak isospin.
/// APID: the fourth generation quarks are quarks.
inline bool isQuark(const int& p) { return p != 0 && (std::abs(p) <= TPRIME || std::abs(p) == MAVTOP);}
inline bool isQuark(const DecodedPID& p){ return isQuark(p.pid()); }
template<class T> inline bool isQuark(const T& p) {return isQuark(p->pdg_id());}

// APID: the fourth generation quarks are not standard model quarks
inline bool isSMQuark(const int& p) { return p != 0 && std::abs(p) <= TQUARK;}
inline bool isSMQuark(const DecodedPID& p){ return isSMQuark(p.pid()); }
template<class T> inline bool isSMQuark(const T& p) {return isSMQuark(p->pdg_id());}

inline bool isStrange(const int& p){ return std::abs(p) == SQUARK;}
template<class T> inline bool isStrange(const T& p) {return isStrange(p->pdg_id());}

inline bool isCharm(const int& p){ return std::abs(p) == CQUARK;}
template<class T> inline bool isCharm(const T& p){return isCharm(p->pdg_id());}

inline bool isBottom(const int& p){ return std::abs(p) == BQUARK;}
template<class T> inline bool isBottom(const T& p){return isBottom(p->pdg_id());}

inline bool isTop(const int& p){ return std::abs(p) == TQUARK;}
template<class T> inline bool isTop(const T& p){return isTop(p->pdg_id());}

/// APID: the fourth generation leptons are leptons.
inline bool isLepton(const int& p){ auto sp = std::abs(p); return sp >= ELECTRON && sp <= NUPRIME; }
inline bool isLepton(const DecodedPID& p){ return isLepton(p.pid()); }
template<class T> inline bool isLepton(const T& p){return isLepton(p->pdg_id());}

/// APID: the fourth generation leptons are not standard model leptons.
inline bool isSMLepton(const int& p){ auto sp = std::abs(p); return sp >= ELECTRON && sp <= NU_TAU; }
inline bool isSMLepton(const DecodedPID& p){ return isSMLepton(p.pid()); }
template<class T> inline bool isSMLepton(const T& p){return isSMLepton(p->pdg_id());}

/// APID: the fourth generation leptons are leptons.
inline bool isChLepton(const int& p){ auto sp = std::abs(p); return sp >= ELECTRON && sp <= LPRIME && sp%2 == 1; }
template<class T> inline bool isChLepton(const T& p){return isChLepton(p->pdg_id());}

inline bool isElectron(const int& p){ return std::abs(p) == ELECTRON;}
template<class T> inline bool isElectron(const T& p){return isElectron(p->pdg_id());}

inline bool isMuon(const int& p){ return std::abs(p) == MUON;}
template<class T> inline bool isMuon(const T& p){return isMuon(p->pdg_id());}

inline bool isTau(const int& p){ return std::abs(p) == TAU;}
template<class T> inline bool isTau(const T& p){return isTau(p->pdg_id());}

/// APID: the fourth generation neutrinos are neutrinos.
inline bool isNeutrino(const int& p){ auto sp = std::abs(p); return sp == NU_E || sp == NU_MU || sp == NU_TAU || sp == NUPRIME;  }
template<class T> inline bool isNeutrino(const T& p){return isNeutrino(p->pdg_id());}

inline bool isSMNeutrino(const int& p){ auto sp = std::abs(p); return sp == NU_E || sp == NU_MU || sp == NU_TAU;  }
template<class T> inline bool isSMNeutrino(const T& p){return isSMNeutrino(p->pdg_id());}

/// Is this a 4th generation fermion?
/// APID: 4th generation fermions are not standard model particles
inline bool isFourthGeneration(const int& p) {return std::abs(p) == BPRIME || std::abs(p) == TPRIME || std::abs(p) == LPRIME || std::abs(p) == NUPRIME;}
template<class T> inline bool isFourthGeneration(const T& p){return isFourthGeneration(p->pdg_id());}

/// PDG rule 4
/// Diquarks have 4-digit numbers with nq1 >= nq2 and nq3 = 0
/// APID: states with top quarks are diquarks
/// APID: states with fourth generation quarks are not diquarks
bool isDiquark(const DecodedPID& p);
inline bool isDiquark(const int& p){ return isDiquark(DecodedPID(p));}
template<class T> inline bool isDiquark(const T& p){return isDiquark(p->pdg_id());}

///Table 43.1
/// PDG rule 5a:
/// The numbers specifying the meson’s quark content conform to the convention
/// nq1= 0 and nq2 >= nq3. The special case K0L is the sole exception to this rule.
/// PDG rule 5C:
/// The special numbers 310 and 130 are given to the K0S and K0L respectively.
/// APID: The special code K0 is used when a generator uses K0S/K0L
/// APID: states with fourth generation quarks are not mesons
bool isMeson(const DecodedPID& p);
inline bool isMeson(const int& p){ return isMeson(DecodedPID(p));}
template<class T> inline bool isMeson(const T& p){return isMeson(p->pdg_id());}

/// Is this a heavy-flavour quarkonium meson?
///
/// @note Original by LHCb in Rivet analysis LHCB_2016_I1504058
///
/// @note phi = s,sbar is not considered quarkonium
bool isQuarkonium(const DecodedPID& p);
inline bool isQuarkonium(const int& p){ return isQuarkonium(DecodedPID(p));}
template<class T> inline bool isQuarkonium(const T& p){return isQuarkonium(p->pdg_id());}

///Table 43.2
/// APID: states with fourth generation quarks are not baryons
bool isBaryon(const DecodedPID& p);
inline bool isBaryon(const int& p){ return isBaryon(DecodedPID(p));}
template<class T> inline bool isBaryon(const T& p){return isBaryon(p->pdg_id());}

/// PDG rule 14
///The 9-digit tetra-quark codes are ±1nrnLnq1nq20nq3nq4nJ. For the particle q1q2 is a diquark and
/// ̄q3 ̄q4 an antidiquark, sorted such that nq1≥nq2, nq3≥nq4, nq1≥nq3, and nq2≥nq4 if nq1=nq3.
///For the antiparticle, given with a negative sign,  ̄q1 ̄q2 is an antidiquark and q3q4 a diquark,
/// with the same sorting except that either nq1>nq3 or nq2>nq4 (so that flavour-diagonal states are particles).
/// The nr, nL, and nJ numbers have the same meaning as for ordinary hadrons.
/// APID: states with fourth generation quarks are not tetraquarks
bool isTetraquark(const DecodedPID& p);
inline bool isTetraquark(const int& p){ return isTetraquark(DecodedPID(p));}
template<class T> inline bool isTetraquark(const T& p){return isTetraquark(p->pdg_id());}

/// PDG rule 15
///The 9-digit penta-quark codes are ±1nrnLnq1nq2nq3nq4nq5nJ, sorted
///such that nq1≥nq2≥nq3≥nq4.  In the particle the first four are
///quarks and the fifth an antiquark while the opposite holds in the
///antiparticle, which is given with a negative sign.  The nr, nL, and
///nJ numbers have the same meaning as for ordinary hadrons.
// APID: states with fourth generation quarks are not pentaquarks
bool isPentaquark(const DecodedPID& p);
inline bool isPentaquark(const int& p){ return isPentaquark(DecodedPID(p));}
template<class T> inline bool isPentaquark(const T& p){return isPentaquark(p->pdg_id());}

// APID Mesons, Baryons, Tetraquarks and Pentaquarks are Hadrons
inline bool isHadron(const DecodedPID& p){ return isMeson(p) || isBaryon(p) || isTetraquark(p) || isPentaquark(p); }
inline bool isHadron(const int& p){ return isHadron(DecodedPID(p));}
template<class T> inline bool isHadron(const T& p){return isHadron(p->pdg_id());}


/// PDG rule 8:
/// The pomeron and odderon trajectories and a generic reggeon trajectory
/// of states in QCD areassigned codes 990, 9990, and 110 respectively
inline bool isTrajectory(const int& p){ return std::abs(p) == POMERON || std::abs(p) == ODDERON || std::abs(p) == REGGEON; }
template<class T> inline bool isTrajectory(const T& p){return isTrajectory(p->pdg_id());}


/// PDG rule 9:
/// Two-digit numbers in the range 21–30 are provided for the Standard
/// Model gauge and Higgs bosons.
/// PDG rule 11b:
/// The graviton and the boson content of a two-Higgs-doublet scenario
/// and of additional SU(2)×U(1) groups are found in the range 31–40.
inline bool isBoson(const int& p){ auto sp = std::abs(p); return sp > 20 && sp < 41; }
inline bool isBoson(const DecodedPID& p){ return isBoson(p.pid()); }
template<class T> inline bool isBoson(const T& p){return isBoson(p->pdg_id());}

inline bool isGluon(const int& p){ return p == GLUON; }
template<class T> inline bool isGluon(const T& p){return isGluon(p->pdg_id());}

inline bool isPhoton(const int& p){ return p == PHOTON; }
template<class T> inline bool isPhoton(const T& p){return isPhoton(p->pdg_id());}

inline bool isZ(const int& p){ return p == Z0BOSON; }
template<class T> inline bool isZ(const T& p){return isZ(p->pdg_id());}

inline bool isW(const int& p){ return std::abs(p) == WPLUSBOSON; }
template<class T> inline bool isW(const T& p){return isW(p->pdg_id());}

/// APID: Additional "Heavy"/"prime" versions of W and Z bosons (Used in MCTruthClassifier)
inline bool isHeavyBoson(const int& p){ return p == ZPRIME || p == ZDBLPRIME || std::abs(p) == WPLUSPRIME; }
template<class T> inline bool isHeavyBoson(const T& p){return isHeavyBoson(p->pdg_id());}

/// APID: HIGGS boson is only one particle.
inline bool isHiggs(const int& p){ return p == HIGGSBOSON; }
template<class T> inline bool isHiggs(const T& p){return isHiggs(p->pdg_id());}

/// APID: Additional Higgs bosons for MSSM (Used in MCTruthClassifier)
inline bool isMSSMHiggs(const int& p){ return p == HIGGS2 || p == HIGGS3 || std::abs(p) == HIGGSPLUS; }
template<class T> inline bool isMSSMHiggs(const T& p){return isMSSMHiggs(p->pdg_id());}

inline bool isGraviton(const int& p){ return p == GRAVITON; }
template<class T> inline bool isGraviton(const T& p) {return isGraviton(p->pdg_id());}

template<class T> inline bool isResonance(const T& p) { return isZ(p) || isW(p) || isHiggs(p) || isTop(p); } // APID: not including t' (pdg_id=8), Z', Z'' and W'+ or BSM Higgs bosons

/// PDG rule 11c:
/// “One-of-a-kind” exotic particles are assigned numbers in the range
/// 41–80. The subrange 61-80 can be used for new heavier fermions in
/// generic models, where partners to the SM fermions would have codes
/// oﬀset by 60. If required, however, other assignments could be
/// made.
inline bool isLeptoQuark(const int& p){ return std::abs(p) == LEPTOQUARK; }
template<class T> inline bool isLeptoQuark(const T& p){return isLeptoQuark(p->pdg_id());}

inline bool isPythia8Specific(const DecodedPID& p){ return (p.ndigits() == 7 && p(0) == 9 && p(1) == 9);}
inline bool isPythia8Specific(const int& p){ return isPythia8Specific(DecodedPID(p));}
template<class T> inline bool isPythia8Specific(const T& p){return isPythia8Specific(p->pdg_id());}

/// PDG Rule 12:
/// APID: Helper function for right-handed neutrino states
/// These are generator defined PDG ID values for right handed
/// neutrinos. (Defined for some MadGraph+Pythia8 samples and
/// referenced in MCTruthClassifierGen.cxx)
inline bool isNeutrinoRH(const int& p){ return (std::abs(p) ==  RH_NU_E || std::abs(p) ==  RH_NU_MU|| std::abs(p) ==  RH_NU_TAU);}
template<class T> inline bool isNeutrinoRH(const T& p){return isNeutrinoRH(p->pdg_id());}

/// Main Table
/// for MC internal use 81–100,901–930,998-999,1901–1930,2901–2930, and 3901–3930
bool isGenSpecific(const int& p);
template<class T> inline bool isGenSpecific(const T& p){return isGenSpecific(p->pdg_id());}

inline bool isGeantino(const int& p){ return (std::abs(p) ==  GEANTINO0 || std::abs(p) ==  GEANTINOPLUS);}
template<class T> inline bool isGeantino(const T& p){return isGeantino(p->pdg_id());}

/// APID: Definition of Glueballs: SM glueballs 99X (X=1,5), 999Y (Y=3,7)
bool isGlueball(const DecodedPID& p);
inline bool isGlueball(const int& p) {  return isGlueball(DecodedPID(p)); }
template<class T> inline bool isGlueball(const T& p) { return isGlueball(p->pdg_id()); }


/// PDG rule 11d
/// Fundamental supersymmetric particles are identified by adding a nonzero n to the particle number. The superpartner
/// of a boson or a left-handed fermion has n = 1 while the superpartner of a right-handed fermion has n = 2. When mixing
/// occurs, such as between the winos and charged Higgsinos to give charginos, or between left and right sfermions, the
/// lighter physical state is given the smaller basis state number.

// APID: Super-partners of standard model quarks only
inline bool isSquark(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && (p(0) == 1 || p(0) == 2) && isSMQuark(pp));
}
inline bool isSquark(const int& p){ return isSquark(DecodedPID(p));}
template<class T> inline bool isSquark(const T& p) { return isSquark(p->pdg_id()); }


// APID: Super-partners of left-handed standard model quarks only
inline bool isSquarkLH(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && p(0) == 1 && isSMQuark(pp));
}
inline bool isSquarkLH(const int& p){ return isSquarkLH(DecodedPID(p));}
template<class T> inline bool isSquarkLH(const T& p) { return isSquarkLH(p->pdg_id()); }


// APID: Super-partners of right-handed standard model quarks only
inline bool isSquarkRH(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && p(0) == 2 && isSMQuark(pp));
}
inline bool isSquarkRH(const int& p){ return isSquarkRH(DecodedPID(p));}
template<class T> inline bool isSquarkRH(const T& p) { return isSquarkRH(p->pdg_id()); }


// APID: Super-partners of standard model leptons only
inline bool isSlepton(const DecodedPID& p){ auto pp = p.shift(1); return (p.ndigits() == 7 && (p(0) == 1 || p(0) == 2) && isSMLepton(pp));}
inline bool isSlepton(const int& p){ return isSlepton(DecodedPID(p));}
template<class T> inline bool isSlepton(const T& p) { return isSlepton(p->pdg_id()); }


// APID: Super-partners of left-handed standard model leptons only
inline bool isSleptonLH(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && p(0) == 1 && isSMLepton(pp));
}
inline bool isSleptonLH(const int& p){ return isSleptonLH(DecodedPID(p));}
template<class T> inline bool isSleptonLH(const T& p) { return isSleptonLH(p->pdg_id()); }


// APID: Super-partners of right-handed standard model leptons only
inline bool isSleptonRH(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && p(0) == 2 && isSMLepton(pp));
}
inline bool isSleptonRH(const int& p){ return isSleptonRH(DecodedPID(p));}
template<class T> inline bool isSleptonRH(const T& p) { return isSleptonRH(p->pdg_id()); }


// APID: Super-partners of gauge bosons including gravitons
inline bool isGaugino(const DecodedPID& p){
  auto pp = p.shift(1); return (p.ndigits() == 7 && p(0) == 1 && isBoson(pp.pid()));
}
inline bool isGaugino(const int& p){ return isGaugino(DecodedPID(p));}
template<class T> inline bool isGaugino(const T& p) { return isGaugino(p->pdg_id()); }


// APID: Super-partners of fundamental particles
inline bool isSuperpartner(const DecodedPID& p){
  return isSlepton(p) || isSquark(p) || isGaugino(p);
}
inline bool isSuperpartner(const int& p){ return isSuperpartner(DecodedPID(p));}
template<class T> inline bool isSuperpartner(const T& p) { return isSuperpartner(p->pdg_id()); }


/// PDG rule 11e
/// Technicolor states have n = 3, with technifermions treated like ordinary fermions. States which are ordinary color singlets
/// have n_r = 0. Color octets have n_r = 1. If a state has non-trivial quantum numbers under the topcolor groups SU(3)1×SU(3)2,
/// the quantum numbers are specified by tech, ij, where i and j are 1 or 2. nLis then 2i+j. The coloron
/// V8, is a heavy gluon color octet and thus is 3100021
bool isTechnicolor(const DecodedPID& p);
inline bool isTechnicolor(const int& p){ return isTechnicolor(DecodedPID(p));}
template<class T> inline bool isTechnicolor(const T& p){return isTechnicolor(p->pdg_id());}

/// PDG rule 11f
/// Excited (composite) quarks and leptons are identified by setting n= 4 and nr= 0
bool isExcited(const DecodedPID& p);
inline bool isExcited(const int& p){ return isExcited(DecodedPID(p));}
template<class T> inline bool isExcited(const T& p){return isExcited(p->pdg_id());}

/// PDG rule 11g:
/// Within several scenarios of new physics, it is possible to have colored particles suﬃciently long-lived for color-singlet hadronic
/// states to form around them. In the context of supersymmetric scenarios, these states are called R-hadrons, since they carry odd
/// R- parity. R-hadron codes, deﬁned here, should be viewed as templates for corresponding codes also in other scenarios, for any
/// long-lived particle that is either an unﬂavored color octet or a ﬂavored color triplet. The R-hadron code is obtained by combining
/// the SUSY particle code with a code for the light degrees of freedom, with as many intermediate zeros removed from the former
/// as required to make place for the latter at the end. (To exemplify, a sparticle n00000n˜q combined with quarks q1 and q2
/// obtains code n00n˜qnq1 nq2 nJ .) Speciﬁcally, the new-particle spin decouples in the limit of large masses, so that the ﬁnal nJ
/// digit is deﬁned by the spin state of the light-quark system alone. An appropriate number of nq digits is used to deﬁne the
/// ordinary-quark content.  As usual, 9 rather than 21 is used to denote a gluon/gluino in composite states. The sign of the hadron
/// agrees with that of the constituent new particle (a color triplet) where there is a distinct new antiparticle, and else is deﬁned as
/// for normal hadrons. Particle names are R with the ﬂavor content as lower index.

/// APID: Definition of R-Glueballs: 100099X (X=1,3), 100999Y (Y=1,5)
/// APID: NB In the current numbering scheme, some states with 2
/// gluinos + gluon or 2 gluons + gluino could have degenerate
/// PDG_IDs.
bool isRGlueball(const DecodedPID& p);
inline bool isRGlueball(const int& p) {  return isRGlueball(DecodedPID(p)); }
template<class T> inline bool isRGlueball(const T& p) { return isRGlueball(p->pdg_id()); }

// APID Define R-Mesons as gluino-quark-antiquark and squark-antiquark bound states (ignore 4th generation squarks/quarks)
// NB Current models only allow gluino-quark-antiquark, stop-antiquark and sbottom-antiquark states
bool isRMeson(const DecodedPID& p);
inline bool isRMeson(const int& p) { return isRMeson(DecodedPID(p)); }
template<class T> inline bool isRMeson(const T& p) { return isRMeson(p->pdg_id()); }

// APID Define R-Baryons as gluino-quark-quark-quark and squark-quark-quark bound states (ignore 4th generation squarks/quarks)
// NB Current models only allow gluino-quark-quark-quark, stop-quark-quark and sbottom-quark-quark states
bool isRBaryon(const DecodedPID& p);
inline bool isRBaryon(const int& p) { return isRBaryon(DecodedPID(p)); }
template<class T> inline bool isRBaryon(const T& p) { return isRBaryon(p->pdg_id()); }


inline bool isRHadron(const DecodedPID& p) {
  return (isRBaryon(p) || isRMeson(p) || isRGlueball(p));
}
inline bool isRHadron(const int& p) { return isRHadron(DecodedPID(p)); }
template<class T> inline bool isRHadron(const T& p) { return isRHadron(p->pdg_id()); }


bool hasSquark(const DecodedPID& p, const int& q);
inline bool hasSquark(const int& p, const int& q){ return hasSquark(DecodedPID(p), q);}
template<class T> inline bool hasSquark(const T& p, const int& q) { return hasSquark(p->pdg_id(), q); }


// APID Define isSUSY to return true for Superpartners of standard model particles and R-Hadrons
// TODO Should this include the MSSM extended Higgs sector??
inline bool isSUSY(const DecodedPID& p){return (isSuperpartner(p) || isRHadron(p));}
inline bool isSUSY(const int& p){ return isSUSY(DecodedPID(p));}
template<class T> inline bool isSUSY(const T& p){return isSUSY(p->pdg_id());}


/// PDG rule 11h
/// A black hole in models with extra dimensions has code 5000040. Kaluza-Klein excitations in models with extra dimensions
/// have n = 5 or n = 6, to distinguish excitations of left-or right-handed fermions or, in case of mixing, the lighter or heavier
/// state (cf. 11d). The non zero nr digit gives the radial excitation number, in scenarios where the level spacing allows these to be
///  distinguished. Should the model also contain supersymmetry, excited SUSY states would be denoted by a nn_r > 0, with n = 1 or 2 as usual.
/// Should some colored states be long-lived enough that hadrons would form around them, the coding strategy of 11g applies, with the initial
/// two nnr digits preserved in the combined code.
inline bool isKK(const DecodedPID& p){return (p.ndigits() == 7 && (p(0) == 5 || p(0) == 6 ) && (p(1) != 9) );}
inline bool isKK(const int& p){ return isKK(DecodedPID(p));}
template<class T> inline bool isKK(const T& p){return isKK(p->pdg_id());}

/// PDG rule 11i
/// Magnetic monopoles and dyons are assumed to have one unit of Dirac monopole charge
/// and a variable integer number nq1nq2 nq3 units of electric charge. Codes 411nq1nq2 nq3 0
/// are then used when the magnetic and electrical charge sign agree and 412nq1nq2 nq3 0
/// when they disagree, with the overall sign of the particle set by the magnetic charge. For
/// now no spin information is provided.
inline bool isMonopole(const DecodedPID& p){return (p.ndigits() == 7 && p(0) == 4 && p(1) == 1  && (p(2) == 1 || p(2) == 2 ) && p(6) == 0);}
inline bool isMonopole(const int& p){ return isMonopole(DecodedPID(p));}
template<class T> inline bool isMonopole(const T& p){return isMonopole(p->pdg_id());}

/// PDG rule 11j:
/// The nature of Dark Matter (DM) is not known, and therefore a definitive
/// classificationis too early. Candidates within specific scenarios are
/// classified therein, such as 1000022 for the lightest neutralino.
/// Generic fundamental states can be given temporary codes in the range 51 - 60,
/// with 51, 52 and 53 reserved for spin 0, 1/2 and 1 ones (this could also be an axion state).
/// Generic mediators of s-channel DM pair creation of annihilation can be given
/// codes 54 and 55 for spin 0 or 1 ones. Separate antiparticles, with negativecodes,
/// may or may not exist. More elaborate new scenarios should be constructed with n= 5 and nr = 9.
bool isDM(const int& p);
template<class T> inline bool isDM(const T& p){return isDM(p->pdg_id());}

/// PDG rule 11k
/// Hidden Valley particles have n = 4 and n_r = 9, and trailing numbers in agreement with their nearest-analog standard particles,
/// as far as possible. Thus 4900021 is the gauge boson g_v of a confining gauge field, 490000n_{q_v} and 490001n_{l_v} fundamental
/// constituents charged or not under this, 4900022 is the γ_v of a non-confining field, and 4900n_{q_{v1}}n_{q_{v2}}n_J a Hidden Valley meson.
bool isHiddenValley(const DecodedPID& p);
inline bool isHiddenValley(const int& p){ return isHiddenValley(DecodedPID(p));}
template<class T> inline bool isHiddenValley(const T& p){return isHiddenValley(p->pdg_id());}

/// In addition, there is a need to identify ”Q-ball” and similar very exotic (multi-charged) particles which may have large, non-integer charge.
/// These particles are assigned the ad-hoc numbering +/-100XXXY0, where the charge is XXX.Y.
/// or +/-200XXYY0, where the charge is XX/YY.
/// The case of +/-200XXYY0 is legacy, see https://gitlab.cern.ch/atlas/athena/-/merge_requests/25862
/// Note that no other quantum numbers besides the charge are considered for these generic multi-charged particles (e.g. isSUSY() is false for them).
/// Such a model was used in previous Run-1 (1301.5272,1504.04188) and Run-2 (1812.03673,2303.13613) ATLAS searches.
inline bool isGenericMultichargedParticle(const DecodedPID& p){return (p.ndigits() == 8 && (p(0) == 1 || p(0) == 2) && p(1) == 0 && p(2) == 0 && p(7) == 0);}
inline bool isGenericMultichargedParticle(const int& p){ return isGenericMultichargedParticle(DecodedPID(p));}
template<class T> inline bool isGenericMultichargedParticle(const T& p){return isGenericMultichargedParticle(p->pdg_id());}

/// PDG rule 16
/// Nuclear codes are given as 10-digit numbers ±10LZZZAAAI.
/// For a (hyper)nucleus consisting of n_p protons, n_n neutrons and
/// n_Λ Λ’s:
/// A = n_p + n_n + n_Λ gives the total baryon number,
/// Z = n_p gives the total charge,
/// L = n_Λ gives the total number of strange quarks.
/// I gives the isomer level, with I= 0 corresponding to the ground
/// state and I > 0 to excitations, see
/// [http://www.nndc.bnl.gov/amdc/web/nubase en.html], where states
/// denoted m, n, p ,q translate to I= 1–4. As examples, the deuteron
/// is 1000010020 and 235U is 1000922350. To avoid ambiguities,
/// nuclear codes should not be applied to a single hadron, like p, n or
/// Λ^0, where quark-contents-based codes already exist.
bool isNucleus(const DecodedPID& p);
inline bool isNucleus(const int& p){ return isNucleus(DecodedPID(p));}
template<class T> inline bool isNucleus(const T& p){return isNucleus(p->pdg_id());}


bool hasQuark(const DecodedPID& p, const int& q);
inline bool hasQuark(const int& p, const int& q){ return hasQuark(DecodedPID(p), q);}
template<class T> inline bool hasQuark(const T& p, const int& q);

template<class T> inline bool hasStrange(const T& p) { return  hasQuark(p,SQUARK); }
template<class T> inline bool hasCharm(const T& p) { return  hasQuark(p,CQUARK); }
template<class T> inline bool hasBottom(const T& p) { return  hasQuark(p,BQUARK); }
template<class T> inline bool hasTop(const T& p) { return  hasQuark(p,TQUARK); }


// APID: The baryon number is defined as:
// B = (1/3)*( n_q - n_{qbar} )
// where n_q⁠ is the number of quarks, and ⁠n_{qbar} is the number of
// antiquarks. By convention, squarks have the same quantum numbers as
// the corresponding quarks (modulo spin and R), so have baryon number
// 1/3.
int baryonNumber3(const DecodedPID& p);
inline int baryonNumber3(const int& p){ return baryonNumber3(DecodedPID(p));}
template<class T> inline int baryonNumber3(const T& p) {return baryonNumber3(p->pdg_id());}

inline double baryonNumber(const DecodedPID& p){ return static_cast<double>(baryonNumber3(p))/3.0;}
inline double baryonNumber(const int& p){ return static_cast<double>(baryonNumber3(DecodedPID(p)))/3.0;}
template<class T> inline double baryonNumber(const T& p) {return baryonNumber(p->pdg_id());}


// APID: The strangeness of a particle is defined as:
// S = − ( n_s − n_{sbar} )
// where n_s represents the number of strange quarks and n_{sbar}
// represents the number of strange antiquarks. By convention, strange
// squarks have the same quantum numbers as strange quarks (modulo
// spin and R), so have strangeness -1.
int strangeness(const DecodedPID& p);
inline int strangeness(const int& p){ return strangeness(DecodedPID(p));}
template<class T> inline int strangeness(const T& p) {return strangeness(p->pdg_id());}


int numberOfLambdas(const DecodedPID& p);
inline int numberOfLambdas(const int& p){ return numberOfLambdas(DecodedPID(p));}
template<class T> inline int numberOfLambdas(const T& p) {return numberOfLambdas(p->pdg_id());}


int numberOfProtons(const DecodedPID& p);
inline int numberOfProtons(const int& p){ return numberOfProtons(DecodedPID(p));}
template<class T> inline int numberOfProtons(const T& p) {return numberOfProtons(p->pdg_id());}


/// APID: graviton and all Higgs extensions are BSM
bool isBSM(const DecodedPID& p);
bool isBSM(const int& p);
template<class T> inline bool isBSM(const T& p){return isBSM(p->pdg_id());}

inline bool isTransportable(const DecodedPID& p){ return isPhoton(p.pid()) || isGeantino(p.pid()) || isHadron(p) || isLepton(p.pid()) || p.pid() == DARKPHOTON;}
inline bool isTransportable(const int& p){ return isTransportable(DecodedPID(p));}
template<class T> inline bool isTransportable(const T& p){return isTransportable(p->pdg_id());}

/// Av: we implement here an ATLAS-sepcific convention: all particles which are 99xxxxx are fine.
bool isValid(const DecodedPID& p);
bool isValid(const int& p);
template<class T> inline bool isValid(const T& p){return isValid(p->pdg_id());}

int leadingQuark(const DecodedPID& p);
inline int leadingQuark(const int& p){ return leadingQuark(DecodedPID(p));}
template<class T> inline int leadingQuark(const T& p) {return leadingQuark(p->pdg_id());}

template<class T> inline bool isLightHadron(const T& p) { auto lq = leadingQuark(p); return  (lq == DQUARK || lq == UQUARK||lq == SQUARK) && isHadron(p); }
template<class T> inline bool isHeavyHadron(const T& p) {  auto lq = leadingQuark(p); return  (lq == CQUARK || lq == BQUARK || lq == TQUARK ) && isHadron(p); }
template<class T> inline bool isStrangeHadron(const T& p) { return  leadingQuark(p) == SQUARK && isHadron(p); }
template<class T> inline bool isCharmHadron(const T& p) { return  leadingQuark(p) == CQUARK && isHadron(p); }
template<class T> inline bool isBottomHadron(const T& p) { return  leadingQuark(p) == BQUARK && isHadron(p); }
template<class T> inline bool isTopHadron(const T& p) { return  leadingQuark(p) == TQUARK && isHadron(p); }

template<class T> inline bool isLightMeson(const T& p) { auto lq = leadingQuark(p); return  (lq == DQUARK || lq == UQUARK||lq == SQUARK) && isMeson(p); }
template<class T> inline bool isHeavyMeson(const T& p) { auto lq = leadingQuark(p); return  (lq == CQUARK || lq == BQUARK || lq == TQUARK) && isMeson(p); }
template<class T> inline bool isStrangeMeson(const T& p) { return  leadingQuark(p) == SQUARK && isMeson(p); }
template<class T> inline bool isCharmMeson(const T& p) { return  leadingQuark(p) == CQUARK && isMeson(p); }
template<class T> inline bool isBottomMeson(const T& p) { return  leadingQuark(p) == BQUARK && isMeson(p); }
template<class T> inline bool isTopMeson(const T& p) { return  leadingQuark(p) == TQUARK && isMeson(p); }

bool isCCbarMeson(const DecodedPID& p);
inline bool isCCbarMeson(const int& p) { return isCCbarMeson(DecodedPID(p)); }
template<class T> inline bool isCCbarMeson(const T& p) { return isCCbarMeson(p->pdg_id());}
bool isBBbarMeson(const DecodedPID& p);
inline bool isBBbarMeson(const int& p) { return isBBbarMeson(DecodedPID(p)); }
template<class T> inline bool isBBbarMeson(const T& p){ return isBBbarMeson(p->pdg_id());}


template<class T> inline bool isLightBaryon(const T& p) { auto lq = leadingQuark(p); return  (lq == DQUARK || lq == UQUARK||lq == SQUARK) && isBaryon(p); }
template<class T> inline bool isHeavyBaryon(const T& p) {  auto lq = leadingQuark(p); return  (lq == CQUARK || lq == BQUARK || lq == TQUARK) && isBaryon(p); }
template<class T> inline bool isStrangeBaryon(const T& p) { return  leadingQuark(p) == SQUARK && isBaryon(p); }
template<class T> inline bool isCharmBaryon(const T& p) { return  leadingQuark(p) == CQUARK && isBaryon(p); }
template<class T> inline bool isBottomBaryon(const T& p) { return  leadingQuark(p) == BQUARK && isBaryon(p); }
template<class T> inline bool isTopBaryon(const T& p) { return  leadingQuark(p) == TQUARK && isBaryon(p); }


// APID: This function selects B-Hadrons which predominantly decay weakly. (Commonly used definition in GeneratorFilters package.)
// 5[1-4]1 L = J = 0, S = 0
// 5[1-5][1-4]2 J = 1/2, n_r = 0, n_L =0
bool isWeaklyDecayingBHadron(const int& p);
inline bool isWeaklyDecayingBHadron(const DecodedPID& p){ return isWeaklyDecayingBHadron(p.pid()); }
template<class T> inline bool isWeaklyDecayingBHadron(const T& p) {return isWeaklyDecayingBHadron(p->pdg_id());}


// APID: This function selects C-Hadrons which predominantly decay weakly. (Commonly used definition in GeneratorFilters package.)
// 4[1-3]1 L = J = 0, S = 0
// 4[1-4][1-3]2 J = 1/2, n_r = 0, n_L =0
// NB Omitting pid = 4322 (Xi'_C+) a this undergoes an EM rather than
// weak decay.  (There was an old version of Herwig that decayed it
// weakly, but this was fixed in Herwig 7.)
bool isWeaklyDecayingCHadron(const int& p);
inline bool isWeaklyDecayingCHadron(const DecodedPID& p){ return isWeaklyDecayingCHadron(p.pid()); }
template<class T> inline bool isWeaklyDecayingCHadron(const T& p) {return isWeaklyDecayingCHadron(p->pdg_id());}


int charge3(const DecodedPID& p);
int charge3(const int& p);
double fractionalCharge(const DecodedPID& p);
inline double fractionalCharge(const int& p){return fractionalCharge(DecodedPID(p));}
template<class T> inline int charge3( const T& p){return charge3(p->pdg_id());}
template<class T> inline double fractionalCharge(const T& p){return fractionalCharge(p->pdg_id());}
template<class T> inline double charge( const T& p){
  if (isGenericMultichargedParticle(p)) // BSM multi-charged particles might have a fractional charge that's not a multiple of 1/3
    return fractionalCharge(p);
  else
    return 1.0*charge3(p)/3.0;
}
template<class T> inline double threeCharge( const T& p){ return charge3(p);}
template<class T> inline bool isCharged( const T& p){ return charge3(p) != 0;}


inline bool isNeutral(const DecodedPID& p){ return p.pid() != 0 && charge3(p) == 0;}
inline bool isNeutral(const int& p){ return isNeutral(DecodedPID(p));}
template<class T> inline bool isNeutral( const T& p){ return p->pdg_id() != 0 && charge3(p) == 0;}


// APID: Including Z' and Z'' as EM interacting.
bool isEMInteracting(const int& p);
template<class T> inline bool isEMInteracting(const T& p){return isEMInteracting(p->pdg_id());}

template<class T> inline bool isParton(const T& p) { return isQuark(p)||isGluon(p);}

// APID: Intended to return 2J
// Useful for G4ParticleDefinition constructor
int spin2(const DecodedPID& p);
inline int spin2(const int& p){ return spin2(DecodedPID(p));}
template<class T> inline int spin2(const T& p) { return spin2(p->pdg_id()); }

inline double spin(const DecodedPID& p) { return 1.0*spin2(p)/2.0; }
inline double spin(const int& p){ return spin(DecodedPID(p));}
template<class T> inline double spin(const T& p) { return spin(p->pdg_id()); }


// APID: Returns an unordered list of the quarks contained by the current particle
std::vector<int> containedQuarks(const int& p);
inline std::vector<int> containedQuarks(const DecodedPID& p) { return containedQuarks(p.pid()); }
template<class T> inline std::vector<int> containedQuarks(const T& p) { return containedQuarks(p->pdg_id()); }

bool isStrongInteracting(const int& p);
template<class T> inline bool isStrongInteracting(const T& p){return isStrongInteracting(p->pdg_id());}

#endif
