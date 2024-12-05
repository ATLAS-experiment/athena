// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef IFPGATRACKSIMTRACKINGTOOL_H
#define IFPGATRACKSIMTRACKINGTOOL_H

#include "GaudiKernel/IAlgTool.h"

class IFPGATrackSimTrackingTool : virtual public IAlgTool {
public:
    virtual ~IFPGATrackSimTrackingTool() = default;
    DeclareInterfaceID(IFPGATrackSimTrackingTool, 1, 0);
};

#endif // IFPGATRACKSIMTRACKINGTOOL_H