/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FolderAddressResolver.h"

#include "GaudiKernel/IClassIDSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "IOVDbParser.h"
#include "IOVDbStringFunctions.h"

namespace IOVDbNamespace {

bool resolveFolderAddress(MsgStream& log,
                          const IOVDbParser& parsedDescription,
                          const std::string& folderName,
                          bool jokey,
                          const std::string& currentKey,
                          IClassIDSvc* clidSvc,
                          FolderAddressSpec& result) {
  // Check for key, giving a different key to the foldername
  result.key = currentKey;
  if (auto newkey = parsedDescription.key(); not newkey.empty() and not jokey) {
    log << MSG::DEBUG << "Key for folder " << folderName << " set to " << newkey
        << " from description string" << endmsg;
    result.key = std::move(newkey);
  }

  // Check for <named/>
  result.named = parsedDescription.named();

  // Get addressHeader
  if (auto newAddrHeader = parsedDescription.addressHeader(); not newAddrHeader.empty()) {
    IOVDbNamespace::replaceServiceType71(newAddrHeader);
    result.addrheader = std::move(newAddrHeader);
  }

  // Get clid, if it exists (set to zero otherwise)
  result.clid = parsedDescription.classId(log);
  // Decode the typeName
  if (!parsedDescription.getKey("typeName", "", result.typeName)) {
    log << MSG::ERROR << "Primary type name is empty" << endmsg;
    return false;
  }
  bool gotCLID = (result.clid != 0);

  log << MSG::DEBUG << "Got folder typename " << result.typeName << endmsg;

  // Only consult the service when the description supplied no clid. It may
  // legitimately be null.
  if (!gotCLID && clidSvc) {
    if (StatusCode::SUCCESS == clidSvc->getIDOfTypeName(result.typeName, result.clid)) {
      gotCLID = true;
    }
  }

  if (!gotCLID) {
    log << MSG::ERROR << "Could not get clid for typeName: " << result.typeName << endmsg;
    return false;
  }

  log << MSG::DEBUG << "Got folder typename " << result.typeName
      << " with CLID " << result.clid << endmsg;
  return true;
}

}  // namespace IOVDbNamespace
