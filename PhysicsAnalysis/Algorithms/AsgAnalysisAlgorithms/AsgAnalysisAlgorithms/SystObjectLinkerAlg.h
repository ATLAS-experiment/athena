/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  SystObjectLinkerAlg:
  This alg applies bidirectional decorations to a set of object collections,
  allowing navigation from the nominal to all systematic variations, and
  from the variations back to the nominal.
*/

/// @author Teng Jian Khoo


// Always protect against multiple includes!
#ifndef CP_SYSTOBJECTLINKERALG
#define CP_SYSTOBJECTLINKERALG

#include <vector>
#include <utility>

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODBase/IParticleContainer.h>

#include <AthLinks/ElementLink.h>

class EventContext;

namespace CP
{

  class SystObjectLinkerAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief The standard constructor
public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;

    /// \brief Initialisation method, for setting up tools and other persistent
    /// configs
    StatusCode initialize() override;
    /// \brief Execute method, for actions to be taken in the event loop
    StatusCode execute(const EventContext& ctx) const override;
    /// We use default finalize() -- this is for cleanup, and we don't do any

private:

    // Members for configurable properties

    /// \brief Setup list of systematics on which to iterate
    CP::SysListHandle m_systematicsList {this};

    /// \brief Setup syst-aware input container handles
    CP::SysReadHandle<xAOD::IParticleContainer>
      m_inputHandle{ this, "input", "",   "Container to read" };

    /// \brief Setup sys-aware output decorations
    CP::SysWriteDecorHandle<ElementLink<xAOD::IParticleContainer> >
      m_syst_link_decor {"systVarLink_%SYS%", this};

  };
}

#endif
