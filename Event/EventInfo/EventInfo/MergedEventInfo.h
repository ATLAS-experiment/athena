/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EVENTINFO_MERGEDEVENTINFO_H
# define EVENTINFO_MERGEDEVENTINFO_H 1
/**
 * @file MergedEventInfo.h
 *
 * @brief This class provides general information about an event.
 *  It extends MergedEventInfo with a list of sub-evts (the original
 *  and the bkg ones)
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov>
 */

#include "GaudiKernel/ClassID.h"
#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"

class EventType;
class TriggerInfo;



/** @class MergedEventInfo
 *
 * @brief This class provides general information about an event.
 *  It extends MergedEventInfo with a list of sub-evts (the original
 *  and the bkg ones)
 *
 **/

class MergedEventInfo: public EventInfo {
public:
  /// \name structors
  //@{
  MergedEventInfo();  ///< POOL required
  // Use default copy constructor.
  /// the constructor to be used
  MergedEventInfo(const EventInfo& origEvent,
                  EventID::number_type newRunNo,
                  EventID::number_type newEvtNo,
                  EventID::number_type newTimeStamp = 0);
  virtual ~MergedEventInfo();

  MergedEventInfo(const MergedEventInfo&) = default;
  MergedEventInfo(MergedEventInfo&&) = default;
  MergedEventInfo& operator=(const MergedEventInfo&) = default;
  MergedEventInfo& operator=(MergedEventInfo&&) = default;
  //@}

  /// \name DataObject-like clid accessors
  //@{
  static const CLID& classID();
  const CLID& clID() const;
  //@}

  /// \name Event information accessors
  //@{
  ///the new identification of the event.
  const EventID* event_ID() const;
  //INHERITED EventType* event_type() const;
  //INHERITED TriggerInfo* trigger_info() const;
  ///the original identification of the event.
  const EventID* origEvent_ID() const;
  //@}
private:
  EventID m_newEventID;
};



inline const EventID*
MergedEventInfo::event_ID() const {
  return &m_newEventID;
}

inline const EventID*
MergedEventInfo::origEvent_ID() const {
  return EventInfo::event_ID();
}

inline const CLID&
MergedEventInfo::clID() const {
  return classID();
}

# include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(MergedEventInfo, 220174395, 1)

inline const CLID &
MergedEventInfo::classID() {
  return ClassID_traits<MergedEventInfo>::ID();
}

#endif // MERGEDEVENTINFO_MERGEDEVENTINFO_H
