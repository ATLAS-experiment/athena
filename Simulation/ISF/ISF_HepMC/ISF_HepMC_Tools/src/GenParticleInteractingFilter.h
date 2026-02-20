/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLEINTERACTINGFILTER_H
#define ISF_HEPMC_GENPARTICLEINTERACTINGFILTER_H 1

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

// STL includes
#include <string>

#include "AtlasHepMC/GenParticle.h"
namespace ISF {

  class ISFParticle;

  /** @class GenParticleInteractingFilter

      Stable/Interacting particle filter for HepMC particles to be used in the
      stack filling process.  Checks this particle for interacting (not nu/G/LSP/whatever).

      @author ZLMarshall -at- lbl.gov
  */
  class GenParticleInteractingFilter final : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticleInteractingFilter( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticleInteractingFilter() = default;

    /** Framework methods */
    virtual StatusCode initialize() override final;
#ifdef HEPMC3
    /** passes through to the private version */
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle ) const override final;
#else

    /** passes through to the private version */
    virtual bool pass(const HepMC::GenParticle& particle ) const override final;
#endif

    /** Additional PDG codes to classify as interacting */
    Gaudi::Property<std::vector<int>> m_additionalInteractingParticleTypes{this, "AdditionalInteractingParticleTypes", {}};

    /** Additional PDG codes to classify as non-interacting */
    Gaudi::Property<std::vector<int>> m_additionalNonInteractingParticleTypes{this, "AdditionalNonInteractingParticleTypes", {}};
  };

}

#endif //> !ISF_HEPMC_GENPARTICLEINTERACTINGFILTER_H
