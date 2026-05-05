/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaPoolTestDataReader_h
#define AthenaPoolTestDataReader_h

/**
 * @file AthenaPoolTestDataReader.h
 *
 * @brief Test Algorithm for POOL I/O tests, reads AthenaPoolData
 * objects from the transient store
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 *
 */

/**
 * @class AthenaPoolTestDataReader
 *
 * @brief Test Algorithm POOL I/O tests, reads AthenaPoolData objects
 * from the transient store
 *
 */

// INCLUDE HEADER FILES:

#include "AthenaBaseComps/AthAlgorithm.h"

class AthenaPoolTestDataReader : public AthAlgorithm
{

 public:

    /// Algorithm constructor
    AthenaPoolTestDataReader(const std::string& name, ISvcLocator* pSvcLocator);

    /// Algorithm destructor
    ~AthenaPoolTestDataReader();
  
    /// Algorithm initialize at begin of job
    virtual StatusCode initialize();

    /// Algorithm execute once per event
    virtual StatusCode execute(); 

    /// Algorithm finalize at end of job
    virtual StatusCode finalize();

};
#endif
