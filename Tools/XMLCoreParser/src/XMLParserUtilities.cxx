/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file XMLCoreParser/src/XMLParserUtilities.cxx
 * @author Shaun Roe
 * @date April, 2026
 * @brief Various text/ptr utilities to use in the ExpatCoreParser.
 */
 
#include "XMLParserUtilities.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace XmlParser{
   std::string
   xmlFileName(const std::string & fname, const std::string & prefix){
     if (fs::exists(fname) ) return fname;
     std::string fileWithPrefix = prefix + "/" + fname;
     if (fs::exists(fileWithPrefix) ) return fileWithPrefix;
     //
     const char* xmlpathenv = std::getenv ("XMLPATH");
     if (xmlpathenv == nullptr) return "";
     std::string xmlpath = xmlpathenv;
     std::string::size_type pos = 0;
     std::string temp_name;
     while (pos != std::string::npos){
       std::string::size_type sep = xmlpath.find (":", pos);
       if (sep == std::string::npos){
        temp_name = xmlpath.substr (pos);
        pos = std::string::npos;
       } else {
        temp_name = xmlpath.substr (pos, sep - pos);
        pos = sep + 1;
       }
       if (temp_name.empty()) continue;
       std::string last_temp_name = temp_name;
       temp_name += "/";
       temp_name += fname;
       if (fs::exists (temp_name)) return temp_name;
      // Test whether prefix is a relative path and if so use it
      if (prefix != "" && '/' != prefix[0]) {
        temp_name =  std::move(last_temp_name);
        temp_name += "/";
        temp_name += prefix;
        temp_name += "/";
        temp_name += fname;
        if (fs::exists (temp_name)) return temp_name;
      }
    }
    return "";
   }


}