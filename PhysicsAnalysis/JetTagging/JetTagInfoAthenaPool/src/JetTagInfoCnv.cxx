/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///
/// The Athena Pool converter
///

#include "JetTagInfoCnv.h"

#include <stdexcept>

using std::string;
using std::runtime_error;
using Analysis::JetTagInfo_tlp1;
using Analysis::JetTagInfoCnv_tlp1;
using Analysis::JetTagInfo_tlp2;
using Analysis::JetTagInfoCnv_tlp2;
using Analysis::JetTagInfo_tlp3;
using Analysis::JetTagInfoCnv_tlp3;

/// Init our base class.
JetTagInfoCnv::JetTagInfoCnv (ISvcLocator *loc)
  : JetTagInfoCnvBase (loc)
{
}

/// Make sure to pass on the init!
StatusCode JetTagInfoCnv::initialize(void)
{
  StatusCode result = JetTagInfoCnvBase::initialize();
  return result;
}

/// Read the persistant bit in. Fortunately, we don't have to do
/// anything at all for this.
void JetTagInfoCnv::readObjectFromPool (const Token* token)
{
  static const pool::Guid p1_guid ("977E2E76-4DA6-4A4B-87A8-2E41353DB9F4");
  static const pool::Guid p2_guid ("50E644C1-E247-41B0-B873-416362824A21");
  static const pool::Guid p3_guid ("8A57BABD-C361-4796-93CD-E8171EF06BC7");

  /// And use the GUID to select what we want

  if(compareClassGuid(token, p3_guid)){
    poolReadObject<JetTagInfo_tlp3> (m_TPConverter, token);
  }
  else if (compareClassGuid(token, p2_guid)) {
    poolReadObject<JetTagInfo_tlp2> (m_TPConverter_p2, token);
  } else if (compareClassGuid(token, p1_guid)) {
    poolReadObject<JetTagInfo_tlp1> (m_TPConverter_p1, token);
  } else {
    throw runtime_error ("Unsupported version of JetTagInfo_tlp1 (unknonw GUID)");
  }
}
