// -*- C++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRIPDIGITIZATION_ISTRIPRADDAMAGELUTTOOL_H
#define STRIPDIGITIZATION_ISTRIPRADDAMAGELUTTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "Identifier/IdentifierHash.h"

namespace ITk {

/**
 * @class IStripRadDamageLUTTool
 * @brief Interface to the 2D charge-collection-efficiency look-up tables for ITk Strip sensors.
 */

class IStripRadDamageLUTTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IStripRadDamageLUTTool, 1, 0);

  virtual ~IStripRadDamageLUTTool() = default;

  /// CCE for the strip at offset dy from a deposit at depth z
  virtual float cce(const IdentifierHash& hash, float z, float dy) const = 0;

  /// Half-thickness of the sensor in the LUT frame [mm] (e.g. 0.152 for a 304 um simulation).
  virtual float zHalfRange(const IdentifierHash& hash) const = 0;

  /// Largest |dy| of strip sensors covered by the table [mm]
  virtual float maxOffset(const IdentifierHash& hash) const = 0;

  /// Random Gaussian smearing to apply to charge groups (0 = off)
  virtual float chargeSmear() const = 0;
};

} // namespace ITk

#endif // STRIPDIGITIZATION_ISTRIPRADDAMAGELUTTOOL_H
