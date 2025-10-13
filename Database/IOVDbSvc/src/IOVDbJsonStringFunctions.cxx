/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "IOVDbJsonStringFunctions.h"
#include <regex>
#include <iostream>
#include <algorithm>
#include "IOVDbStringFunctions.h"
#include "Base64Codec.h"
#include "CoralBase/Blob.h"
#include "FolderTypes.h"

namespace IOVDbNamespace{
  std::string
  jsonAttribute(const coral::Attribute & attr){
    std::ostringstream os;
    attr.toOutputStream(os);
    const std::string native=os.str();
    const bool stringPayload=(native.find(" (string) ") != std::string::npos);
    const bool blobPayload=(native.find(" (blob) ") != std::string::npos);
    const std::string sep(" : ");
    const auto separatorPosition = native.find(sep);
    const std::string payloadOnly=native.substr(separatorPosition+3);
    if (stringPayload) return quote(sanitiseJsonString(payloadOnly));
    if (blobPayload){
      return quote(IOVDbNamespace::base64Encode(attr.data<coral::Blob>()));
    }
    if (payloadOnly == "NULL") {
      return "null";
    }
    return payloadOnly;
  }

  std::string
  jsonAttributeList(const coral::AttributeList& atrlist){
    std::string os("[");
    const unsigned int nelement=atrlist.size();
    std::string delimiter(" ");
    for (unsigned int i(0);i!=nelement;++i){
      if (i==1) delimiter = s_delimiterJson;
      os+=delimiter;
      os+=jsonAttribute(atrlist[i]);
    }
    os+="]";
    return os;
  }

}

