/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileTBHitToNtuple.h
// Author   : Anna
// Created  : June 2004
//
// DESCRIPTION
// 
//    To create Hit Ntuple file from TileHitContainer
//
// Properties (JobOption Parameters):
//
//    TileHitContainer            string   key value of Hits in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    int      ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TileTBHitToNtuple_H
#define TileTBHitToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class TileTBID;

#include <string>

class TileTBHitToNtuple : public AthAlgorithm {
public:
    //Constructor
    TileTBHitToNtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileTBHitToNtuple();                         
    
    //Gaudi Hooks
    StatusCode initialize();    
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h31", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "Pathname of ntuple file"};

    NTuple::Item<int> m_nchan;
    NTuple::Item<double> m_tolE;

    NTuple::Array<float> m_energy;
    NTuple::Array<float> m_time;

    NTuple::Array<int> m_type;
    NTuple::Array<int> m_channel;
    NTuple::Array<int> m_module;
    
    Gaudi::Property<std::string> m_hitContainer{this, "TileHitContainer", "TileTBHits", "Name of hit vector for ancillary detectors"};

    const TileTBID*   m_tileTBID{nullptr};
};

#endif
