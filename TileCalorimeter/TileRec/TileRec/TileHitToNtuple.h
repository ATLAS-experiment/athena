/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileHitToNtuple.h
// Author   : Gia
// Created  : March. 2003
//
// DESCRIPTION
// 
//    To create Hit Ntuple file from TileHitContainer
//
// Properties (JobOption Parameters):
//
//    TileHitContainer            string   key value of Hits in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    string   ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TileHitToNtuple_H
#define TileHitToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class TileID;

#include <string>

class TileHitToNtuple : public AthAlgorithm {
public:
    //Constructor
    TileHitToNtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileHitToNtuple();                         
    
    //Gaudi Hooks
    StatusCode initialize();    
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h30", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "Pathname of ntuple file"};
    Gaudi::Property<bool> m_commitNtuple{this, "CommitNtuple", true, "Commit ntuple in execute"};
    Gaudi::Property<int>  m_maxLength{this, "MaxLength", 4999, "Maximum number of hits in ntuple"};

    NTuple::Item<int> m_nchan;
    NTuple::Item<double> m_tolE;

    NTuple::Array<float> m_energy;
    NTuple::Array<float> m_time;
    NTuple::Array<int> m_pmt ; 

    NTuple::Array<int> m_detector;
    NTuple::Array<int> m_side;
    NTuple::Array<int> m_sample;
    NTuple::Array<int> m_eta;
    NTuple::Array<int> m_phi;
    
    Gaudi::Property<std::string> m_hitContainer{this, "TileHitContainer", "TileHitCnt", "Key value of Hits in TDS"};

    const TileID*   m_tileID{nullptr};
};

#endif
