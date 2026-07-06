/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETIDENTIFIER_BCMPRIME_ID_H
#define INDETIDENTIFIER_BCMPRIME_ID_H

#include "InDetIdentifier/PixelID.h"

/**
 * @class BCMPrime_ID
 *
 * Identifier helper for BCMPrime. BCMPrime is a diamond detector, but it reuses
 * the pixel-like solid-state identifier machinery for pad-level readout geometry.
 */
class BCMPrime_ID final: public PixelID
{
public:
  BCMPrime_ID(const std::string& name = "BCMPrime_ID", const std::string& group = "pixel");

  AtlasDetectorID::HelperType helper() const override final
  {
    // AtlasDetectorID has no dedicated BCMPrime helper category in this release.
    // Use the luminosity-detector pixel-like category while keeping a BCMPrime helper class.
    return AtlasDetectorID::HelperType::PLR;
  }

  int initialize_from_dictionary(const IdDictMgr& dict_mgr) override;

private:
  int initLevelsFromDict();
  int bcm_field_value() const;

  size_type m_LUMI_INDEX{1};
  size_type m_BCM_INDEX{2};

  IdDictFieldImplementation m_lumi_impl;
  IdDictFieldImplementation m_bcm_impl;
};

CLASS_DEF(BCMPrime_ID, 162824295, 1)

#endif // INDETIDENTIFIER_BCMPRIME_ID_H
