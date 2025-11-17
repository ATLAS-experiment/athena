/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef FIX_HEPMC
#define FIX_HEPMC

#include "GeneratorModules/GenBase.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/Loops.h"
#include <set>
#include <memory>


/// @brief A "fix-up" algorithm to correct weird event records
///
/// FixHepMC is run in evgen production, *after* the chain of input,
/// shower, hadronization, and afterburner generators, but before
/// the TestHepMC check algorithm, any analysis algs, and the POOL
/// event stream output. Its job is to "normalise" strange event
/// features to allow greater consistency of MC event record use in
/// ATLAS.
///
/// Examples of its features include removing dummy "padding" particles
/// from event records, stripping parton and EW boson intermediates
/// from hadron/tau decay chains, and ensuring that every event has at
/// least one entry in its event weight vector.
///
class FixHepMC : public GenBase {
public:

  FixHepMC(const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode execute();
  StatusCode finalize();


private:

  /// @name Classifiers for identifying particles to be removed
  //@{
  bool isPID0(const HepMC::ConstGenParticlePtr& p) const;
  bool isNonTransportableInDecayChain(const HepMC::ConstGenParticlePtr& p) const;
  bool isSimpleLoop(const HepMC::ConstGenParticlePtr& p) const;
  bool fromDecay(const HepMC::ConstGenParticlePtr& p, std::shared_ptr<std::set<int> >& storage) const;
  //@}

  /// @name Config properties
  //@{
  bool m_killLoops;   // Kill loops?
  bool m_killPDG0;    // Kill PDG0 particles?
  bool m_cleanDecays; // Clean decays?
  bool m_purgeUnstableWithoutEndVtx; // Remove unstable particles without decay vertex?
  bool m_ignoreSemiDisconnected; // Ignore semi-disconnected particles (normal in Sherpa)
  std::string m_forced_momentum{""}; // Force momentum unit for the event
  std::string m_forced_length{""}; // Force length unit for the event
  //@}

  /// @name Cleaned-particle counters
  //@{
  long m_loopKilled;
  long m_pdg0Killed;
  long m_decayCleaned;
  long m_unstablePurged;
  long m_totalSeen;
  long m_replacedPIDs;
  //@}
   
  std::map<int,int> m_pidmap; //!< map of pids to change.


  MC::Loops<HepMC::GenEvent,HepMC::ConstGenParticlePtr,HepMC::ConstGenVertexPtr> m_looper; //!< member to detect loops


};


#endif

#endif
