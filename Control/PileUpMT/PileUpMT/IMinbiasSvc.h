/* -*- C++ -*- */
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IMINBIASSVC_H
#define IMINBIASSVC_H

#include <cstdint>
#include <vector>

#include "GaudiKernel/IService.h"
#include "StoreGate/StoreGateSvc.h"

class IMinbiasSvc : virtual public IService {
 public:
  virtual StatusCode beginHardScatter(
      const EventContext& ctx) = 0;  /// Tell IMinbiasSvc we're
                                     /// starting a hard scatter
  virtual StoreGateSvc* getMinbias(
      const EventContext& ctx,
      std::uint64_t mb_id) = 0;  /// Get a minbias event
  virtual std::size_t getNumForBunch(
      const EventContext& ctx,
      int bunch) const = 0;  /// Return number of minbias events
                             /// to use for a specific bunch crossing

  virtual std::int64_t get_hs_id(const EventContext& ctx) const = 0;
  virtual StatusCode endHardScatter(
      const EventContext& ctx) = 0;  /// Tell IMinbiasSvc we're done with
                                     /// a hard scatter

  /// Create InterfaceID
  DeclareInterfaceID(IMinbiasSvc, 1, 1);
};

#endif  // IMINBIASSVC_H
