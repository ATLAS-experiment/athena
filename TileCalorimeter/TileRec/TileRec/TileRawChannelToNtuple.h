/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileRawChannelToNtuple.h
// Author   : Zhifang
// Created  : Nov. 2002
//
// DESCRIPTION
// 
//    To create RawChannel Ntuple file from RawChannel container
//
// Properties (JobOption Parameters):
//
//    TileRawChannelContainer     string   key value of RawChannels in TDS 
//    NtupleLoc                   string   pathname of ntuple file
//    NtupleID                    int      ID of ntuple
//
// BUGS:
//  
// History:
//  
//  
//****************************************************************************
#ifndef TileRawChannelToNtuple_H
#define TileRawChannelToNtuple_H

#include "TileConditions/TileCablingSvc.h"

#include "GaudiKernel/NTuple.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "StoreGate/ReadHandleKey.h"

class TileID;
class TileHWID;

#include <string>

class TileRawChannelToNtuple : public AthAlgorithm {
public:

    using AthAlgorithm::AthAlgorithm;
    virtual ~TileRawChannelToNtuple() = default;

    //Gaudi Hooks
    StatusCode initialize() override;
    StatusCode execute() override;
    StatusCode finalize() override;

private:

    Gaudi::Property<std::string> m_ntupleLoc{this,
       "NTupleLoc", "/FILE1/TileRec", "Tile raw channel ntuple location"};

    Gaudi::Property<std::string> m_ntupleID{this,
       "NTupleID", "h70", "Tile raw channel ntuple ID"};

    SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
       "TileRawChannelContainer", "TileRawChannelCnt", "Tile raw channel container name."};

    /**
     * @brief Name of Tile cabling service
     */
    ServiceHandle<TileCablingSvc> m_cablingSvc{ this,
       "TileCablingSvc", "TileCablingSvc", "The Tile cabling service"};

    NTuple::Tuple* m_ntuplePtr{nullptr};

    NTuple::Item<int> m_nchan;
    NTuple::Item<double> m_tolE;

    NTuple::Array<float> m_energy;
    NTuple::Array<float> m_time;
    NTuple::Array<float> m_quality;

    NTuple::Array<int> m_detector;
    NTuple::Array<int> m_side;
    NTuple::Array<int> m_sample;
    NTuple::Array<int> m_eta;
    NTuple::Array<int> m_phi;
    NTuple::Array<int> m_pmt;
    NTuple::Array<int> m_channel;
    NTuple::Array<int> m_gain;
    
    const TileID*   m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};
};

#endif
