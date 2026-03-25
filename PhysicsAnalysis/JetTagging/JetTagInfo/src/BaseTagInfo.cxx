/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           BaseTagInfo.cxx  -  Description
                             -------------------
    begin   : January 2005
    authors : Andreas Wildauer (CERN PH-ATC), Fredrik Akesson (CERN PH-ATC)
    email   : andreas.wildauer@cern.ch, fredrik.akesson@cern.ch
    changes :

 ***************************************************************************/

#include "JetTagInfo/BaseTagInfo.h"

namespace Analysis
{
  /** constructor with info type */
  BaseTagInfo::BaseTagInfo(const TagInfoType& tagJetInfoType) : JetTagInfoBase(),
          m_tagJetInfoType(tagJetInfoType)
  {
  
  }
}
