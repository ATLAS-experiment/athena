/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLEFINALSTATEFILER_H
#define ISF_HEPMC_GENPARTICLEFINALSTATEFILER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

namespace ISF {

  class ISFParticle;

  /** @class GenParticleFinalStateFilter

      Stable/Interacting particle filter for HepMC particles to be used in the
      stack filling process.

      @author Andreas.Salzburger -at- cern.ch
  */
  class GenParticleFinalStateFilter : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticleFinalStateFilter( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticleFinalStateFilter() = default;

    /** Athena algtool's Hooks */
    virtual StatusCode  initialize() override final;

#ifdef HEPMC3
    /** Returns the Particle Stack, should register truth */
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle) const override final;
#else
    /** Returns the Particle Stack, should register truth */
    virtual bool pass(const HepMC::GenParticle& particle) const override final;
#endif

    Gaudi::Property<double> m_checkGenSimStable{this, "CheckGenSimStable", true};    //!< boolean switch to check on sim stable
    Gaudi::Property<double> m_checkGenInteracting{this, "CheckGenInteracting", true};  //!< boolean switch to check on gen interacting

  };

}


#endif //> !ISF_HEPMC_GENPARTICLEFINALSTATEFILER_H
