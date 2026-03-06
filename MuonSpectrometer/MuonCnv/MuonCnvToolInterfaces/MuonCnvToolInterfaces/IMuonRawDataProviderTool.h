/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCNVTOOLINTERFACES_IMUONRAWDATAPROVIDERTOOL_H
#define MUONCNVTOOLINTERFACES_IMUONRAWDATAPROVIDERTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "ByteStreamData/RawEvent.h"
#include "Identifier/IdentifierHash.h"
#include "GaudiKernel/EventContext.h"
#include <vector>


namespace Muon
{

/**Interface defining the tools used to convert Muon BS to MuonRDOs.
*/
class IMuonRawDataProviderTool : virtual public IAlgTool
{
public:
    DeclareInterfaceID( IMuonRawDataProviderTool, 1, 0 );

public:
    /** Decoding method. - current methods: let's keep them! */
    using ROBFragmentList = std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>;
    /** Event Context functions **/
    virtual StatusCode convert(const EventContext&) const  = 0;
    virtual StatusCode convert(const std::vector<IdentifierHash>&, const EventContext&) const  = 0;
    virtual StatusCode convert(const std::vector<uint32_t>&, const EventContext&) const = 0;
};
}

#endif // !MUONCNVTOOLINTERFACES_IMUONRDOTOPREPDATATOOL_H
