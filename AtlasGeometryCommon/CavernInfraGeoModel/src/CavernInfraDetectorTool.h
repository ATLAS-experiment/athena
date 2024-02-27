/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORTOOL_H
#define CAVERNINFRAGEOMODEL_CAVERNINFRADETECTORTOOL_H

#include "GeoModelUtilities/GeoModelTool.h"
class CavernInfraDetectorManager;

class CavernInfraDetectorTool final : public GeoModelTool 
{
 public:
  CavernInfraDetectorTool( const std::string& type, const std::string& name, const IInterface* parent );
  ~CavernInfraDetectorTool() = default;

  virtual StatusCode create() override;
  virtual StatusCode clear() override;
  
 private:
  const CavernInfraDetectorManager* m_manager{nullptr};
};

#endif 
