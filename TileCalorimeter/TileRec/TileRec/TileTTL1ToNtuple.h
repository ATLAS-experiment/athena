/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileTTL1ToNtuple.h
// Author   : Sanya.Solodkov@cern.ch
// Created  : Nov 2005
//
// DESCRIPTION
// 
//    To create Ntuple with all TileTTL1 file from TileTTL1Container
//
// Properties (JobOption Parameters):
//
//    TileTTL1Container           string   key value of TileTTL1 in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    string   ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TileTTL1ToNtuple_H
#define TileTTL1ToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class CaloLVL1_ID;
class TileTBID;

#include <string>

class TileTTL1ToNtuple : public AthAlgorithm {
public:
    //Constructor
    TileTTL1ToNtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileTTL1ToNtuple();                         
    
    //Gaudi Hooks
    StatusCode initialize();    
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h130", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "pathname of ntuple file"};
    Gaudi::Property<bool> m_commitNtuple{this, "CommitNtuple", true, "Commit ntuple in execute"};
    Gaudi::Property<int> m_maxLength{this, "MaxLength", 2048, "Maximum number of TTL1s in ntuple"};
    Gaudi::Property<int> m_nSamples{this, "NSamples", 7, "Number of samples"};

    NTuple::Item<int> m_nchan;

    NTuple::Array<int> m_side;
    NTuple::Array<int> m_eta;
    NTuple::Array<int> m_phi;
    
    NTuple::Matrix<float> m_samples;

    Gaudi::Property<std::string> m_ttl1Container{this, "TileTTL1Container", "TileTTL1Cnt", "key value of TileTTL1 in TDS"};

    const CaloLVL1_ID* m_TT_ID{nullptr};
    const TileTBID*    m_tileTBID{nullptr};
};

#endif
