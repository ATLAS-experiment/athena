/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "HLTSrcIdMap.h" 
#include "eformat/SourceIdentifier.h"

// implementation of HLTSrcIdMap
//    Map ROD Source ID to higher level Fragment Source ID.  
//    

uint32_t HLT::HLTSrcIdMap::getRobID( uint32_t rod_id ) const
{
 eformat::helper::SourceIdentifier helpHLTResultID(m_detId, rod_id); 
 return  helpHLTResultID.code(); 
}

uint32_t HLT::HLTSrcIdMap::getRosID( uint32_t rob_id) const
{
 return getRobID(rob_id);
}

uint32_t HLT::HLTSrcIdMap::getDetID  ( uint32_t ros_id) const
{
 return getRobID(ros_id);
}

