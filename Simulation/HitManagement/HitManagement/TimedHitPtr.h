/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITMANAGEMENT_TIMEDHITPTR
#define HITMANAGEMENT_TIMEDHITPTR

/** @class TimedHitPtr
 *  @brief a smart pointer to a hit that also provides access to the
 *  extended timing info of the host event. For a pu event this is
 *  taken from the event PileUpTimeEventIndex usually accessed via
 *  PileUpMergeSvc::TimedList
 **/

#include <type_traits>

template <typename HIT>
class TimedHitPtr {
public:
  using element_type = std::add_const_t<HIT>;
  using value_type = HIT;
  using const_value_type = element_type;
  ///STL required constructors
  TimedHitPtr() = default;
  TimedHitPtr(const TimedHitPtr<HIT>& rhs)  = default;
  TimedHitPtr(TimedHitPtr<HIT>&& rhs)  = default;


  ///minimal constructor: pass only t0 offset of bunch xing
  TimedHitPtr(float eventTime, const HIT* pHit, int pileupType=0) :
    m_eventTime(eventTime), m_eventId(0), m_pileupType(pileupType), m_pHit(pHit) {}
  ///use this constructor when hit has a PileUpTimeEventIndex
  TimedHitPtr(float eventTime, unsigned short eventId, const HIT* pHit, int pileupType=0) :
    m_eventTime(eventTime), m_eventId(eventId), m_pileupType(pileupType), m_pHit(pHit) {}

  ///assignment operator
  TimedHitPtr<HIT>& operator=(const TimedHitPtr<HIT>& rhs) = default;
  TimedHitPtr<HIT>& operator=(TimedHitPtr<HIT>&& rhs) = default;
  ///smart pointer interface
  const HIT& operator*() const   { return *m_pHit; }
  const HIT* operator->() const  { return m_pHit; }
  const HIT* get() const { return m_pHit; }

  operator bool () const { return m_pHit != nullptr; }
  ///the index of the component event in PileUpEventInfo. Allows, in principle,
  ///to navigate back to the parent event
  unsigned short eventId() const { return m_eventId; }

  ///the type of event which the hit came from (signal, low pt
  ///minbias, high pt minbias, cavern bg, etc.)
  int pileupType() const { return m_pileupType; }

  /// t0 offset of the bunch xing containing the hit in ns. This is an integer
  /// multiple of the bunch xing distance for a certain event (e.g. N*25ns)
  float eventTime() const { return m_eventTime; }

private:

  ///t0 offset of the bunch xing in ns
  float m_eventTime{0.f};
  ///the index in PileUpEventInfo of the component event hosting this hit
  unsigned short m_eventId{0u};
  int m_pileupType{0};
  const HIT* m_pHit{nullptr}; //don't own

  template <class FHIT>
  friend float hitTime(const TimedHitPtr<FHIT>&);
};

template <class HIT>
inline float hitTime(const TimedHitPtr<HIT>& tHit) {
  return tHit.m_eventTime + hitTime(*tHit);
}



template <class HIT>
bool operator < (const TimedHitPtr<HIT>& lhs, const TimedHitPtr<HIT>& rhs ) {
  return ( lhs.get() && rhs.get() && *lhs < *rhs);
}
#endif
