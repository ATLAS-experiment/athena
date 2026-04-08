/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArByteStream/LATOMEMapping.h"
#include "Identifier/HWIdentifier.h"
#include "ByteStreamData/RawEvent.h" //for OFFLINE_FRAGMENTS_NAMESPACE
#include "GaudiKernel/MsgStream.h"
#include <iostream>
#include <fstream>


using namespace OFFLINE_FRAGMENTS_NAMESPACE;

typedef std::map<int, HWIdentifier> latome_map;

void LATOMEMapping::fill(latome_map& toFill, const std::string& inputfile,
                         MsgStream& log) {
  std::ifstream ifs(inputfile);
  if (ifs.fail()) log << MSG::ERROR << "Fail to read" << inputfile << endmsg;
  int value, key;
  while (ifs >> value >> key) {
    if (value != -999) toFill[key] = HWIdentifier(value);
  }
}
