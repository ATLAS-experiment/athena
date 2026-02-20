/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_SRCIDMAP_H
#define BYTESTREAMCNVSVC_SRCIDMAP_H


/** @class SrcIdMap
    @brief This class provides default conversion between Lower level Source ID to
    higher level source ID. 

    This is to be used in assembling the fragments from ROD fragments

    @author 	Hong Ma 
    July 14, 2002 

*/


#include <cstdint>

class SrcIdMap {

public:
  /** constructor
  */
  SrcIdMap(); 

  /** @brief Make a ROB Source ID from a ROD source ID
  */ 
  uint32_t getRobID  ( uint32_t rod_id) const;

  /** @brief Make a ROS Source ID from a ROB source ID
  */ 
  uint32_t getRosID  ( uint32_t rob_id) const;

  /** @brief Make a SubDetector ID from ROS source ID 
  */
  uint32_t getDetID  ( uint32_t ros_id) const;

};

#endif 
