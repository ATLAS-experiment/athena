/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaPoolTestDataWriter_h
#define AthenaPoolTestDataWriter_h

/**
 * @file AthenaPoolTestDataWriter.h
 *
 * @brief Test Algorithm for POOL I/O tests, writes AthenaPoolData
 * objects to the transient store
 *
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 *
 */

/**
 * @class AthenaPoolTestDataWriter
 *
 * @brief Test Algorithm POOL I/O tests, writes AthenaPoolData objects
 * to the transient store
 *
 */

// INCLUDE HEADER FILES:

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

// Contains thread-unsafe old-style code modifying EventInfo in place.
class ATLAS_NOT_THREAD_SAFE AthenaPoolTestDataWriter : public AthAlgorithm
{

public:

    /// Algorithm constructor
    AthenaPoolTestDataWriter(const std::string& name, ISvcLocator* pSvcLocator);

    /// Algorithm destructor
    ~AthenaPoolTestDataWriter();
  
    /// Algorithm execute once per event
    virtual StatusCode execute() override;

private:

    /// Create only part of the collections
    Gaudi::Property<bool> m_partialCreate{this, "PartialCreate", false};

    /// For partial create read second half of collections
    Gaudi::Property<bool> m_readOtherHalf{this, "ReadOtherHalf", false};

    /// For partial create read first half of collections
    Gaudi::Property<bool> m_readFirstHalf{this, "ReadFirstHalf", false};

};
#endif
