/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigConfHLTUtils/HLTUtils.h"
#include "TrigCompositeUtils/HLTIdentifier.h"


HLT::Identifier::Identifier( const std::string& stringID )
  : m_id( TrigConf::HLTUtils::string2hash( stringID, "Identifier" ) ) {}

std::string HLT::Identifier::name() const {
  return TrigConf::HLTUtils::hash2string( numeric(), "Identifier" );
}

MsgStream& operator<< ( MsgStream& m, const HLT::Identifier& id ) {
  m << id.name() << " ID#" << id.numeric();
  return m;
}

HLT::Identifier HLT::Identifier::fromToolName( const std::string& tname ) {
  return HLT::Identifier(  tname.substr( tname.find('.') + 1 ) );
}
