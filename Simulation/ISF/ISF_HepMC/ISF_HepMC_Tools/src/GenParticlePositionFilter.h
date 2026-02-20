/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_HEPMC_GENPARTICLEPOSITIONFILTER_H
#define ISF_HEPMC_GENPARTICLEPOSITIONFILTER_H 1

// STL includes
#include <string>
#include <vector>

// FrameWork includes
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

// ISF includes
#include "ISF_Interfaces/IGeoIDSvc.h"

namespace ISF {

  /** @class GenParticlePositionFilter

      Particle filter by position, to be used for initial GenEvent read-in.

      @author Andreas.Salzburger -at- cern.ch
  */
  class GenParticlePositionFilter : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    //** Constructor with parameters */
    GenParticlePositionFilter( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    ~GenParticlePositionFilter() = default;

    /** Athena algtool's Hooks */
    virtual StatusCode  initialize() override final;

    /** does the given particle pass the filter? */
#ifdef HEPMC3
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle) const override final;
#else
    virtual bool pass(const HepMC::GenParticle& particle) const override final;
#endif

  private:
    ServiceHandle<IGeoIDSvc> m_geoIDSvc{this, "GeoIDService", "ISF_GeoIDSvc", "The GeoID Service"};
    Gaudi::Property<std::vector<int>> m_checkRegion{this, "CheckRegion", {}, "Check if the given particles are within the specified regions"};
  };

}


#endif //> !ISF_HEPMC_GENPARTICLEPOSITIONFILTER_H
