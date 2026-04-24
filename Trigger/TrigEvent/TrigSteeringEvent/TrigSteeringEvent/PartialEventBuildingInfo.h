/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS
#ifndef TRIGSTEERINGEVENT_PARTIALEVENTBUILDINGINFO_H
#define TRIGSTEERINGEVENT_PARTIALEVENTBUILDINGINFO_H
#include "AthenaKernel/CLASS_DEF.h"
#include "eformat/SourceIdentifier.h"

#include <set>
#include <stdint.h>
#include <vector>

class PartialEventBuildingInfo {
public:
  PartialEventBuildingInfo();
  ~PartialEventBuildingInfo();
  /** @brief adds ROB to be added to partially built event
   * @warning obsolete
   */
  void add( uint32_t rob_id );  

  /** @brief   adds collection of robs to partially built event 
   * @warning obsolete
   */
  void add( const std::vector<uint32_t>& rob_ids ); 

  /** @brief  gets back all ROBs added (fills up rob_ids vector given by reference)
   * @warning note that vector is not cleared
   * @warning obsolete
   */
  void get( std::vector<uint32_t>& rob_ids ) const; 



  /** @brief adds ROB to be added to partially built event
   *
   */
  void addROB( uint32_t rob_id );

  /** @brief adds collection of robs to partially built event
   *
   */
  void addROBs( const std::vector<uint32_t>& rob_ids ); 

  /** @brief gets back all ROBs added (fills up rob_ids vector given by reference)
   * @warning note that vector is not cleared)
   *
   */
  void getROBs( std::vector<uint32_t>& rob_ids ) const; 

  /** @brief gets back all ROBs added 
   */
  const std::set<uint32_t>& getROBs() const; 

  /** @brief add detector  to be in partially built event
   *
   */
  void addSubDetector( eformat::SubDetector id );  
  void addSubDetector( std::vector<eformat::SubDetector> id );  
  void addSubDetector( eformat::SubDetectorGroup id );  

  /** @brief gets back all SubDetectorss added (fills up ids vector given by reference,)
   * @warning note that vector is not cleared
   *
   */
  void getSubDetectors( std::vector<eformat::SubDetector>& ids ) const; 
   

  /** @brief gets back all SubDetectorss added
   *
   */
  const std::set<uint32_t>&  getSubDetectors(  ) const; 

  /** @brief clears the list of ROBs and Dets
   *
   **/
  void clear();

  /** @brief merges two PEBI objects
   * I.e. it the subject of the method invocation will have ROBs and SubDets list which is union of the two
   **/
  void merge(const PartialEventBuildingInfo& peb);


private:
  std::set<uint32_t> m_robs;         //!< selected ROBs for partial event building 
  std::set<uint32_t> m_subDetectors; //!< selected SubDetectors 
};

#endif
#endif //XAOD_ANALYSIS
