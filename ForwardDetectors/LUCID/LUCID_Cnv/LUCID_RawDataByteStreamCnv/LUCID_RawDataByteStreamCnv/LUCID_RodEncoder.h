/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_RODENCODER_H
#define LUCID_RODENCODER_H

#include <inttypes.h>

#include "ByteStreamData/RawEvent.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"

#include "LUCID_RawEvent/LUCID_Digit.h"
#include "LUCID_RawEvent/LUCID_RawData.h"

class LUCID_RodEncoder
{
public:
  typedef std::vector<const LUCID_Digit*> VDIGIT;
  struct Cache
  {
    unsigned int hitcounter0 = 0;
    unsigned int hitcounter1 = 0;
    unsigned int hitcounter2 = 0;
    unsigned int hitcounter3 = 0;
    VDIGIT Digits{};
  };

  LUCID_RodEncoder();
  ~LUCID_RodEncoder();

  void addDigit(const LUCID_Digit* digit, Cache& cache) const
  {
    cache.Digits.push_back(digit);
  }
  void encode(std::vector<uint32_t>& data_block,
              Cache& cache,
              MsgStream& log) const;

  const VDIGIT& getDigits(Cache& cache) const { return cache.Digits; }

private:
};

#endif
