/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak


#ifndef ASG_ANALYSIS_ALGORITHMS__SYS_LIST_DUMPER_ALG_H
#define ASG_ANALYSIS_ALGORITHMS__SYS_LIST_DUMPER_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgServices/ServiceHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <AsgTools/PropertyWrapper.h>

namespace CP
{
  /// \brief Dump systematics names into a histogram

  class SysListDumperAlg final : public EL::AnaAlgorithm
  {
    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual ::StatusCode initialize () override;
    virtual ::StatusCode execute () override;
    virtual ::StatusCode finalize () override;

    /// \brief make the systematics vector using a regex
  private:
    std::vector<CP::SystematicSet> makeSystematicsVector (const std::string &regex) const;

    /// \brief the handle for the systematics service
  private:
    ServiceHandle<ISystematicsSvc> m_systematicsService {this, "systematicsService", "SystematicsSvc", "systematics service"};

    /// \brief the regex
  private:
    Gaudi::Property<std::string> m_regex {this, "systematicsRegex", "", "systematics regex"};

    /// \brief the name of the histogram to use
  private:
    Gaudi::Property<std::string> m_histogramName {this, "histogramName", "systematics", "the name of the output histogram"};

    /// \brief whether the next event will be the first event
  private:
    bool m_firstEvent = true;
  };
}

#endif
