/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_ALGORITHMS_DROPQSPARTICLEDAUGHTERSTOOL_H
#define ISF_ALGORITHMS_DROPQSPARTICLEDAUGHTERSTOOL_H 1

// STL includes
#include <string>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

// ISF_Interfaces includes
#include "ISF_HepMC_Interfaces/IGenParticleFilter.h"
#include "ISF_Interfaces/IGenEventFilter.h"

namespace ISF {

  class TruthPreselectionTool : public extends<AthAlgTool, IGenEventFilter> {

  public:
    /** Constructor with parameters */
    TruthPreselectionTool( const std::string& t, const std::string& n, const IInterface* p );

    /** Destructor */
    virtual ~TruthPreselectionTool() = default;

    /** Athena algorithm's interface method initialize() */
    virtual StatusCode  initialize() override final;
    /** IGenEventFilter interface */
    virtual std::unique_ptr<HepMC::GenEvent> filterGenEvent(const HepMC::GenEvent& inputEvent) const override final;

  private:
    bool passesFilters(HepMC::ConstGenParticlePtr& part, const ToolHandleArray<IGenParticleFilter>& filters) const;
    bool identifiedQuasiStableParticleForSim(HepMC::ConstGenParticlePtr& part) const;
    bool hasQuasiStableAncestorParticle(HepMC::ConstGenParticlePtr& part) const;
    bool isPostQuasiStableParticleVertex(HepMC::ConstGenVertexPtr& vtx) const;
    /** Filter passes if a difference between the decision of m_genParticleOldFilters and m_genParticleNewFilters is found.
        m_genParticleCommonFilters is applied before to select relevant particles.
        If only m_genParticleCommonFilters is specified, filter passes if any particle passes this one
    **/
    ToolHandleArray<IGenParticleFilter>  m_genParticleFilters{this, "GenParticleFilters", {}, "Tools for filtering out GenParticles"};    //!< HepMC::GenParticle filters for both selections
    ToolHandle<IGenParticleFilter>  m_quasiStableFilter{this, "QuasiStableParticleFilter", "", "Tools for finding quasi-stable particles"};
  };
}

#endif //> !ISF_ALGORITHMS_DROPQSPARTICLEDAUGHTERSTOOL_H
