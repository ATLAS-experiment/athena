/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file IIOVRegistrationSvc.h 
 * 
 * @brief This is an interface to a tool used to register conditions
 * objects in the Interval of Validity (IOV) database
 * 
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * @author Antoine Pérus <perus@lal.in2p3.fr>
 * 
 */

#ifndef REGISTRATIONSERVICES_IIOVREGISTRATIONSVC_H
#define REGISTRATIONSERVICES_IIOVREGISTRATIONSVC_H


// Gaudi
#include "GaudiKernel/IAlgTool.h"
#include <stdint.h>
#include <string_view>

class IOVTime;


/** 
 ** @class IIOVRegistrationSvc
 ** 
 ** @brief This is an interface to a service used to register conditions
 ** objects in the Interval of Validity (IOV) database
 **  
 **    Properties:
 **
 **    - RecreateFolder:<pre>  flag to force the recreation of the requested folder</pre>
 **    - BeginRun:<pre>        Begin run number   (default: IOVTime::MINRUN)</pre>	 
 **    - EndRun:<pre>          End run number     (default: IOVTime::MAXRUN)</pre>	 
 **    - BeginLB:<pre>      Begin Lumiblock number (default: IOVTime::MINEVENT)</pre>	 
 **    - EndLB:<pre>        End Lumiblock number   (default: IOVTime::MAXEVENT)</pre>	 
 **    - BeginTime:<pre>       Begin time	  (default: IOVTime::MINTIMESTAMP)</pre>
 **    - EndTime:<pre>         End time	          (default: IOVTime::MAXTIMESTAMP)</pre>
 **    - IOVDbTag:<pre>        the tag to be used</pre>
 **/



class IIOVRegistrationSvc : virtual public IInterface
{

public:    
  
    /// Declare interface ID
    DeclareInterfaceID(IIOVRegistrationSvc, 1 , 0);

    /// Register IOV DB for an object given its typeName - run/LB numbers
    ///   interval or times interval  and tag are taken from JobOptions
    ///   Choice between run/LB and timestamp given in JobOptions
    virtual StatusCode registerIOV(std::string_view typeName) const = 0;
    
    /// Register IOV DB for an object given its typeName - run/LB numbers
    ///   interval or times interval taken from JobOptions
    ///   tag is specified
    ///   Choice between run/LB and timestamp given in JobOptions
    virtual StatusCode registerIOV( std::string_view typeName, std::string_view tag ) const = 0;
    
    /// Register IOV DB for an object given its typeName and its key
    ///   run/LB numbers interval or times interval  and tag are taken
    ///   from JobOptions
    ///   Choice between run/LB and timestamp given in JobOptions
    virtual StatusCode registerIOV( std::string_view typeName, std::string_view key,
				    std::string_view tag ) const = 0;
    
    /// Register IOV DB for an object given its typeName, tag and run/LB
    /// numbers interval
    virtual StatusCode registerIOV( std::string_view typeName,
				    std::string_view tag,
				    unsigned int beginRun, 
				    unsigned int endRun, 
				    unsigned int beginLB, 
				    unsigned int endLB ) const = 0;
    
    /// Register IOV DB for an object given its typeName, tag and
    /// times interval
    virtual StatusCode registerIOV( std::string_view typeName, 
				    std::string_view tag,
				    uint64_t beginTime, 
				    uint64_t endTime ) const = 0;

    /// Register IOV DB for an object given its typeName, key, tag and run/LB
    /// numbers interval
    virtual StatusCode registerIOV( std::string_view typeName,
				    std::string_view key,
				    std::string_view tag,
				    unsigned int beginRun, 
				    unsigned int endRun, 
				    unsigned int beginLB, 
				    unsigned int endLB ) const = 0;
    
    /// Register IOV DB for an object given its typeName, key, tag and
    /// times interval
    virtual StatusCode registerIOV( std::string_view typeName,
				    std::string_view key,
				    std::string_view tag,
				    uint64_t beginTime, 
				    uint64_t endTime ) const = 0;

    /// Register IOV DB for an object given its typeName, key, folder, tag 
    ///  and run/LB numbers interval
    virtual StatusCode registerIOV( std::string_view typeName,
				    std::string_view key,
				    std::string_view folder,
				    std::string_view tag,
				    unsigned int beginRun, 
				    unsigned int endRun, 
				    unsigned int beginLB, 
				    unsigned int endLB ) const = 0;

    /// Register IOV DB for an object given its typeName, key, folder, tag and
    /// times interval
    virtual StatusCode registerIOV( std::string_view typeName,
				    std::string_view key,
				    std::string_view folder,
				    std::string_view tag,
				    uint64_t beginTime, 
				    uint64_t endTime ) const = 0;
};

#endif // REGISTRATIONSERVICES_IIOVREGISTRATIONSVC_H
