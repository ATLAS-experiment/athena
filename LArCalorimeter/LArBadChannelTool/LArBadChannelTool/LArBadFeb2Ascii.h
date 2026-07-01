/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LArBadFeb2Ascii_H
#define LArBadFeb2Ascii_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"

#include <string>


class LArBadFeb2Ascii : public AthAlgorithm 
{
public:
  //Delegate constructor
  using AthAlgorithm::AthAlgorithm;
  ~LArBadFeb2Ascii()=default;

  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

private:
  SG::ReadCondHandleKey<LArBadFebCont> m_BFKey{this,"BFKey","LArBadFeb"};
  Gaudi::Property<std::string> m_fileName{this,"FileName",""};

};

#endif
