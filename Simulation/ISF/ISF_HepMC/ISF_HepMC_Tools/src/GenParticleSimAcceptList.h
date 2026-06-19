/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLESIMACCEPTLIST_H
#define ISF_HEPMC_GENPARTICLESIMACCEPTLIST_H 1

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

// STL includes
#include <string>
#include <vector>

#include "AtlasHepMC/GenParticle.h"
namespace ISF {

  class ISFParticle;

  /** @class GenParticleSimAcceptList

      Stable/Interacting particle filter for HepMC particles to be used in the
      stack filling process.  Checks this particle and all daughters.

      @author ZLMarshall -at- lbl.gov
  */
  class GenParticleSimAcceptList : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticleSimAcceptList( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticleSimAcceptList(){}

    /** Athena algtool's Hooks */
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    /** passes through to the private version */
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle ) const override;

  private:
    /** returns true if the the particle and all daughters are on the accept list */
    bool pass(const HepMC::ConstGenParticlePtr& particle , std::vector<int> & used_vertices ) const;
    StringArrayProperty m_acceptLists{this, "AcceptLists", {"G4particle_acceptlist.txt"} }; //!< The location of the accept lists
    std::vector<long int>             m_pdgId;                //!< Allowed PDG IDs
    BooleanProperty m_qs{this, "QuasiStableSim", true}; //!< Switch for quasi-stable particle simulation
    BooleanProperty m_useShadowEvent{this, "UseShadowEvent", false, "New approach to selecting particles for simulation" };
    DoubleProperty m_minDecayRadiusQS{this, "MinimumDecayRadiusQS", 30.19*Gaudi::Units::mm}; //!< Decay radius below which QS particles should be ignored
  };

}


#endif //> !ISF_HEPMC_GENPARTICLESIMACCEPTLIST_H
