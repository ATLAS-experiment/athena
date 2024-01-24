/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelConditionsData/PixelConditionsDataStringUtils.h"
#include <sstream> //std::istringstream
#include <stdexcept> //for std::runtime_error


namespace PixelConditionsData{
 //
  std::vector<std::string> 
  getParameterString(const std::string& varName, const std::vector<std::string>& buffer){
    std::string sParam = "";
    std::string sMessage = "";

    for (size_t i=0; i<buffer.size(); i++) {
      if (buffer[i].find(varName.c_str())!=std::string::npos) {
        std::istringstream iss(buffer[i]);
        std::string s;
        bool chkParam = false;
        bool chkMessage = false;
        while (iss >> s) {
          if (s.find("{{")!=std::string::npos) {
            sParam += s.substr(2,s.length());
            chkParam = true;
          }
          else if (s.find("},")!=std::string::npos) {
            sParam += s.substr(0,s.length()-2);
            sParam += ",";
          }
          else if (s.find("}}")!=std::string::npos) {
            sParam += s.substr(0,s.length()-2);
            chkParam = false;
            chkMessage = true;
          }
          else if (s.find("{")!=std::string::npos && s.find("}")!=std::string::npos) {
            sParam += s.substr(1,s.length()-1);
          }
          else if (s.find("{")!=std::string::npos) {
            sParam += s.substr(1,s.length());
            chkParam = true;
          }
          else if (s.find("}")!=std::string::npos) {
            sParam += s.substr(0,s.length()-1);
            chkParam = false;
            chkMessage = true;
          }
          else if (chkParam==true) {
            sParam += s;
          }
          else if (chkMessage==true) {
            sMessage += " " + s;
          }
        }
      }
    }
    if (sParam.empty()) {
      throw std::runtime_error ("PixelConfigCondAlg::getParameterString() Input variable " + varName + " was not found. " );
    }

    std::vector<std::string> vParam;
    int offset = 0; 
    for (;;) {
      auto pos = sParam.find(",",offset);
      if (pos==std::string::npos) {
        vParam.push_back(sParam.substr(offset,pos));
        break;
      }
      vParam.push_back(sParam.substr(offset,pos-offset));
      offset = pos + 1;
    }

    std::vector<std::string> vvParam;
    for (const auto & param : vParam) {
      if (param.find("\"")!=std::string::npos) {
        if (vParam.size()==1) {
          vvParam.push_back(param.substr(1,param.length()-3));
        } else {
          vvParam.push_back(param.substr(1,param.length()-2));
        }
      } else {
        vvParam.push_back(param);
      }
    }
    return vvParam;
  }
  
}
