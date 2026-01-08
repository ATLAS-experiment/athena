/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Elmar Ritsch
 * @date October 2016
 * @brief A generic particle filter tool for HepMC::GenParticle types
 */

#ifndef ISF_HEPMC_GENPARTICLEGENERICFILTER_H
#define ISF_HEPMC_GENPARTICLEGENERICFILTER_H 1

// STL includes
#include <string>
#include <vector>
#include <limits>
#include <algorithm>

// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"
// ISF includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"

namespace ISF {

  // ISF forward declarations
  class ISFParticle;

  /// used to store a list of PDG particle codes
  typedef std::vector<int>      PDGCodes;


  /**
   * @class GenParticleGenericFilter
   * @headerfile GenParticleGenericFilter.h
   *
   * @brief Core Athena algorithm for the Integrated Simulation Framework
   *
   * This GenParticle filter provides a general way of selecting/filtering out particles
   * during GenEvent read-in.
   */
  class GenParticleGenericFilter : public extends<AthAlgTool, IGenParticleFilter> {

  public:
    /// Constructor with framework parameters
    GenParticleGenericFilter( const std::string& t, const std::string& n, const IInterface* p );

    /// Empty Destructor
    ~GenParticleGenericFilter() = default;

    /// Athena algtool's Hooks
    virtual StatusCode  initialize() override final;

    /// Interface method that returns whether the given particle passes all cuts or not
#ifdef HEPMC3
    virtual bool pass(const HepMC::ConstGenParticlePtr& particle) const override final;
#else
    virtual bool pass(const HepMC::GenParticle& particle) const override final;
#endif

  private:
    /// Check whether the given particle passes all configure cuts or not
#ifdef HEPMC3
    bool check_cuts_passed(const HepMC::ConstGenParticlePtr& particle) const;
#else
    bool check_cuts_passed(const HepMC::GenParticle& particle) const;
#endif

    /// the cuts defined by the use
    Gaudi::Property<double> m_minEta{this, "MinEta", std::numeric_limits<double>::lowest(), "Minimum Particle Pseudorapidity"};     //!< min pseudorapidity cut
    Gaudi::Property<double> m_maxEta{this, "MaxEta", std::numeric_limits<double>::max(), "Maximum Particle Pseudorapidity"};     //!< max pseudorapidity cut
    Gaudi::Property<double> m_minPhi{this, "MinPhi", -M_PI, "Minimum Particle Phi"};     //!< min phi cut
    Gaudi::Property<double> m_maxPhi{this, "MaxPhi", M_PI, "Maximum Particle Phi"};     //!< max phi cut
    Gaudi::Property<double> m_minMom{this, "MinMom", std::numeric_limits<double>::lowest(), "Minimum Particle Momentum"};     //!< min momentum cut
    Gaudi::Property<double> m_maxMom{this, "MaxMom", std::numeric_limits<double>::max(), "Maximum Particle Momentum"};     //!< max momentum cut
    Gaudi::Property<PDGCodes> m_pdgs{this, "ParticlePDG", {}, "List of accepted particle PDG IDs (any accepted if empty)"};

    /// Geometrical region (=cylindrical volume around z-axis) within which this filter is applicable
    Gaudi::Property<double> m_maxApplicableRadius{this, "MaxApplicableRadius", std::numeric_limits<decltype(m_maxApplicableRadius)>::max(), "Only particles with ProductionVertexRadius<MaxApplicableRadius may get filtered out"};
  };

} // ISF namespace


#endif //> !ISF_HEPMC_GENPARTICLEGENERICFILTER_H
