/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SYSTEMATICS_SVC_H
#define SYSTEMATICS_HANDLES__SYSTEMATICS_SVC_H

#include <AsgServices/AsgService.h>
#include <CxxUtils/checker_macros.h>
#include <PATInterfaces/SystematicSet.h>
#include <SystematicsHandles/ISystematicsSvc.h>
#include "AsgTools/PropertyWrapper.h"
#include <mutex>
#include <unordered_map>

namespace CP
{
  /// \brief the canonical implementation of \ref ISystematicsSvc

  class SystematicsSvc final : public asg::AsgService,
                                 virtual public ISystematicsSvc
  {
    //
    // public interface
    //

    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    SystematicsSvc (const std::string& name,
                      ISvcLocator* pSvcLocator);



    //
    // inherited interface
    //

  public:
    virtual StatusCode initialize () override;
    virtual StatusCode finalize () override;
    virtual std::vector<CP::SystematicSet>
    makeSystematicsVector () const override;
    virtual StatusCode addSystematics (const CP::SystematicSet& recommended,
                                       const CP::SystematicSet& affecting) const override;
    virtual CP::SystematicSet
    getObjectSystematics (const std::string& name) const override;
    virtual StatusCode
    setObjectSystematics (const std::string& name,
                          const CP::SystematicSet& systematics) const override;
    virtual CP::SystematicSet
    getDecorSystematics (const std::string& objectName,
                         const std::string& decorName) const override;
    virtual StatusCode
    setDecorSystematics (const std::string& objectName,
                         const std::string& decorName,
                         const CP::SystematicSet& systematics) const override;
    virtual StatusCode
    registerCopy (const std::string& fromName,
                  const std::string& toName) const override;
    virtual std::string
    getCopySource (const std::string& toName) const override;
    virtual StatusCode
    makeSystematicsName (std::string& result,
                         const std::string& name,
                         const CP::SystematicSet& sys) const override;



    //
    // private interface
    //

    /// \brief the names of the systematics to request
  private:
    Gaudi::Property<std::vector<std::string>> m_systematicsList {this, "systematicsList", {}, "the list of systematics to run"};

    /// \brief the regular expression for filterinf systematics
  private:
    Gaudi::Property<std::string> m_systematicsRegex {this, "systematicsRegex", "(.*)", "systematics filter regex"};

    /// \brief load all recommended systematics at the given number of
    /// sigmas
    ///
    /// The idea here is that this allows to run a simple analysis by
    /// itself without having to generate the list of systematics
    /// manually.
  private:
    Gaudi::Property<float> m_sigmaRecommended {this, "sigmaRecommended", 0, "the sigma with which to run recommended systematics"};

    /// \brief nominal systematics name
  private:
    Gaudi::Property<std::string> m_nominalSystematicsName {this, "nominalSystematicsName", "NOSYS", "the name to use for the nominal systematic (instead of the empty string)"};


    /// \brief the list of affecting systematics
  private:
    mutable SystematicSet m_affectingSystematics ATLAS_THREAD_SAFE;

    /// \brief the list of recommended systematics
  private:
    mutable SystematicSet m_recommendedSystematics ATLAS_THREAD_SAFE;

    /// \brief the list of per-object systematics
  private:
    mutable std::unordered_map<std::string,CP::SystematicSet> m_objectSystematics ATLAS_THREAD_SAFE;

    /// \brief the list of per-object-and-decoration systematics
  private:
    mutable std::unordered_map<std::string,CP::SystematicSet> m_decorSystematics ATLAS_THREAD_SAFE;

    /// \brief the map of registered copies
  private:
    mutable std::unordered_map<std::string,std::string> m_copies ATLAS_THREAD_SAFE;

    /// \brief a mutex for accessing the above mutable members
  private:
    mutable std::mutex m_systematicsMutex;
  };
}

#endif
