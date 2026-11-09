/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILEINFOLOADER_H
#define TILECONDITIONS_TILEINFOLOADER_H
//****************************************************************************
// Filename : TileInfoLoader.h
// Author   : Ed Frank
// Created  : May 2002
//
// DESCRIPTION
// 
//    TileConditions/TileInfo.h is the Tile interface to the transient
// detector store.  Rather than having every item accessed separately from
// the TDS, people will grab a TileInfo from the TDS and make calls on it.
// Those calls will defer back to the TDS so that time validity is done
// correctly.  We think this may be simpler than getting everything
// separately.  We expect, in the future, to break the TileInfo into pieces,
// but for now, it's all in one.
//
// Until a conditions database exists, we just make a TileInfo from parameters
// in a fplain file.  TileInfoLoader exists just to produce a tileInfo at
// the start of the job until the conditions DB comes along.
//
// Properties (JobOption Parameters):
//
//
// History:
//   07Feb01  efrank  Creation, based upon LArTowerBuilder.h
//  
// BUGS:
//  
//****************************************************************************

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "Gaudi/Property.h"

// Athena includes
#include "AthenaBaseComps/AthService.h"

#include <string>
#include <vector>

class TilePulseShapes;
class TileWienerFilterWeights;
class StoreGateSvc;
class TileCablingSvc;
template<class TYPE> class SvcFactory;
class TileInfo;

static const InterfaceID IID_TileInfoLoader("TileInfoLoader", 1, 0);

class TileInfoLoader: public AthService {

    friend class SvcFactory<TileInfoLoader> ;

  public:

    TileInfoLoader(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TileInfoLoader();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    static const InterfaceID& interfaceID() { return IID_TileInfoLoader; }

private:
    //** Loading stuff from files */
    StatusCode buildDigitsShapesHiLo (TileInfo& info);
    StatusCode buildTTL1Shapes(TileInfo& info,
                               const std::string& ShapeFile,
                               int &NBins, int &Time0Bin, int &BinsPerX
                               , std::vector<double> &FullShape, std::vector<double> &Shape);
    void buildCovMatrix (TileInfo& info);

    Gaudi::Property<std::string> m_tileInfoName{this,
        "TileInfo", "TileInfo", "Name of TileInfo object in Detector Store"};

    //=== services
    ServiceHandle<StoreGateSvc> m_detStore;

    /** Pointer to TilePulseShapes object*/
    TilePulseShapes* m_pulsevar;

    /** Properties to forward to TileInfo. */
    //=== expected number of ADC samples in digitization
    Gaudi::Property<int> m_nSamples{this, "NSamples", 7, "Number of 25ns time slices in TileDigits"};
    Gaudi::Property<int> m_iTrigSample{this, "TrigSample", 3, "sample index of t=0 in TileDigits"};
    //=== Noise and conversion factors used in digitization
    Gaudi::Property<bool> m_tileNoise{this, "TileNoise", true, "true => add noise to TileDigits and TileRawChannels (used in MC generation)"};
    Gaudi::Property<bool> m_tileCoherNoise{this, "TileCoherNoise", false, "true => add coherent noise to TileDigits and TileRawChannels (used in MC generation)"};
    Gaudi::Property<bool> m_tileZeroSuppress{this, "TileZeroSuppress", false, "true => applies threshold cuts in HitToRC, HitToDigits, RCToCells"};
    Gaudi::Property<int> m_noiseScaleIndex{this, "NoiseScaleIndex", 1, "allowed values 0-3 to choose proper ADC->OF noise scale factor for different OF methods"};
    Gaudi::Property<double> m_thresholdRawChannel{this, "ThresholdRawChannel", -3.0, "threshold for TileRawChannel [adc] (high gain only, negative -> cut on abs value)"};
    Gaudi::Property<double> m_thresholdDigits{this, "ThresholdDigits", -2.0, "threshold for TileDigits [adc] (high gain only, negative -> cut on abs value)"};
    // values for TileCal Level1 Trigger Towers
    Gaudi::Property<double> m_ttL1Calib{this, "TTL1Calib", 4.1, "converts pCb to mV for TTL1"};
    Gaudi::Property<double> m_ttL1NoiseSigma{this, "TTL1NoiseSigma", 2.5, "sigma of noise in TTL1 (mV)"};
    Gaudi::Property<double> m_ttL1Thresh{this, "TTL1Thresh", 5.0, "threshold cut for TTL1 signals (mV)"};
    Gaudi::Property<double> m_ttL1Ped{this, "TTL1Ped", 0.0, "pedestal for TTL1 signals (mV)"};
    Gaudi::Property<double> m_ttL1Max{this, "TTL1Max", 1850.0, "max value for TTL1 signals (mV)"};
    // values for TileCal Muon Receiver Boards (!!! TO UPDATE !!! DUMMY Values) ~1mV = 1 ADC count
    Gaudi::Property<double> m_MuRcvCalib{this, "MuRcvCalib", 14.20, "converts pCb to ADC counts for MuRcv"};
    Gaudi::Property<double> m_MuRcvNoiseSigma{this, "MuRcvNoiseSigma", 2, "sigma of noise in MuRcv (ADC counts)"};
    Gaudi::Property<double> m_MuRcvThresh{this, "MuRcvThresh", 5, "threshold cut for MuRcv signals (ADC count)"};
    Gaudi::Property<double> m_MuRcvPed{this, "MuRcvPed", 11.73, "pedestal for MuRcv signals (ADC count)"};
    Gaudi::Property<double> m_MuRcvMax{this, "MuRcvMax", 255, "max value for MuRcv signals (ADC count)"};
    // values for Level1 Muon output (not used currently)
    Gaudi::Property<double> m_muL1Calib{this, "MuL1Calib", 204.2, "converts pCb to mV for Mu"};
    Gaudi::Property<double> m_muL1NoiseSigma{this, "MuL1NoiseSigma", 15.6, "sigma of noise in Mu (mV)"};
    Gaudi::Property<double> m_muL1Thresh{this, "MuL1Thresh", 5.0, "threshold cut for Mu signals (mV)"};
    Gaudi::Property<double> m_muL1Ped{this, "MuL1Ped", 0.0, "pedestal for Mu signals (mV)"};
    Gaudi::Property<double> m_muL1Max{this, "MuL1Max", 1850.0, "max value for Mu signals (mV)"};
    // new values for high gain MBTS output connected to normal trigger tower
    Gaudi::Property<double> m_mbtsL1Calib{this, "MBTSL1Calib", 222.0, "converts pCb to mV for MBTS"};
    Gaudi::Property<double> m_mbtsL1NoiseSigma{this, "MBTSL1NoiseSigma", 0.6, "sigma of noise in MBTS (mV)"};
    Gaudi::Property<double> m_mbtsL1Thresh{this, "MBTSL1Thresh", 5.0, "threshold cut for MBTS signals (mV)"};
    Gaudi::Property<double> m_mbtsL1Ped{this, "MBTSL1Ped", 0.0, "pedestal for MBTS signals (mV)"};
    Gaudi::Property<double> m_mbtsL1Max{this, "MBTSL1Max", 1850.0, "max value for MBTS signals (mV)"};
    // values for custom TileCal cosmic trigger boards
    Gaudi::Property<double> m_ttL1CosmicsGain{this, "TTL1CosmicsGain", 5.0, "gain of the Cosmics board"};
    Gaudi::Property<double> m_ttL1CosmicsThresh{this, "TTL1CosmicsThresh", 5.0, "threshold in the Cosmics board"};
    //=== Digitization
    Gaudi::Property<int> m_ADCmax{this, "ADCmax", 1023, "Maximum ADC value"};
    Gaudi::Property<int> m_ADCmaskValue{this, "ADCmaskValue", 2047, "ADC mask value"};

    /** Pointer to Wiener Filtering weights*/
    TileWienerFilterWeights* m_WFWeights;

    /** Filenames of input files */
    //=== Digits pulse shapes
    Gaudi::Property<std::string> m_digitsShapeFileHi{this, "DigitsShapeFileHi", "pulsehi_physics.dat", "Filename for high gain pulse shape"};
    Gaudi::Property<std::string> m_digitsShapeFileLo{this, "DigitsShapeFileLo", "pulselo_physics.dat", "Filename for low gain pulse shape"};
    //=== Trigger Tower Pulse shapes
    Gaudi::Property<std::string> m_TTL1ShapeFile{this, "filename_TTL1ShapeFile", "pulse_adder_tower_physics.dat", "Filename for trigger tower pulse shape"};
    Gaudi::Property<std::string> m_MuRcvShapeFile{this, "filename_MuRcvShapeFile", "pulse_adder_muonRcv_physics.dat", "Filename for Muon Receiver pulse shape"}; // using the sames as for TTL1ShapeFile !!! TO UPDATE !!!
    Gaudi::Property<std::string> m_MuL1ShapeFile{this, "filename_MuL1ShapeFile", "pulse_adder_muon_physics.dat", "Filename for muon level1 trigger pulse shape"};
    //=== Covariance matrices for noise simulation
    Gaudi::Property<std::string> m_DecoCovaFilePrefix{this, "filename_DecoCovaFilePrefix", "DecoCovaMatrix", "Prefix of the files with decomposed covariance matrices for coherent noise simulation"};

    /** Reference pulse shape filenames, forwarded to TilePulseShapes in initialize() */
    //=== CIS Pulse shapes and derivatives
    Gaudi::Property<std::string> m_filenameLoCIS{this, "filename_lo_cis", "pulselo_cis_100.dat", "Low gain CIS pulseshape 100pF"};
    Gaudi::Property<std::string> m_filenameLoCISDer{this, "filename_lo_cis_der", "dpulselo_cis_100.dat", "Low gain CIS pulseshape derivative 100pF"};
    Gaudi::Property<std::string> m_filenameSLoCIS{this, "filename_slo_cis", "pulselo.0230055.12.cap.5.2.dat", "Low gain CIS pulseshape 5pF"};
    Gaudi::Property<std::string> m_filenameSLoCISDer{this, "filename_slo_cis_der", "dpulselo.0230055.12.cap.5.2.dat", "Low gain CIS pulseshape derivative pF"};
    Gaudi::Property<std::string> m_filenameHiCIS{this, "filename_hi_cis", "pulsehi_cis_100.dat", "High gain CIS pulseshape 100pF"};
    Gaudi::Property<std::string> m_filenameHiCISDer{this, "filename_hi_cis_der", "dpulsehi_cis_100.dat", "High gain CIS pulseshape derivative 100pF"};
    Gaudi::Property<std::string> m_filenameSHiCIS{this, "filename_shi_cis", "pulsehi.0230055.12.cap.5.2.dat", "High gain CIS pulseshape 5pF"};
    Gaudi::Property<std::string> m_filenameSHiCISDer{this, "filename_shi_cis_der", "dpulsehi.0230055.12.cap.5.2.dat", "High gain CIS pulseshape derivative 5pF"};
    //=== Physics Pulse shapes and derivatives
    Gaudi::Property<std::string> m_filenameLoPhys{this, "filename_lo_phys", "pulselo_physics.dat", "Low gain physics pulseshape"};
    Gaudi::Property<std::string> m_filenameHiPhys{this, "filename_hi_phys", "pulsehi_physics.dat", "High gain physics pulseshape"};
    Gaudi::Property<std::string> m_filenameLoPhysDer{this, "filename_lo_phys_der", "dpulselo_physics.dat", "Low gain physics pulseshape derivative"};
    Gaudi::Property<std::string> m_filenameHiPhysDer{this, "filename_hi_phys_der", "dpulsehi_physics.dat", "High gain physics pulseshape derivative"};
    //=== Laser Pulse shapes and derivatives
    Gaudi::Property<std::string> m_filenameLoLas{this, "filename_lo_las", "pulselo_laser.dat", "Low gain laser pulseshape"};
    Gaudi::Property<std::string> m_filenameHiLas{this, "filename_hi_las", "pulsehi_laser.dat", "High gain laser pulseshape"};
    Gaudi::Property<std::string> m_filenameLoLasDer{this, "filename_lo_las_der", "dpulselo_laser.dat", "Low gain laser pulseshape derivative"};
    Gaudi::Property<std::string> m_filenameHiLasDer{this, "filename_hi_las_der", "dpulsehi_laser.dat", "High gain laser pulseshape derivative"};
    //=== CIS Leakage Pulse shapes and derivatives
    Gaudi::Property<std::string> m_filenameSLeakLo{this, "filename_sleaklo", "leaklo.0230055.12.cap.5.2.dat", "Low gain CIS leakage pulse 5pF"};
    Gaudi::Property<std::string> m_filenameLeakLo{this, "filename_leaklo", "leaklo_100.dat", "Low gain CIS leakage pulse 100pF"};
    Gaudi::Property<std::string> m_filenameDSLeakLo{this, "filename_dsleaklo", "dsleaklo.0230055.12.cap.5.2.dat", "Derivative low gain CIS leakage pulse 5pF"};
    Gaudi::Property<std::string> m_filenameDLeakLo{this, "filename_dleaklo", "dleaklo_100.dat", "Derivative low gain CIS leakage pulse 100pF"};
    Gaudi::Property<std::string> m_filenameSLeakHi{this, "filename_sleakhi", "leakhi.0230055.12.cap.5.2.dat", "High gain CIS leakage pulse 5pF"};
    Gaudi::Property<std::string> m_filenameLeakHi{this, "filename_leakhi", "leakhi_100.dat", "High gain CIS leakage pulse 100pF"};
    Gaudi::Property<std::string> m_filenameDSLeakHi{this, "filename_dsleakhi", "dsleakhi.0230055.12.cap.5.2.dat", "Derivative high gain CIS leakage pulse 5pF"};
    Gaudi::Property<std::string> m_filenameDLeakHi{this, "filename_dleakhi", "dleakhi_100.dat", "Derivative high gain CIS leakage pulse 100pF"};
    //=== Digitizer noise RMS as a function of channel number
    Gaudi::Property<std::string> m_filenameOrigNoise{this, "filename_orig_noise", "noise_digi_orig.dat", "file with noise rms per channel"};
    Gaudi::Property<std::string> m_filenameNkNoise{this, "filename_nk_noise", "noise_digi_nk.dat", "file with noise rms per channel"};

    //=== TileWienerFilterWeights configuration
    Gaudi::Property<bool> m_loadWienerFilterWeights{this, "LoadWienerFilterWeights", false, "Load Wiener filter weights"};
    /** Forwarded to TileWienerFilterWeights in initialize() */
    Gaudi::Property<int> m_WFNSamplesPhys{this, "WienerFilterPhysicsNSamples", 7, "Number of samples for Wiener filter physics weights"};
    Gaudi::Property<int> m_WFLuminosity{this, "WienerFilterLuminosity", 40, "Luminosity for Wiener filter weights"};

    // name of TileCalibContainer in detector store
    std::string m_calibCntName;

    std::vector<std::string> m_connectedDrawers; // list of connected drawers
    // we assume that list contains even number of elements - pairs of (begin,end) range,
    // e.g. all drawers connected looks like this:
    // [ "0x100", "0x13F", "0x200", "0x23F", "0x300", "0x33F", "0x400", "0x43F" ]
    // and this is the list for few drawers in LBA connected:
    // [ "0x10A", "0x10C", "0x121", "0x123" ]
    // empty list is equivalent to list with all drawers connected

    TileInfoLoader(const TileInfoLoader&);
    TileInfoLoader& operator=(const TileInfoLoader&);
};

#endif  // TILECONDITIONS_TILEINFOLOADER_H
