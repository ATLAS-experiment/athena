/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLELIFETIMEFILTER_H
#define ISF_HEPMC_GENPARTICLELIFETIMEFILTER_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"

// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

namespace ISF {

  /** @class GenParticleLifetimeFilter

      Filter out particles with lifetimes shorter than a minimum value
  */
  class GenParticleLifetimeFilter : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticleLifetimeFilter( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticleLifetimeFilter() = default;

    /** does the given particle pass the filter? */
#ifdef HEPMC3
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle) const override final;
#else
    virtual bool pass(const HepMC::GenParticle& particle) const override final;
#endif

  private:
    Gaudi::Property<double> m_minLifetime{this, "MinimumLifetime", 0.000001}; //units of c*ns
  };

}


#endif //> !ISF_HEPMC_GENPARTICLELIFETIMEFILTER_H
