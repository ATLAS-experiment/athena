/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_MM_RAWDATA_H
#define MUONRDO_MM_RAWDATA_H

#include <inttypes.h>
#include <vector>
#include <iosfwd>
#include "Identifier/Identifier.h"

class MsgStream;

namespace Muon {

/**
Temporary class to hold the MM RDO.
*/
class MM_RawData final{

  friend class MM_RawDataCnv_p1;
  friend class MM_RawDataCnv_p2;
  friend class MM_RawDataCnv_p3;

 private:

  // not sure whether the offline identifier is needed
  Identifier m_id{};

  /// channel Id
  int m_channel{0};
  /// tdc counts or ns
  int m_time{0};
  /// adc counts or electrons
  int m_charge{0};
  /// rel bcid
  uint16_t m_relBcid{0};
  //indicator if the charge and time are in counts or physical units
  bool m_timeAndChargeInCounts{false};


 public:

  MM_RawData ( const Identifier& id ) : m_id(id) {}

  MM_RawData ( const Identifier& id, const int channel,
	       const int time, const int charge, const uint16_t relBcid, bool timeAndChargeInCounts)
    :  m_id(id), m_channel(channel), m_time(time), m_charge(charge), m_relBcid(relBcid), m_timeAndChargeInCounts(timeAndChargeInCounts) {}

  MM_RawData() = default;
  MM_RawData(const MM_RawData&) = default;
  MM_RawData(MM_RawData&&) = default;
  MM_RawData& operator=(const MM_RawData&) = default;
  MM_RawData& operator=(MM_RawData&&) = default;
  ~MM_RawData() = default;

  const Identifier& identify() const { return m_id; }

  // access functions to the online data
  int channel() const { return m_channel; }
  int time() const { return m_time; }
  uint16_t relBcid() const { return m_relBcid; }
  int charge() const { return m_charge; }
  bool timeAndChargeInCounts() const {return m_timeAndChargeInCounts;}


  // Lower time bound of digits selected at the end of digitization, excluding the peaktime.
  // As of June 2022, the main event is assigned to relative_BCID=1, which is set to correspond
  // to the time interval [-12.5ns, +12.5ns]. The digits within the BC window [0, +7] are kept.
  // Therefore the lower time bound is -37.5 ns.
  static constexpr double s_lowerTimeBound{-37.5};

  // BC window
  static constexpr int s_BCWindow{8};

};
}

/**Overload of << operator for MsgStream for debug output*/
MsgStream& operator << ( MsgStream& sl, const Muon::MM_RawData& coll);

/**Overload of << operator for std::ostream for debug output*/
std::ostream& operator << ( std::ostream& sl, const Muon::MM_RawData& coll);


#endif







