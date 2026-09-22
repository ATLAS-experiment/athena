/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// FolderAddressResolver.h
// Backend-neutral resolution of the address-construction fields (SG key,
// named-channel flag, address header, CLID) from a folder's parsed description string

#ifndef IOVDBSVC_FOLDERADDRESSRESOLVER_H
#define IOVDBSVC_FOLDERADDRESSRESOLVER_H

#include "GaudiKernel/ClassID.h"

#include <string>

class MsgStream;
class IOVDbParser;
class IClassIDSvc;

namespace IOVDbNamespace {

// Resolved address-construction fields for a folder.
struct FolderAddressSpec {
  std::string key;           ///< SG key (unchanged unless the description overrides it)
  bool        named{false};  ///< Folder has named channels
  std::string addrheader;    ///< Address header string from the description. Empty if it has none
  std::string typeName;      ///< Primary type name, read from the description
  CLID        clid{0};       ///< CLID, read from the description or resolved via IClassIDSvc
};

// Fills result from parsedDescription's SG key, named-channel flag, address
// header, and CLID. If jokey is true, currentKey is kept and the description's
// <key> is ignored. clidSvc may be null: it resolves the CLID only when the
// description has none. Returns false, with an ERROR already logged, if
// typeName is empty or no CLID resolves.
bool resolveFolderAddress(MsgStream& log,
                          const IOVDbParser& parsedDescription,
                          const std::string& folderName,
                          bool jokey,
                          const std::string& currentKey,
                          IClassIDSvc* clidSvc,
                          FolderAddressSpec& result);

}  // namespace IOVDbNamespace

#endif  // IOVDBSVC_FOLDERADDRESSRESOLVER_H
