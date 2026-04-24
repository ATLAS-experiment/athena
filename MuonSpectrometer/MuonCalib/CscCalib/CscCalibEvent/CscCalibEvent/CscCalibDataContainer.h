/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CSCCALIBEVENT_CSCCALIBDATACONTAINER_H
#define CSCCALIBEVENT_CSCCALIBDATACONTAINER_H 

/****************************************************************
 Name: CscCalibDataContainer.h
 Package: MuonSpectrometer/Muoncalib/CscCalib/CscCalibEvent
 Author: Ketevi A. Assamagan, October 2005
 Class to hold collections of CSC pulser calibration data
****************************************************************/

#include <string>

#include "CscCalibEvent/CscCalibDataCollection.h"
#include "MuonIdHelpers/CscIdHelper.h"

#include "AthenaKernel/CLASS_DEF.h"
#include "EventContainers/IdentifiableContainer.h"

class CscCalibDataContainer : public IdentifiableContainer<CscCalibDataCollection> {
 public:  
    
    /** constructor */
    CscCalibDataContainer(int maxHash) ;

    /** destructor */ 
    virtual ~CscCalibDataContainer()=default; 

    /** type definitions */
    typedef CscCalibDataCollection::size_type size_type ; 
    typedef IdentifiableContainer<CscCalibDataCollection> MyBase; 

    /** content size */
    size_type calibData_size() const ; 

    /** IdentifiableContainer is still a DataObject Put CLID here. */ 
    static CLID classID();

};

CLASS_DEF(CscCalibDataContainer,4162,0) 

/** Output stream for printing */
std::ostream& operator<<(std::ostream& lhs, const CscCalibDataContainer& rhs);

#endif



