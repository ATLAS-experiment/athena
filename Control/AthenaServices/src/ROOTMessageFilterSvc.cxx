//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "ROOTMessageFilterSvc.h"

// Gaudi include(s).
#include "Gaudi/Parsers/CommonParsers.h"
#include "Gaudi/Parsers/Factory.h"

// ROOT include(s).
#include <TError.h>

// System include(s).
#include <regex>
#include <stdexcept>

namespace Gaudi::Parsers {

StatusCode parse(std::set<std::tuple<std::string, std::string, int>>& rules,
                 std::string_view input) {
  // Do the heavy lifting using Gaudi's templated helper function.
  return Gaudi::Parsers::parse_(rules, input);
}

}  // namespace Gaudi::Parsers

namespace {

/// Structure describing a message handling rule
struct Rule {
  std::regex locationPattern;  ///< Location pattern to match
  std::regex messagePattern;   ///< Message pattern to match
  int severity;                ///< Severity level to apply the rule for
  std::size_t hash;            ///< Hash value for the rule, for comparisons
};

/// Comparison operator for @c Rule
///
/// Needed to be able to use @c Rule in an @c std::set
///
bool operator<(const Rule& lhs, const Rule& rhs) {
  return (lhs.hash < rhs.hash);
}

/// Singular / global data for filtering ROOT messages
struct StaticData {

  /// The "original" error handler function
  ErrorHandlerFunc_t m_errorHandler{nullptr};

  /// Parsed suppression rules for handling ROOT messages
  std::set<Rule> m_suppressionRules;

};  // struct AthenaErrorHandler

/// Global data for the ROOT error handler
///
/// This variable is only written to during the initialization and
/// finalization of the job. During event processing we only read from it.
/// That **should** make it thread safe...
///
static StaticData s_data ATLAS_THREAD_SAFE;

/// Function taking care of handling ROOT messages in Athena jobs
///
/// @param level    The severity level of the message
/// @param abort    Whether the message is an error that should abort the job
/// @param location The source of the message
/// @param message  The message itself
///
void errorHandler(Int_t level, Bool_t abort, const char* location,
                  const char* message) {

  // Explicitly only access the global data in a const way. To not mistakenly
  // modify it.
  const StaticData& constData = s_data;

  /// If there is no global data vailable, something has gone very wrong.
  if (!constData.m_errorHandler) {
    throw std::runtime_error("ROOTMessageFilterSvc is incorrectly initialized");
  }

  // Check if any of the suppression rules apply.
  for (const Rule& rule : constData.m_suppressionRules) {
    if (std::regex_match(location, rule.locationPattern) &&
        std::regex_match(message, rule.messagePattern) &&
        (rule.severity == level)) {
      // If a rule matches, then suppress the message.
      return;
    }
  }

  // Print the message using the pre-existing error handler.
  (*(constData.m_errorHandler))(level, abort, location, message);
}

}  // namespace

namespace Athena {

StatusCode ROOTMessageFilterSvc::initialize ATLAS_NOT_THREAD_SAFE() {

  // Print the received configuration.
  ATH_MSG_DEBUG("Using: " << m_suppressionRules);

  // Extend the global list of suppression rules with the ones configured on
  // this service.
  for (const auto& rule : m_suppressionRules.value()) {
    s_data.m_suppressionRules.insert({
        std::regex{std::get<0>(rule)},  // locationPattern
        std::regex{std::get<1>(rule)},  // messagePattern
        std::get<2>(rule),              // severity
        std::hash<std::string>{}(std::get<0>(rule) + std::get<1>(rule) +
                                 std::to_string(std::get<2>(rule)))  // hash
    });
  }

  // Set up our own error handler, while remembering the original one. But only
  // if we have not set it up already.
  if (!s_data.m_errorHandler) {
    s_data.m_errorHandler = ::SetErrorHandler(::errorHandler);
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

StatusCode ROOTMessageFilterSvc::finalize ATLAS_NOT_THREAD_SAFE() {

  // Restore the original ROOT error handler. If it has not been restored yet.
  if (s_data.m_errorHandler) {
    ::SetErrorHandler(s_data.m_errorHandler);
    s_data.m_errorHandler = nullptr;
  }

  // Return gracefully.
  return StatusCode::SUCCESS;
}

}  // namespace Athena
