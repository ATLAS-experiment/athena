/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BLM_Module.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef BLMGEOMODEL_BLMMODULE_H
#define BLMGEOMODEL_BLMMODULE_H

class BLM_ModuleParameters;
class StoredMaterialManager;
class MsgStream;
class GeoPhysVol;

  /** @class BLM_Module
      @brief Beam Loss Monitor module builder
      @author  Bostjan Macek <bostjan.macek@cern.ch>
  */

class BLM_Module
{
 public:
  GeoPhysVol* Build(StoredMaterialManager* mat_mgr, const BLM_ModuleParameters* parameters, MsgStream* msg);

};

#endif
