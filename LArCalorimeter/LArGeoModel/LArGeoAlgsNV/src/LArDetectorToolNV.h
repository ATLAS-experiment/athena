/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file LArDetectorToolNV.h
 *
 * @brief Declaration of LArDetectorToolNV class
 *
 */

#ifndef LARGEOALGSNV_LARDETECTORTOOLNV_H
#define LARGEOALGSNV_LARDETECTORTOOLNV_H

#include "GeoModelUtilities/GeoModelTool.h"
#include "LArGeoCode/LArAlignHelper.h"

class LArDetectorManager;

/**
 * @class LArDetectorToolNV
 *
 * @brief LArDetectorToolNV is a standard GeoModel tool, which calls LArDetectorFactory::create(),
 * stores LArDetectorManager to the Detector Store and also implements the
 * align() function which applies misalignments on top of the 'regular' geometry.
 *
 **/

class LArDetectorToolNV final : public GeoModelTool {
 public:
  // Standard Constructor
  LArDetectorToolNV( const std::string& type, const std::string& name, const IInterface* parent );

  // Standard Destructor
  virtual ~LArDetectorToolNV();

  // Create Method:
  virtual StatusCode create() override;

  // Clear Method
  virtual StatusCode clear() override;

  // Apply alignments (for simulation only)
  virtual StatusCode align() override;

 private:

  Gaudi::Property<bool> m_barrelSaggingOn{this,"SaggingBarrelAccordeon",false};
  Gaudi::Property<int>  m_barrelVisLimit{this,"BarrelCellVisLimit",-1};
  Gaudi::Property<int>  m_fcalVisLimit{this,"FCALVisLimit",-1};

  Gaudi::Property<bool> m_buildBarrel{this,"BuildBarrel",true};
  Gaudi::Property<bool> m_buildEndcap{this,"BuildEndcap",true};

  Gaudi::Property<bool> m_applyAlignments{this,"ApplyAlignments",false};

  const LArDetectorManager *m_manager{};

  Gaudi::Property<std::string> m_geometryConfig{this,"GeometryConfig","FULL"}; // FULL, SIMU, RECO

  Gaudi::Property<std::string> m_EMECVariantInner{this,"EMECVariantInner","Wheel"};
  Gaudi::Property<std::string> m_EMECVariantOuter{this,"EMECVariantOuter","Wheel"};

  Gaudi::Property<bool> m_activateFT{this,"ActivateFeedThrougs",true};
  Gaudi::Property<bool> m_enableMBTS{this,"EnableMBTS",true};

  LArAlignHelper m_alignHelper;
};

#endif
