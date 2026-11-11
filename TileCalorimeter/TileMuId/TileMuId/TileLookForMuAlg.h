/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileLookForMuAlg.h
// Author   : G Usai 
// Created  : Jun 2003
//
// DESCRIPTION
// 
//  
// Properties (JobOption Parameters):
//
//
// BUGS:
//  
// History:
//      
//   
//****************************************************************************
#ifndef  TILEMUID_TILELOOKFORMUALG_H
#define  TILEMUID_TILELOOKFORMUALG_H 

// Tile includes
#include "TileEvent/TileContainer.h"

// Calo includes
#include "CaloEvent/CaloCellContainer.h"

// Athena includes 
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "Gaudi/Property.h"

// C++ STL includes
#include <vector> 
#include <string>

//Forward declaration
class TileID;

class TileLookForMuAlg: public AthReentrantAlgorithm {
  public:
    // constructor 
    TileLookForMuAlg(const std::string& name, ISvcLocator* pSvcLocator);
    // destructor 
    virtual ~TileLookForMuAlg();

    virtual StatusCode initialize() override;
    virtual StatusCode execute (const EventContext& ctx) const override;
    virtual StatusCode finalize() override;


    //static std::string TileLookForMuAlg_stream;

  private:

    const TileID* m_tileID{nullptr};

    // define a  numeration  scheme for the  cells
    enum N_CELLS_SAMP {
      N_CELLS_D = 13, N_CELLS_BC = 30, N_CELLS_A = 30
    };
    enum N_OF_MODULES {
      N_MODULES = 64
    };
    double m_etaD[N_CELLS_D]{};
    double m_etaBC[N_CELLS_BC]{};
    double m_etaA[N_CELLS_A]{};
    const int m_nMuMax{30};
    // and patterns search from D to BC and  from BC to A layers   
    Gaudi::Property<std::vector<int>> m_fromDtoBC{this, "From3to2", {}};
    Gaudi::Property<std::vector<int>> m_fromBCtoA{this, "From2to1", {}};
    //   std::map<double,unsigned int> eta2map
    // lower thresholds for the 3 sample
    Gaudi::Property<double> m_loThrA{this, "LowerTresh0MeV", 80.0};
    Gaudi::Property<double> m_loThrBC{this, "LowerTresh1MeV", 80.0};
    Gaudi::Property<double> m_loThrD{this, "LowerTresh2MeV", 80.0};
    Gaudi::Property<double> m_loThrITC{this, "LowerTreshScinMeV", 160.0};
    // std::vector<double>  hiThrD; 
    Gaudi::Property<std::vector<double>> m_hiThrD{this, "UpperTresh2MeV", {}};
    Gaudi::Property<std::vector<double>> m_hiThrBC{this, "UpperTresh1MeV", {}};
    Gaudi::Property<std::vector<double>> m_hiThrA{this, "UpperTresh0MeV", {}};

    SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this,"CellsNames",
                                                            "AllCalo",
                                                            "Input Calo cell container key"};

    SG::WriteHandleKey<TileMuContainer> m_muContainerKey{this,"TileMuTagsOutputName",
                                                         "TileMuObj",
                                                         "Output Tile mu container key"};


};
#endif // TILEMUID_TILELOOKFORMUALG_H
