// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/Event.h"
#include "xAODRootAccess/TVirtualIncidentListener.h"
#include "xAODRootAccess/tools/TVirtualManager.h"

// Project include(s).
#include "xAODCore/tools/IOStats.h"
#include "xAODCore/tools/PerfStats.h"
#include "xAODRootAccessInterfaces/TActiveEvent.h"
#ifndef XAOD_STANDALONE
#include "SGTools/CurrentEventStore.h"
#endif  // not XAOD_STANDALONE

// ROOT include(s).
#include <TClass.h>

// System include(s).
#include <iomanip>
#include <sstream>

namespace xAOD {

/// @param name Name to use in printed messages
///
Event::Event(std::string_view name)
    : TVirtualEvent(),
      Details::IProxyDictBase(),
      asg::AsgMessaging(std::string{name}) {

  // Make sure that the I/O monitoring is active.
  PerfStats::instance();

  // Make this the active event.
  setActive();
}

Event::~Event() {

  // If this is set up as the active event at the moment, notify
  // the active event object that this object will no longer be
  // available.
  if (TActiveEvent::event() == this) {
    TActiveEvent::setEvent(nullptr);
  }
#ifndef XAOD_STANDALONE
  if (SG::CurrentEventStore::store() == this) {
    SG::CurrentEventStore::setStore(nullptr);
  }
#endif  // not XAOD_STANDALONE
}

void Event::setActive() const {

  // The active event and current store are thread-local globals.
  Event* nc_this ATLAS_THREAD_SAFE = const_cast<Event*>(this);
  TActiveEvent::setEvent(static_cast<TVirtualEvent*>(nc_this));
#ifndef XAOD_STANDALONE
  SG::CurrentEventStore::setStore(nc_this);
#endif  // not XAOD_STANDALONE
}

/// This function receives the rules for selecting which dynamic auxiliary
/// branches should be written for a given container, in the exact same
/// format in which we need to set it in the Athena output ItemList.
///
/// @param containerKey The name of the auxiliary container in question
/// @param itemList The variable list according to the formatting rules
///
void Event::setAuxItemList(const std::string& containerKey,
                           const std::string& itemList) {

  // Decoded attributes.
  std::set<std::string> attributes;

  // Split up the received string using "." as the separator.
  if (itemList.size()) {
    std::istringstream ss(itemList);
    std::string attr;
    while (std::getline(ss, attr, '.')) {
      attributes.insert(attr);
    }
  }

  // Remember the setting.
  m_auxItemList[containerKey] = std::move(attributes);
}

/// This function works pretty much like IIncidentSvc::addListener does in
/// Athena. It tells the TEvent object that when certain "interesting
/// incidents" happen, a given object should be notified about it.
///
/// @param listener Pointer to the object that should be notified
/// @returns The usual @c StatusCode types
///
StatusCode Event::addListener(TVirtualIncidentListener* listener) {

  // Check that we received a valid pointer:
  if (listener == nullptr) {
    ATH_MSG_ERROR("Received a null pointer for the listener");
    return StatusCode::FAILURE;
  }

  // Add the listener.
  if (m_listeners.insert(listener).second == false) {
    ATH_MSG_WARNING("Listener " << static_cast<void*>(listener)
                                << " was added previously already");
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function allows us to remove a listener when for instance a
/// metadata tool is deleted during a job.
///
/// @param listener Pointer to the listener that should be removed
/// @returns The usual @c StatusCode types
///
StatusCode Event::removeListener(TVirtualIncidentListener* listener) {

  // Remove the listener. Or at least try to...
  if (m_listeners.erase(listener) != 1u) {
    ATH_MSG_ERROR("Listener " << static_cast<void*>(listener) << " not known");
    return StatusCode::FAILURE;
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function can be used to remove all the listeners from the internal
/// list. Should not be necessary under regular circumstances.
///
void Event::clearListeners() {

  m_listeners.clear();
}

/// The names of containers can change during the lifetime of the experiment.
/// One such change happened after the DC14 exercise, when many containers
/// got a new name. (Like "ElectronCollection" became simply "Electrons".)
///
/// This function allows us to create aliases with which certain containers
/// should be accessible. So that the analyser would be able to access
/// older files, while using the latest container name(s).
///
/// @param onfile The name of the container as it was saved into the input
///               file
/// @param newName The alias with which the object/container should be
///                accessible
/// @returns The usual @c StatusCode types
///
StatusCode Event::addNameRemap(const std::string& onfile,
                               const std::string& newName) {

  // Check if this name is known on the input or output already. As that's
  // not good.
  if (m_inputEventFormat.exists(newName)) {
    ATH_MSG_ERROR("Can't use \"" << newName << "\" as the target name in the \""
                                 << onfile << "\" -> \"" << newName
                                 << "\" remapping");
    return StatusCode::FAILURE;
  }

  // Check if this name was remapped to something already.
  auto itr = m_nameRemapping.find(newName);
  if (itr != m_nameRemapping.end()) {
    ATH_MSG_WARNING("Overriding existing name remapping \""
                    << itr->second << "\" -> \"" << itr->first << "\" with: \""
                    << onfile << "\" -> \"" << newName << "\"");
  }

  /// Save the new name association:
  m_nameRemapping[newName] = onfile;

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function simply clears out any existing name remapping declarations.
/// In case the remapping rules need to be changed in the code in some
/// complicated way.
///
void Event::clearNameRemap() {

  m_nameRemapping.clear();
}

/// This function can be used for debugging, to check what container/object
/// name remapping rules are in place for the current TEvent object.
///
void Event::printNameRemap() const {

  // Print a header.
  ATH_MSG_INFO("Name remapping rules:");

  // In case no remapping rules have been set.
  if (m_nameRemapping.empty()) {
    ATH_MSG_INFO("   NONE");
    return;
  }

  // Otherwise.
  for (const auto& [newName, onfile] : m_nameRemapping) {
    ATH_MSG_INFO("   \"" << newName << "\" -> \"" << onfile << "\"");
  }
}

/// These appear harmless so long as you don't actually try to access the
/// links. Which will cause other errors anyway.
///
/// @param value The new value for the option
///
void Event::printProxyWarnings(bool value) {

  m_printEventProxyWarnings = value;
}

const EventFormat* Event::inputEventFormat() const {

  if (hasInput()) {
    return &m_inputEventFormat;
  }
  return nullptr;
}

const EventFormat* Event::outputEventFormat() const {

  return m_outputEventFormat;
}

/// This function behaves exactly like @c StoreGateSvc::dump(). It doesn't
/// actually print anything to the screen, it just returns a user
/// readable dump of the contents of the current input file/chain.
///
/// It is a pretty dumb implementation for the moment. Should be made
/// nicer later on.
///
/// @returns The user-readable contents of the current input file/chain
///
std::string Event::dump() {

  // The internal stream object.
  std::ostringstream ost;
  ost << "<<<<<<<<<<<<<<<<<<<< xAOD::TEvent Dump >>>>>>>>>>>>>>>>>>>>\n";

  // Loop over the input EventFormat object.
  for (const auto& [key, element] : m_inputEventFormat) {

    // Get the type.
    ::TClass* cl = ::TClass::GetClass(element.className().c_str());
    const std::type_info* ti = (cl ? cl->GetTypeInfo() : nullptr);
    if ((cl == nullptr) || (cl->IsLoaded() == false) || (ti == nullptr)) {
      ATH_MSG_WARNING("Unknown type (" << element.className()
                                       << ") found in the event format");
      continue;
    }

    // Skip containers that are not available anyway.
    static const bool METADATA = false;
    if (!contains(element.branchName(), *ti, METADATA)) {
      continue;
    }

    // Do the printout.
    ost << " Hash: 0x" << std::setw(8) << std::setfill('0') << std::hex
        << element.hash() << " Key: \"" << element.branchName() << "\"\n";

    ost << "   type: " << element.className() << "\n";
    const bool isNonConst =
        transientContains(element.branchName(), *ti, METADATA);
    ost << "   isConst: " << (isNonConst ? "No" : "Yes") << "\n";
    static const bool SILENT = false;
    ost << "   Data: "
        << (isNonConst
                ? getOutputObject(element.branchName(), *ti, METADATA)
                : getInputObject(element.branchName(), *ti, SILENT, METADATA))
        << "\n";
  }

  // Finish with the construction:
  ost << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
  return ost.str();
}

/// This is a convenience function for printing basic I/O information about
/// the current job. It can be called at the end of a job to get an overview
/// of what the job did exactly I/O-wise.
void Event::printIOStats() const {

  // Simply do this via the xAODCore code:
  IOStats::instance().stats().Print("Summary");
}

}  // namespace xAOD
