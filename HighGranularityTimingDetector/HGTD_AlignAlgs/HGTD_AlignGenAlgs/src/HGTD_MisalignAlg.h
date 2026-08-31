/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_ALIGNALGS_HGTD_MISALIGNALG_H
#define HGTD_ALIGNALGS_HGTD_MISALIGNALG_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "Gaudi/Property.h"

#include <vector>
#include <string>
#include <fstream>
#include "TTree.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"
#include "HGTD_Identifier/HGTD_ID.h"

#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ITHistSvc.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "GeoModelKernel/GeoVAlignmentStore.h"

#include "HGTD_AlignGenTools/IHGTD_AlignDBTool.h"
#include "GaudiKernel/ToolHandle.h"

class GeoAlignableTransform;

class HGTD_MisalignAlg : public AthAlgorithm {

public:
    using AthAlgorithm::AthAlgorithm;
    
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) override;
    virtual StatusCode finalize() override;

private:

    Gaudi::Property<int> m_mode{this, "MisalignMode", 1};
    Gaudi::Property<double> m_shiftX{this, "ShiftX", 0.05};
    Gaudi::Property<double> m_shiftY{this, "ShiftY", 0.0};
    Gaudi::Property<double> m_shiftZ{this, "ShiftZ", 0.0};

    Gaudi::Property<double> m_sigmaX{this, "SigmaX", 0.01};
    Gaudi::Property<double> m_sigmaY{this, "SigmaY", 0.01};
    Gaudi::Property<double> m_sigmaZ{this, "SigmaZ", 0.01};


    Gaudi::Property<bool> m_applyTranslation{this, "ApplyTranslation", true};
    Gaudi::Property<bool> m_doDebugPrint{this, "DoDebugPrint", true};

    Gaudi::Property<bool> m_applyRotation{this, "ApplyRotation", false};
    Gaudi::Property<int>  m_maxModules{this, "MaxModules", -1};    
    
    ServiceHandle<IRndmGenSvc> m_rndmSvc{
        this, "RndmGenSvc", "RndmGenSvc", "Random number service"
    };

    Gaudi::Property<std::string> m_randomStream{
        this,
        "RandomStream",
        "HGTDMisalignment",
        "Random number stream name"
    };

    std::vector<GeoAlignableTransform*> m_alignables;

    Gaudi::Property<std::string> m_outputFile{
        this,
        "OutputFile",
        "hgtd_misalignment.txt",
        "Output file name"
    };

    const HGTD_DetectorManager* m_hgtdManager{nullptr};
    
    Gaudi::Property<bool> m_applyToGeometry{
        this,
        "ApplyToGeometry",
        false,
        "Apply misalignment deltas to HGTD alignable transforms"
    };

    Gaudi::Property<int> m_testHash{
        this,
        "TestHash",
        -1,
        "If >=0, apply geometry delta only to this hash"
    };

    Gaudi::Property<bool> m_createFreshDB{
        this,
        "CreateFreshDB",
        true,
        "Create a new HGTD alignment database"
    };

    Gaudi::Property<std::string> m_sqliteTag{
        this,
        "SQLiteTag",
        "HGTD_AlignTag",
        "SQLite alignment tag"
    };

    Gaudi::Property<bool> m_writeToDB{
        this,
        "WriteToDB",
        false,
        "Write misalignment to HGTD alignment database"
    };

    ToolHandle<IHGTD_AlignDBTool> m_alignDBTool{
        this,
        "AlignDBTool",
        "HGTD_AlignDBTool",
        "HGTD alignment database tool"
    };

    const HGTD_ID* m_hgtdIdHelper{nullptr};

    //------------------------------------------------------------
    // Validation ROOT tree
    // Stores one entry per HGTD detector element containing the
    // nominal position together with the applied misalignment.
    //------------------------------------------------------------

    TTree* m_tree{nullptr};

    // Alignment parameters
    float m_localDx{0.f};
    float m_localDy{0.f};
    float m_localDz{0.f};

    // Nominal module center
    float m_centerX{0.f};
    float m_centerY{0.f};
    float m_centerZ{0.f};

    // Misaligned module center
    float m_shiftedX{0.f};
    float m_shiftedY{0.f};
    float m_shiftedZ{0.f};

    //  Detector identification
    unsigned int m_hash{0};
    int m_endcap{0};
    int m_layer{0};
    int m_phiModule{0};
    int m_etaModule{0};

    std::ofstream m_outfile;
    bool m_firstEvent{true};
    unsigned int m_nEvents{0};
    
    StatusCode GenerateMisalignment();


};

#endif