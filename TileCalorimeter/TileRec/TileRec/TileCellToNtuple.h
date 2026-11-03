/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileCellToNtuple.h
// Author   : Zhifang
// Created  : Jan. 2003
//
// DESCRIPTION
// 
//    To create Cell Ntuple file from CaloCell container
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
#ifndef TileCellToNtuple_H
#define TileCellToNtuple_H

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"

class TileID;
class TileTBID;

#include <string>

class TileCellToNtuple : public AthAlgorithm {
public:
    //Constructor
    TileCellToNtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileCellToNtuple();                         
    
    //Gaudi Hooks
    StatusCode initialize();    
    StatusCode execute(const EventContext& ctx);
    StatusCode finalize();

private:
    NTuple::Tuple* m_ntuplePtr{nullptr};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h110", "ID of ntuple"};
    Gaudi::Property<std::string> m_ntupleLoc{this, "NTupleLoc", "/FILE1/TileRec", "pathname of ntuple file"};
    Gaudi::Property<bool> m_scinCells{this, "ScinCells", true, "Read scintillator (MBTS) cells from TileCellContainer"};

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
    
    NTuple::Array<int> m_type;
    NTuple::Array<int> m_channel;
    NTuple::Array<int> m_module;

    Gaudi::Property<std::string> m_cellContainer{this, "TileCellContainer", "MBTSContainer", "key value of Cells in TDS"}; // "TileCellContainerKey";

    const TileID*   m_tileID{nullptr};
    const TileTBID*   m_tileTBID{nullptr};
};

#endif
