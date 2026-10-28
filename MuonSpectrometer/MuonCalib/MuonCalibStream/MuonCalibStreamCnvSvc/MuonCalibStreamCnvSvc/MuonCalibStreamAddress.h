/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/
//====================================================================
//	MuonCalibStreamAddress.h
//====================================================================
#ifndef MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMADDRESS_H
#define MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMADDRESS_H

// Framework include files
#include "GaudiKernel/GenericAddress.h"

class MuonCalibStreamAddress : public GenericAddress {
public:
    // Standard Destructor
    virtual ~MuonCalibStreamAddress() = default;

    // Standard Constructor
    MuonCalibStreamAddress(const CLID &clid, const std::string &fname, const std::string &cname, int p1 = 0, int p2 = 0);

    MuonCalibStreamAddress(const CLID &clid);

    // special storageType for MuonCalibStream 0x63
    static constexpr long storageType() { return 0x63; }
};
#endif  // MUONCALIBSTREAMCNVSVCBASE_MUONCALIBSTREAMADDRESS_H
