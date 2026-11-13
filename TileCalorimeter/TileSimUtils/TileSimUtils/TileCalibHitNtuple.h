/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILESIMUTILS_TILECALIBHITNTUPLE_H
#define TILESIMUTILS_TILECALIBHITNTUPLE_H

/********************************************************************
 *
 * NAME:     TileCalibHitNtuple
 * PACKAGE:  offline/TileCalorimeter/TileSimUtils
 *
 * AUTHOR :  A. Solodkov
 * CREATED:  Nov 08, 2012
 *
 * PURPOSE:  Store coordinates of every 
 *           single calibration hit in ntuple
 *
 *  Input: event number, DM cell ID, hit coordiates
 *  Output: ntuple 
 *  Parameters: stream name, ntuple name, max tree size
 *   
 ********************************************************************/

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>

#include <inttypes.h>
#include "Gaudi/Property.h"

/**
 @class TileCalibHitNtuple
 @brief This class creates ntuple with calibration hits info
 */
class TileCalibHitNtuple: public AthAlgTool {

  public:
    TileCalibHitNtuple(const std::string& type, const std::string& name, const IInterface* parent); //!< Constructor

    virtual ~TileCalibHitNtuple(); //!< Destructor

    virtual StatusCode initialize();  //!< intialize method
    virtual StatusCode finalize();    //!< finalize method

    static const InterfaceID& interfaceID(); //!< AlgTool InterfaceID

    void storeHit(int det, int mod, int tow, int samp, int reg, int calc, int evt, float x, float y, float z, float e,
                  const char *volume = 0, int pid = 0, float ekine = 0., float xLocal = 0., float yLocal = 0.,
                  float zLocal = 0., float phiLocal = 0.);

  private:

    // handle to THistSvc
    ServiceHandle<ITHistSvc> m_thistSvc{this, "THistSvc", "THistSvc"};

    // NTuple parameters
    Gaudi::Property<std::string> m_streamName{this, "StreamName", "AANT"};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "TileCalibHit"};
    Gaudi::Property<int64_t> m_treeSize{this, "TreeSize", 16000000000LL};

    // The ntuple
    TTree* m_ntuplePtr{nullptr};

    Gaudi::Property<int> m_level{this, "InfoLevel", 0};

    // ntuple variables
    short m_det{0};
    short m_mod{0};
    short m_tow{0};
    short m_samp{0};
    short m_reg{0};
    short m_calc{0};
    short m_evt{0};
    float m_xPos{0.0};
    float m_yPos{0.0};
    float m_zPos{0.0};
    float m_ener{0.0};
    char m_volume[100]{};
    short m_pid{0};
    float m_ekine{0.0};
    float m_xLocal{0.0};
    float m_yLocal{0.0};
    float m_zLocal{0.0};
    float m_phiLocal{0.0};
    
};

#endif
