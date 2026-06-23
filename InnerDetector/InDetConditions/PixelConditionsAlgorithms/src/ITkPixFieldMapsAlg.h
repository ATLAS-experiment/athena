/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/ 
/**
 * @file PixelConditionsAlgorithms/ITkPixFieldMapsAlg.h
 * @author Shaun Roe, based on code by Soshi Tsuno 
 * @date June 2026
 * @brief Store pixel device field map parameters in ITkPixFieldMaps.
 */

#ifndef PixelConditionsAlgorithms_ITkPixFieldMapsAlg_h
#define PixelConditionsAlgorithms_ITkPixFieldMapsAlg_h

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/ITkPixFieldMaps.h"

class ITkPixFieldMapsAlg : public AthCondAlgorithm {
  public:
    ITkPixFieldMapsAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    Gaudi::Property<std::string> m_efieldFilePath
    {this, "EFieldFilePath", "PixelDigitization/maps_IBL_PL_80V_fl0e14.root", "File path for EField files"};

    SG::WriteCondHandleKey<ITkPixFieldMaps> m_writeFieldMapKey
    {this, "ITkPixFieldMapsKey", "ITkPixFieldMapsKey", "Output key for efield data"};
};

#endif
