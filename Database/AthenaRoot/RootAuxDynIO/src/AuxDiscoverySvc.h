/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AUXDISCOVERYSVC_H
#define AUXDISCOVERYSVC_H

/** @file AuxDiscoverySvc.h
 *  @brief This file contains the class definition for the AuxDiscoverySvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaBaseComps/AthMessaging.h"
#include "RootAuxDynIO/IRootAuxDynIO.h"

// Forward declarations
class AthenaPoolAuxStore;

/** @class AuxDiscoverySvc 
 *  @brief This class provides the interface between AthenaPoolCnvSvc and AuxStore classes.
 **/
class AuxDiscoverySvc : public AthMessaging, public RootAuxDynIO::IAuxDynShare {
public:
   AuxDiscoverySvc(const IAthenaSerializeSvc* serSvc, IAthenaIPCTool* ipcTool);
   ~AuxDiscoverySvc();

   /// Receive dynamic aux store variables from streaming tool
   StatusCode receiveStore(TClass* cl, void* obj, int num = 0);

   /// Send dynamic aux store variables to streaming tool
   StatusCode sendStore(TClass* cl,
                        const void* obj,
                        const std::string& classId,
                        const std::string& contName,
                        int num = 0);
private:
   const IAthenaSerializeSvc* m_serSvc{};
   IAthenaIPCTool* m_ipcTool{};
};

#endif
