/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ABBAMAPPING_H
#define ABBAMAPPING_H
#include <map>
#include "LArByteStream/LArABBADecoder.h"
class MsgStream;

class ABBAMapping
{
public:
  static void fill(std::map<int, HWIdentifier> *toFill, int iphi, MsgStream& log);

};

#endif // ABBAMAPPING_H

