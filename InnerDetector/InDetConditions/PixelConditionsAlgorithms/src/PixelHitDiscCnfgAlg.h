/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/PixelHitDiscCnfgAlg.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Store FEI4 HitDiscConfig parameter in PixelHitDiscCnfgData.
 */

#ifndef PIXELHITDISCCNFGALG_H
#define PIXELHITDISCCNFGALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelHitDiscCnfgData.h"
#include "PersistentDataModel/AthenaAttributeList.h"


class PixelHitDiscCnfgAlg : public AthCondAlgorithm {
  public:
    PixelHitDiscCnfgAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~PixelHitDiscCnfgAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:

    SG::ReadCondHandleKey<AthenaAttributeList> m_readKey
    {this, "ReadKey", "/PIXEL/HitDiscCnfg", "Input HitDiscCnfg folder"};

    SG::WriteCondHandleKey<PixelHitDiscCnfgData> m_writeKey
    {this, "WriteKey", "PixelHitDiscCnfgData", "Output HitDiscCnfg data"};

};

#endif
