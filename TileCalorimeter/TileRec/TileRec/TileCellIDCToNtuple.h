/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileCellIDCToNtuple.h
// Author   : Zhifang
// Created  : Jan. 2003
//
// DESCRIPTION
// 
//    To create Cell Ntuple file from CellIDC container
//
// Properties (JobOption Parameters):
//
//    TileCellContainer           string   key value of Cells in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    int      ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TileCellIDCToNtuple_H
#define TileCellIDCToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class TileID;

#include <string>

class TileCellIDCToNtuple : public AthAlgorithm {
public:
    //Constructor
    TileCellIDCToNtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileCellIDCToNtuple();                         
    
    //Gaudi Hooks
    StatusCode initialize();    
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h90", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "pathname of ntuple file"};

    NTuple::Item<int> m_nchan;
    NTuple::Item<double> m_tolE;

    NTuple::Array<float> m_energy;
    NTuple::Array<float> m_enediff;
    NTuple::Array<float> m_time;
    NTuple::Array<float> m_quality;

    NTuple::Array<int> m_detector;
    NTuple::Array<int> m_side;
    NTuple::Array<int> m_sample;
    NTuple::Array<int> m_eta;
    NTuple::Array<int> m_phi;
    
    Gaudi::Property<std::string> m_cellContainer{this, "TileCellIDC", "TileCellIDC", "key value of Cells in TDS"};
    const TileID*   m_tileID{nullptr};
};

#endif
