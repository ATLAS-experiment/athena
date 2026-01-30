/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVC_FULLEVENTASSEMBLER_H
#define BYTESTREAMCNVSVC_FULLEVENTASSEMBLER_H

#include <cstdint>
#include <map>

#include "ByteStreamCnvSvcBase/SrcIdMap.h"
#include "ByteStreamData/RawEvent.h" 

class MsgStream;

/** @class FullEventAssemblerBase
    @brief base class for assembling a full atlas raw event from subfragments

*/ 

class FullEventAssemblerBase {

 public:
   FullEventAssemblerBase( ) { } ;
   virtual ~FullEventAssemblerBase( ) { } ;

   virtual void fill(RawEventWrite* re, MsgStream& log) = 0 ; 
   virtual void clear() = 0 ; 
};

/** @class FullEventAssembler
    @brief Template class for assembling a full atlas raw event from subfragments

*/ 


template <class IDMAP=SrcIdMap> 
class FullEventAssembler:public FullEventAssemblerBase {


 public:
   FullEventAssembler( );
   ~FullEventAssembler( );
   // type aliases for ROD data, and maps

   /** @brief type for Identifier mapping
    */
   using IDMAP_t = IDMAP;

   /** @brief ROD data as a vector of unsigned int
    */
   using RODDATA = std::vector<uint32_t>;

   /** @brief map of ROD data, indexed by ROD ids
    */
   using RODMAP = std::map< uint32_t, RODDATA* >;

   /** @brief map of ROB data, indexed by ROB ids
    */
   using ROBMAP = std::map< uint32_t, OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment* >; 

   /** @brief get a block of ROD data 

	if it does not exist, create it 
	the users is supposed to fill the container with data. 
   */ 
   RODDATA* getRodData(uint32_t id ); 

   /** @brief Fill the FullEventFragment with all the ROD data stored in this.
   */ 
   void fill(RawEventWrite* re, MsgStream& log); 

   /** @brief begin iterator for all existing ROD 
	*/
   RODMAP::const_iterator begin() const; 

   /** @brief end iterator for all existing ROD 
	*/
   RODMAP::const_iterator end() const; 

   /** @brief change the ROD minor version
	*/
   void setRodMinorVersion(uint16_t m ); 

   /** @brief change the ROB minor version
	*/
   void setRobMinorVersion(uint16_t m ); 

   /** @brief change Detector Event Type
	*/
   void setDetEvtType(uint32_t m ); 

   /** @brief change LVL1 Trigger Type
	*/
   void setLvl1TriggerType(uint8_t m ); 

   /** @brief Return reference to IDMAP 
	*/ 
   IDMAP_t& idMap(); 

   /** @brief Clear internal stack
   */ 
   void clear();

private:

   /** @brief converting from ROD to ROB 
    */
   void RodToRob(RODMAP& rodMap, ROBMAP& robMap, MsgStream& log ) const;

   /** @brief converting from SubDetector to FullEvent
    */
   void RobToEvt(ROBMAP& robMap, RawEventWrite* re, MsgStream& log ) const;

private:
   IDMAP_t m_idmap; 

   RODMAP m_rodMap; 
   ROBMAP m_robMap; 

   uint32_t m_runnum  ; 
   uint32_t m_lvl1_id ; 

   // minor version
   uint16_t m_rod_version; 
   uint16_t m_rob_version; 

   uint32_t m_detEvtType  ; 
   // level 1 trigger type
   uint8_t m_lvl1_trigger_type  ; 
   uint16_t m_bcid  ; 
};

#include "ByteStreamCnvSvcBase/FullEventAssembler.icc" 

#endif



