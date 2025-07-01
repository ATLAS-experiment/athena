// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef ATHENASERVICES_ROOTMESSAGEFILTERSVC_H
#define ATHENASERVICES_ROOTMESSAGEFILTERSVC_H

// Athena include(s).
#include "AthenaBaseComps/AthService.h"

// Gaudi include(s).
#include "Gaudi/Property.h"

// System include(s).
#include <set>
#include <string>
#include <string_view>
#include <tuple>

namespace Gaudi::Parsers {

/// Parse the specific tuple property used by @c Athena::ROOTMessageFilterSvc
StatusCode parse(std::set<std::tuple<std::string, std::string, int>>& rules,
                 std::string_view input);

}  // namespace Gaudi::Parsers

namespace Athena {

/// Service for filtering "ROOT messages" in Athena
///
/// Currently the service is just used to suppress certain messages coming
/// from ROOT, which are deemed not useful or too verbose.
///
class ROOTMessageFilterSvc : public AthService {

 public:
  // Inherit the base class's constructor(s)
  using AthService::AthService;

  /// @name Function(s) implemented from @c AthService
  /// @{

  /// Function initializing the service.
  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE() override;
  /// Function finalizing the service.
  virtual StatusCode finalize ATLAS_NOT_THREAD_SAFE() override;

  /// @}

 private:
  /// @name Service properties
  /// @{

  /// List of suppression rules for handling ROOT messages
  Gaudi::Property<std::set<std::tuple<std::string, std::string, int>>>
      m_suppressionRules{
          this,
          "SuppressionRules",
          {},
          "List of rules for suppressing ROOT messages. "
          "Format: [('location regex', 'message regex', output level), ...]"};

  /// @}

};  // class ROOTMessageFilterSvc

}  // namespace Athena

#endif  // NOT ATHENASERVICES_ROOTMESSAGEFILTERSVC_H
