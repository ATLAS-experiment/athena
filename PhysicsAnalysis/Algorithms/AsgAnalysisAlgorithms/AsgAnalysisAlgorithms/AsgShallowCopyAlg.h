/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef ASG_ANALYSIS_ALGORITHMS__ASG_SHALLOW_COPY_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__ASG_SHALLOW_COPY_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <xAODBase/IParticleContainer.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysListHandle.h>


namespace CP
{
  /// \brief create a shallow copy of an input contsainer
  ///
  /// This is a generic algorithm that works for all object types to
  /// create a shallow copy (or one per systematic).  This avoids the
  /// need for every CP algorithm to be able to shallow copy its
  /// inputs, just in case it happens to be first in the sequence...

  class AsgShallowCopyAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    AsgShallowCopyAlg (const std::string& name,
                       ISvcLocator* pSvcLocator);


  public:
    StatusCode initialize () override;

  public:
    StatusCode execute () override;


    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the input collection we run on
  private:
    SysReadHandle<xAOD::IParticleContainer> m_inputHandle {
      this, "input", "", "the input collection to run on"};

    /// \brief the output view container we produce
  private:
    SysWriteHandle<xAOD::IParticleContainer> m_outputHandle {
      this, "output", "", "the output view container to produce"};

    /// \brief the templated version of execute for a single systematic
  private:
    template<typename Type> StatusCode
    executeTemplate (const CP::SystematicSet& sys);

    /// \brief the version of execute to find the type
  private:
    StatusCode executeFindType (const CP::SystematicSet& sys);

  private:
    StatusCode (AsgShallowCopyAlg::* m_function) (const CP::SystematicSet& sys) {&AsgShallowCopyAlg::executeFindType};
  };
}

#endif
