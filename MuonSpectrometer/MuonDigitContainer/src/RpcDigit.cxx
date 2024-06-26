/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// RpcDigit.cxx

#include "MuonDigitContainer/RpcDigit.h"

//**********************************************************************
// Member functions.
//**********************************************************************
 
//**********************************************************************

// Full constructor from Identifier.

RpcDigit::RpcDigit(const Identifier& id, float time, float ToT, bool stripSide):
    MuonDigit(id), m_time{time}, m_ToT{ToT}, m_stripSide{stripSide} { }



