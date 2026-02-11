#ifndef IPPERFORMANCE_TRACKTRUTHHELPERS_H
#define IPPERFORMANCE_TRACKTRUTHHELPERS_H

//#include <EventLoop/Worker.h>
#ifndef __MAKECINT__
#include "xAODTracking/Vertex.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTracking/TrackParticleContainer.h"
#endif // not __MAKECINT__

#include "TH1D.h"



class TruthMatchProbabilityCut {
 protected:
  double m_truthmatchprobabilitycut;
 public:
 TruthMatchProbabilityCut(double truthmatchprobabilitycut = 0.5) :
  m_truthmatchprobabilitycut (truthmatchprobabilitycut) {};
  bool accept(const xAOD::TrackParticle* track, const xAOD::Vertex*) const {
    static const SG::Accessor<float> mAcc_truthMatchProbability("truthMatchProbability"); 
    if( !mAcc_truthMatchProbability(*track)) {
      Warning("TruthMatchProbabilityCut()", "Track Particle has no MatchProb! Is this data?" );
      return true;
    }
    const SG::Accessor<float> mAcc_truthProb("truthMatchProbability");
    const float truthProb = mAcc_truthProb(*track);
    return ( truthProb >= m_truthmatchprobabilitycut );
  }
};




class TrackTruthHelpers {

public:
  TrackTruthHelpers(double pt = 500., double eta = 2.5, double truthMatchProb = 0.5);
  //TrackTruthHelpers(double pt, double eta, double truthMatchProb, std::string name, EL::Worker* worker);
  //TrackTruthHelpers(std::string name, EL::Worker* worker);

  /** Check if the truth particle is a stable charged particle */
  bool isStableParticle( const xAOD::TruthParticle* );

  /** Check if the track particle originated from is a primary
      charged particle within detector acceptance and good truth
      match probability */
  bool isPrimary( const xAOD::TrackParticle* );

  /** Check if the truth particle is a primary charged particle
      within detector acceptance */
  bool isPrimaryParticle( const xAOD::TruthParticle* );

  /** Check if the track particle originated from is a secondary
      charged particle good truth match probability */
  bool isSecondary( const xAOD::TrackParticle* );

  /** Check if the truth particle is a secondary charged particle */
  bool isSecondaryParticle( const xAOD::TruthParticle* );

  /** Check if the track particle originated from is a strange baryon */
  bool isStrangeBaryon( const xAOD::TrackParticle* );

  /** Check if the truth particle is a strange baryon */
  bool isStrangeBaryonParticle( const xAOD::TruthParticle* );

  /** Check if the track is a combinatorial fake or has
      no truth associated to it */
  bool isFake( const xAOD::TrackParticle* );

  /** Check if the track particle originated from is a primary
      charged particle without detector acceptance */
  bool isOOPS( const xAOD::TrackParticle* );

  /** Get the truth particle from associated to a track particle.
      Return 0 if there is no truth particle associated */
  const xAOD::TruthParticle* truthParticle(const xAOD::TrackParticle* ) ;

  /** Check phase space of truth particle */
  bool passAcceptance(const xAOD::TruthParticle* );

  /** Print cutflow summary */
  void printSummary();

  /** Book cutflow histograms */
  //void BookHistograms(std::string name, EL::Worker *worker);

  /** Fill all cutflows **/
  void fillCutflow( const xAOD::TrackParticle* );


  double m_PtCut;
  double m_EtaCut;
  double m_truthmatchprobability;
  bool printCutflow;
  TH1D* h_primary;
  TH1D* h_secondary;
  TH1D* h_fake;
  TH1D* h_OOPS;
  TH1D* h_strangeBaryon;

  std::vector<std::string> m_PrimaryCuts = \
    {"All","Status","Charge","Barcode","PhaseSpace","!isStrangeBaryon","TruthMatchProb"};
  std::vector<std::string> m_SecondaryCuts = \
    {"All","Status","Charge","Barcode","TruthMatchProb"};
  std::vector<std::string> m_StrangeBaryonCuts = \
    {"All","Status","Charge","Barcode","isStrangeBaryon","TruthMatchProb",
     /*Only as cross check added in CutflowChallenge*/ "PhaseSpace"};
  std::vector<std::string> m_FakeCuts = \
    {"All","NoTruth","BadTruthMatchProb","Barcode=0","Neutral"};
  std::vector<std::string> m_OOPSCuts = \
    {"All","Status","Charge","Barcode","NotPhaseSpace","!isStrangeBaryon","TruthMatchProb"};
};

#endif // CTIDEEFFICIENCY_TRACKTRUTHHELPERS_H
