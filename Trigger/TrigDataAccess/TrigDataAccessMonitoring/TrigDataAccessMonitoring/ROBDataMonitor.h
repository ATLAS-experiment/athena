//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef ROBDATAMONITOR_H
#define ROBDATAMONITOR_H

#include "eformat/SourceIdentifier.h"
#include <iostream>
#include <string>
#include <map>

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"

namespace robmonitor {

  /**
   * A structure with data about ROB properties 
   */
  enum ROBHistory { 
    UNCLASSIFIED      = 0,  // ROB was requested but never arrived at processor. History unknown. 
    RETRIEVED         = 1,  // ROB was retrieved from ROS by DataCollector
    HLT_CACHED        = 2,  // ROB was found already in the internal cache of the ROBDataProviderSvc
    DCM_CACHED        = 4,  // ROB was found already in the internal cache of the DCM
    IGNORED           = 8,  // ROB was on the "ignore" list and therefore not retrieved 
    UNDEFINED         = 16, // ROB was not on the "enabled" list, should not happen
    NUM_ROBHIST_CODES = 6   // number of different history codes
  };

  /**
   * A structure with data about ROB properties 
   */
  class ROBDataStruct {
  public:

    /** @brief default constructor */
    ROBDataStruct() = default;

    /** @brief constructor
        @param  ROB Source ID
     */                                   
    ROBDataStruct(const uint32_t);

    // data variables
    uint32_t rob_id{0};                           // rob source id
    uint32_t rob_size{0};                         // size of rob in words
    robmonitor::ROBHistory rob_history{robmonitor::UNCLASSIFIED};        // History of ROB retrieval
    uint32_t rob_status_word{0};                  // last status word in the ROB header


    // Accessor functions
    /** @brief ROB is unclassified */
    bool isUnclassified() const;
    /** @brief ROB was found in ROBDataProviderSvc cache */
    bool isHLTCached() const;
    /** @brief ROB was found in DCM cache */
    bool isDCMCached() const;
    /** @brief ROB was retrieved over network */
    bool isRetrieved() const;
    /** @brief ROB was ignored */
    bool isIgnored() const;
    /** @brief ROB was not enabled */
    bool isUndefined() const;
    /** @brief ROB has no status words set */
    bool isStatusOk() const;

    // Extraction operator
    friend std::ostream& operator<<(std::ostream& os, const ROBDataStruct& rhs);
  };

  /**
   * The structure which is used to monitor the ROB data request in L2
   * It is created for every addROBData call 
   */
  class ROBDataMonitorStruct {
  public:

    /** @brief default constructor */
    ROBDataMonitorStruct() = default;

    /** @brief constructor
        @param  L1 ID
	@param  requestor name 
     */                                   
    ROBDataMonitorStruct(const uint32_t, const std::string&);             

    /** @brief constructor
        @param  L1 ID
	@param  vector of ROB Ids
	@param  requestor name 
     */                                   
    ROBDataMonitorStruct(const uint32_t, const std::vector<uint32_t>&, const std::string&);

    ROBDataMonitorStruct(const ROBDataMonitorStruct&) = default;

    ROBDataMonitorStruct(ROBDataMonitorStruct&&) noexcept = default;

    ROBDataMonitorStruct& operator=(const ROBDataMonitorStruct&) = default;

    ROBDataMonitorStruct& operator=(ROBDataMonitorStruct&&) noexcept = default;

    // data variables
    uint32_t lvl1ID{0};                                                 /// current L1 ID from L1 ROBs
    std::string requestor_name{"UNKNOWN"};                              /// name of requesting algorithm
    std::map<const uint32_t,robmonitor::ROBDataStruct> requested_ROBs;  /// map of ROBs requested

    // Run3 TrigTimeStamp
    uint64_t start_time{};         /// start time of ROB request (microsec since epoch)
    uint64_t end_time{};           /// stop  time of ROB request (microsec since epoch)

    // Accessor functions to ROB history summaries
    /** @brief number of ROBs in structure */
    unsigned allROBs() const;
    /** @brief number of unclassified ROBs in structure */
    unsigned unclassifiedROBs() const;
    /** @brief number of ROBDataProviderSvc cached ROBs in structure */
    unsigned HLTcachedROBs() const;
    /** @brief number of DCM cached ROBs in structure */
    unsigned DCMcachedROBs() const;
    /** @brief number of retrieved ROBs in structure */
    unsigned retrievedROBs() const;
    /** @brief number of ignored ROBs in structure */
    unsigned ignoredROBs() const;
    /** @brief number of undefined ROBs in structure */
    unsigned undefinedROBs() const;
    /** @brief number of ROBs with no status words set in structure */
    unsigned statusOkROBs() const;

    /** @brief elapsed time for ROB request in [ms] */
    float elapsedTime() const;

    // Extraction operator
    friend std::ostream& operator<<(std::ostream& os, const ROBDataMonitorStruct& rhs);
  };

  // Extraction operators
  std::ostream& operator<<(std::ostream& os, const ROBDataStruct& rhs);
  std::ostream& operator<<(std::ostream& os, const ROBDataMonitorStruct& rhs);

} // end namespace robmonitor

typedef DataVector<robmonitor::ROBDataMonitorStruct> ROBDataMonitorCollection;

CLASS_DEF( ROBDataMonitorCollection , 1303465505 , 1 )


#endif /* ROBDATAMONITOR_H */
