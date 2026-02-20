/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/src/RCUObject.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2016
 * @brief read-copy-update (RCU) style synchronization for Athena.
 */


#include "AthenaKernel/RCUObject.h"
#include "AthenaKernel/IRCUSvc.h"
#include "boost/dynamic_bitset.hpp"
#include <cstdlib>


namespace {


/**
 * @brief Declare that the grace period for a slot is ending.
 * @param Lock object (external locking).
 * @param ctx Event context for the slot.
 * @param grace Bitmask tracking grace periods.
 * @returns true if any slot is still in a grace period.
 *          false if no slots are in a grace period.
 *
 * The caller must be holding the mutex for @c grace.
 */
inline
bool endGrace (const EventContext& ctx,
               boost::dynamic_bitset<>& grace)
{
  EventContext::ContextID_t slot = ctx.slot();
  if (slot == EventContext::INVALID_CONTEXT_ID) return false;
  if (slot >= grace.size()) std::abort();
  grace[slot] = false;
  return grace.any();
}


} // anonymous namespace


namespace Athena {


struct RCUObjectGraceSets
{
  RCUObjectGraceSets (size_t nslots)
    : m_grace (nslots),
      m_oldGrace (nslots)
  {
  }


  /// Bit[i] set means that slot i is in a grace period.
  boost::dynamic_bitset<> m_grace;

   /// Same thing, for the objects marked as old.
  boost::dynamic_bitset<> m_oldGrace;
};


/**
 * @brief Constructor, with RCUSvc.
 * @param svc Service with which to register.
 *
 * The service will call @c quiescent at the end of each event.
 */
IRCUObject::IRCUObject (IRCUSvc& svc)
  : m_svc (&svc),
    m_graceSets (std::make_unique<RCUObjectGraceSets> (svc.getNumSlots())),
    m_nold(0),
    m_dirty(false)
{
  m_svc->add (this);
}


/**
 * @brief Constructor, with event slot count.
 * @param nslots Number of active event slots.
 *
 * This version does not register with a service.
 */
IRCUObject::IRCUObject (size_t nslots)
  : m_svc (nullptr),
    m_graceSets (std::make_unique<RCUObjectGraceSets> (nslots)),
    m_nold(0),
    m_dirty(false)
{
}


/**
 * @brief Destructor.
 *
 * Remove this object from the service if it has been registered.
 */
IRCUObject::~IRCUObject()
{
  if (m_svc && m_svc->remove (this).isFailure())
    std::abort();
}


/**
 * @brief Move constructor.
 *
 * Allow passing these objects via move.
 */
IRCUObject::IRCUObject (IRCUObject&& other)
  : m_svc (other.m_svc),
    m_graceSets (std::move (other.m_graceSets)),
    m_nold (other.m_nold),
    m_dirty (false)
{
  other.m_nold = 0;
  if (other.m_dirty) {
    m_dirty = true;
  }
  other.m_dirty = false;
  if (m_svc) {
    if (m_svc->remove (&other).isFailure()) {
      std::abort();
    }
    other.m_svc = nullptr;
    m_svc->add (this);
  }
}


/**
 * @brief Out-of-line part of quiescent().
 */
void IRCUObject::quiescentOol (const EventContext& ctx)
{
  // We get here after the dirty flag has already been checked.
  lock_t g (m_mutex);
  if (!::endGrace(ctx, m_graceSets->m_grace)) {
    clearAll(g);
    m_nold = 0;
    m_dirty = false;
  }
  else if (m_nold > 0 && !::endGrace(ctx, m_graceSets->m_oldGrace)) {
    if (clearOld(g, m_nold)) {
      m_dirty = false;
    }
    m_nold = 0;
  }
}


/**
 * @brief Declare that the grace period for a slot is ending.
 * @param lock Lock object (external locking).
 * @param ctx Event context for the slot.
 * @returns true if any slot is still in a grace period.
 *          false if no slots are in a grace period.
 *
 * The caller must be holding the mutex for this object.
 */
bool IRCUObject::endGrace (lock_t& /*lock*/, const EventContext& ctx)
{
  return ::endGrace (ctx, m_graceSets->m_grace);
}


/**
 * @brief Declare that all slots are in a grace period.
 * @param Lock object (external locking).
 *
 * The caller must be holding the mutex for this object.
 */
void IRCUObject::setGrace (lock_t& /*lock*/)
{
  m_graceSets->m_grace.set();
  if (!m_dirty) m_dirty = true;
}


/**
 * @brief Make existing pending objects old, if possible.
 * @param lock Lock object (external locking).
 * @param garbageSize Present number of objects pending deletion.
 *
 * A new object is about to be added to the list of objects pending deletion.
 * If there are any existing pending objects and there are no existing
 * old objects, make the current pending objects old.
 */
void IRCUObject::makeOld (lock_t& /*lock*/, size_t garbageSize)
{
  if (garbageSize && m_nold == 0) {
    m_graceSets->m_oldGrace = m_graceSets->m_grace;
    m_nold = garbageSize;
  }
}


} // namespace Athena
