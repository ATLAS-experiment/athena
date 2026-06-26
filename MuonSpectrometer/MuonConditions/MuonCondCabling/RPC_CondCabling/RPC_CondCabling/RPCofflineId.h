/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RPCOFFLINEID_H
#define RPCOFFLINEID_H

struct RPCofflineId {
    std::string stationName;
    int stationEta = 0;
    int stationPhi = 0;
    int doubletR = 0;
    int doubletZ = 0;
    int doubletPhi = 0;
    int gasGap = 0;
    int measuresPhi = 0;
    int strip = 0;
    void init() {
        stationEta = -99;
        stationPhi = -99;
        doubletR = -99;
        doubletZ = -99;
        doubletPhi = -99;
        gasGap = -99;
        measuresPhi = -99;
        strip = -99;
    }
};

#endif
