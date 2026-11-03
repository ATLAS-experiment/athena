/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileHitVecToNtuple.h
// Author   : Mikhail Makouski
// Created  : July. 2005
//
// DESCRIPTION
//
//    To create Hit Ntuple file from TileHitVector
//
// Properties (JobOption Parameters):
//
//    TileHitVector               string   key value of HitVector
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    string   ID of ntuple
//
// BUGS:
//
// History:
//
//
//****************************************************************************

#ifndef TileHitVecToNtuple_H
#define TileHitVecToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class TileID;

#include <string>

class TileHitVecToNtuple : public AthAlgorithm {
public:
    // Constructor
    TileHitVecToNtuple(const std::string& name, ISvcLocator* pSvcLocator);
  
    //Destructor
    virtual ~TileHitVecToNtuple();
  
    //Gaudi Hooks
    StatusCode initialize();
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();
  
private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h32", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "Pathname of ntuple file"};
    Gaudi::Property<bool> m_commitNtuple{this, "CommitNtuple", true, "Commit ntuple in execute"};
    Gaudi::Property<int>  m_maxLength{this, "MaxLength", 4999, "Maximum number of hits in ntuple"};
  
    NTuple::Item<int> m_nchan;
    NTuple::Item<double> m_tolE;
    NTuple::Item<double> m_tolE0;
    NTuple::Item<double> m_tolE1;
    NTuple::Item<double> m_tolE2;
    NTuple::Item<double> m_tolE3;
    NTuple::Item<double> m_tolE4;
  
    NTuple::Array<float> m_energy;
    NTuple::Array<float> m_time;
    NTuple::Array<int> m_pmt ;
  
    NTuple::Array<int> m_detector;
    NTuple::Array<int> m_side;
    NTuple::Array<int> m_sample;
    NTuple::Array<int> m_eta;
    NTuple::Array<int> m_phi;
  
    Gaudi::Property<std::string> m_hitVector{this, "TileHitVector", "TileHitVec", "Key value of HitVector"};
    
    const TileID*   m_tileID{nullptr};
};

#endif
