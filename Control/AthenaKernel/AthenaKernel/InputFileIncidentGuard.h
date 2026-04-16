/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthenaKernel/InputFileIncidentGuard.h
 * @brief RAII guard that guarantees a matching end-incident for every
 *        begin-incident.
 */

#ifndef ATHENAKERNEL_INPUTFILEINCIDENTGUARD_H
#define ATHENAKERNEL_INPUTFILEINCIDENTGUARD_H

/**
 * @class InputFileIncidentGuard
 * @brief RAII guard that guarantees a matching end-incident for every
 *        begin-incident.
 *
 * @section Purpose
 *
 * Event selectors fire BeginInputFile / EndInputFile incidents to notify
 * listeners (e.g. MetaDataSvc) of input file transitions.  These incidents
 * must always be paired: every Begin must have a matching End.  When the
 * firing points are scattered across multiple methods, manual pairing is
 * error-prone -- early returns, exceptions, or forgotten calls can leave
 * incidents unmatched.
 *
 * This guard uses RAII to enforce pairing: the constructor fires the begin
 * incident, and the destructor fires the matching end incident.  Instances
 * can only be created through the static @c begin() factory, which is
 * marked @c [[nodiscard]] so the compiler warns if the guard is discarded.
 *
 * @section Semantics
 *
 * - <b>Move-only</b>: the moved-from instance is disarmed (will not fire
 *   the end incident).  Copy is deleted.
 * - <b>File transitions</b>: use @c transition() instead of direct
 *   assignment to guarantee End(old) fires before Begin(new).
 * - <b>Custom incident types</b>: defaults to BeginInputFile / EndInputFile,
 *   overridable for other paired incidents (e.g. BeginInputMemFile /
 *   EndInputMemFile).
 *
 * @section Usage
 *
 * Typical usage as an @c std::optional member of an event selector:
 * @code
 *   // Member:
 *   std::optional<InputFileIncidentGuard> m_guard;
 *
 *   // File transition (End for old file, then Begin for new):
 *   InputFileIncidentGuard::transition(m_guard, incSvc, name(), fileName, guid);
 *
 *   // First file (no previous guard to reset):
 *   m_guard = InputFileIncidentGuard::begin(incSvc, name(), fileName, guid);
 *
 *   // Shutdown -- fire End for the last file:
 *   m_guard.reset();
 *
 *   // Scoped guard for eventless files (Begin + End in same scope):
 *   {
 *      auto guard = InputFileIncidentGuard::begin(incSvc, name(),
 *                        fileName, {}, "eventless:" + fileName);
 *   }
 * @endcode
 */

#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/FileIncident.h"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

class InputFileIncidentGuard {
public:
   /**
    * @brief Factory: fire the begin incident and return a guard whose
    *        destructor fires the matching end incident.
    * @param incSvc        The incident service used to fire incidents.
    * @param source        Source name passed to the FileIncident (typically
    *                      the selector's @c name()).
    * @param beginFileName fileName for the begin incident.
    * @param guid          File GUID passed to the FileIncident.
    * @param endFileName   fileName for the end incident.  Defaults to
    *                      "FID:" + @p guid; pass explicitly for eventless files.
    * @param beginType     Incident type string for the begin incident.
    * @param endType       Incident type string for the end incident.
    *
    * This is the only way to create an instance.
    */
   [[nodiscard("End incident will not fire if the guard is discarded")]]
   static InputFileIncidentGuard begin(IIncidentSvc& incSvc,
                                       std::string_view source,
                                       std::string_view beginFileName,
                                       std::string_view guid,
                                       std::string_view endFileName = {},
                                       std::string_view beginType = IncidentType::BeginInputFile,
                                       std::string_view endType = IncidentType::EndInputFile) {
      return InputFileIncidentGuard(incSvc, source, beginFileName, guid,
                                    endFileName.empty() ? "FID:" + std::string(guid)
                                                        : std::string(endFileName),
                                    beginType, endType);
   }

   /**
    * @brief Replace the guard in an optional, with strict End-before-Begin
    *        ordering.
    * @param guard  The optional holding the current guard.  It is reset
    *               first (firing the end incident for the old file, if any),
    *               then a new guard is emplaced (firing the begin incident
    *               for the new file).
    *
    * The remaining parameters are forwarded to @c begin().
    * Use this for file transitions instead of direct assignment.
    */
   static void transition(std::optional<InputFileIncidentGuard>& guard,
                          IIncidentSvc& incSvc,
                          std::string_view source,
                          std::string_view beginFileName,
                          std::string_view guid,
                          std::string_view endFileName = {},
                          std::string_view beginType = IncidentType::BeginInputFile,
                          std::string_view endType = IncidentType::EndInputFile) {
      guard.reset();
      guard = begin(incSvc, source, beginFileName, guid,
                    endFileName, beginType, endType);
   }

   /// Destructor: fires the end incident unless the guard has been
   /// moved from (disarmed).
   ~InputFileIncidentGuard() {
      if (m_incSvc) {
         m_incSvc->fireIncident(
            FileIncident(m_source, m_endType,
                         m_endFileName, m_guid));
      }
   }

   /// Move constructor: takes ownership; the source is disarmed.
   InputFileIncidentGuard(InputFileIncidentGuard&& other) noexcept
      : m_incSvc(std::exchange(other.m_incSvc, nullptr))
      , m_source(std::move(other.m_source))
      , m_guid(std::move(other.m_guid))
      , m_endFileName(std::move(other.m_endFileName))
      , m_endType(std::move(other.m_endType))
   {}

   /// Move assignment: fires the end incident for the file being replaced
   /// (if any), then takes ownership of @p other.
   InputFileIncidentGuard& operator=(InputFileIncidentGuard&& other) noexcept {
      if (this != &other) {
         if (m_incSvc) {
            m_incSvc->fireIncident(
               FileIncident(m_source, m_endType,
                            m_endFileName, m_guid));
         }
         m_incSvc = std::exchange(other.m_incSvc, nullptr);
         m_source = std::move(other.m_source);
         m_guid = std::move(other.m_guid);
         m_endFileName = std::move(other.m_endFileName);
         m_endType = std::move(other.m_endType);
      }
      return *this;
   }

   /// Copy is not allowed.
   InputFileIncidentGuard(const InputFileIncidentGuard&) = delete;
   /// Copy assignment is not allowed.
   InputFileIncidentGuard& operator=(const InputFileIncidentGuard&) = delete;

private:
   /// Private constructor used by @c begin().
   InputFileIncidentGuard(IIncidentSvc& incSvc,
                          std::string_view source,
                          std::string_view beginFileName,
                          std::string_view guid,
                          std::string endFileName,
                          std::string_view beginType,
                          std::string_view endType)
      : m_incSvc(&incSvc)
      , m_source(source)
      , m_guid(guid)
      , m_endFileName(std::move(endFileName))
      , m_endType(endType)
   {
      m_incSvc->fireIncident(
         FileIncident(m_source, std::string(beginType),
                      std::string(beginFileName), m_guid));
   }

   /// Incident service pointer; nullptr if disarmed (moved-from).
   IIncidentSvc* m_incSvc;
   /// Source name for both begin and end incidents.
   std::string m_source;
   /// File GUID passed to the FileIncident.
   std::string m_guid;
   /// fileName used for the end incident.
   std::string m_endFileName;
   /// Incident type string for the end incident (e.g. "EndInputFile").
   std::string m_endType;
};

#endif // ATHENAKERNEL_INPUTFILEINCIDENTGUARD_H
