/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//  Filename : TileInfoLoader
//  Author   : Ed Frank
//  Created  : May, 2002
//
//  DESCRIPTION:
//     Implementation comments only.  Class level comments go in .h file.
//
//  HISTORY:
//     17May02  efrank  First version.
//
//  BUGS:
//
//*****************************************************************************

// Tile includes
#include "TileIdentifier/TileHWID.h"
#include "TileConditions/TileInfo.h"
#include "TileConditions/TileInfoLoader.h"
#include "TileConditions/TileCablingSvc.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TilePulseShapes.h"
#include "TileConditions/TileWienerFilterWeights.h"

// Calo includes
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/TileTBID.h"
#include "CaloIdentifier/CaloLVL1_ID.h"

// Athena includes
#include "StoreGate/StoreGateSvc.h"
#include "PathResolver/PathResolver.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "AthenaKernel/errorcheck.h"

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"


#include "TMatrixD.h"
#include "cstdio"
#include "cstdlib"
#include <iomanip>
#include <string>
#include <fstream>
#include <algorithm> //for std::none-of

//*****************************************************************************
TileInfoLoader::TileInfoLoader(const std::string& name,
			       ISvcLocator* pSvcLocator)
//*****************************************************************************
  : AthService(name, pSvcLocator)
  , m_detStore("DetectorStore", name)
  , m_pulsevar(new TilePulseShapes())
  , m_WFWeights(new TileWienerFilterWeights())
{
}

//*****************************************************************************
TileInfoLoader::~TileInfoLoader() {
//*****************************************************************************
  delete m_pulsevar;
  delete m_WFWeights;
}

//*****************************************************************************
StatusCode TileInfoLoader::initialize() {
//*****************************************************************************

  ATH_MSG_DEBUG( "Initializing...." << name() );

  auto info = std::make_unique<TileInfo>();

  // Forward properties.
  info->m_nSamples = m_nSamples;
  info->m_iTrigSample = m_iTrigSample;
  info->m_tileNoise = m_tileNoise;
  info->m_tileCoherNoise = m_tileCoherNoise;
  info->m_tileZeroSuppress = m_tileZeroSuppress;
  info->m_noiseScaleIndex = m_noiseScaleIndex;
  info->m_thresholdRawChannel = m_thresholdRawChannel;
  info->m_thresholdDigits = m_thresholdDigits;
  info->m_ttL1Calib = m_ttL1Calib;
  info->m_ttL1NoiseSigma = m_ttL1NoiseSigma;
  info->m_ttL1Thresh = m_ttL1Thresh;
  info->m_ttL1Ped = m_ttL1Ped;
  info->m_ttL1Max = m_ttL1Max;
  info->m_MuRcvCalib = m_MuRcvCalib;
  info->m_MuRcvNoiseSigma = m_MuRcvNoiseSigma;
  info->m_MuRcvThresh = m_MuRcvThresh;
  info->m_MuRcvPed = m_MuRcvPed;
  info->m_MuRcvMax = m_MuRcvMax;
  info->m_muL1Calib = m_muL1Calib;
  info->m_muL1NoiseSigma = m_muL1NoiseSigma;
  info->m_muL1Thresh = m_muL1Thresh;
  info->m_muL1Ped = m_muL1Ped;
  info->m_muL1Max = m_muL1Max;
  info->m_mbtsL1Calib = m_mbtsL1Calib;
  info->m_mbtsL1NoiseSigma = m_mbtsL1NoiseSigma;
  info->m_mbtsL1Thresh = m_mbtsL1Thresh;
  info->m_mbtsL1Ped = m_mbtsL1Ped;
  info->m_mbtsL1Max = m_mbtsL1Max;
  info->m_ttL1CosmicsGain = m_ttL1CosmicsGain;
  info->m_ttL1CosmicsThresh = m_ttL1CosmicsThresh;
  info->m_ADCmax = m_ADCmax;
  info->m_ADCmaskValue = m_ADCmaskValue;

  // Forward reference pulse shape filenames to TilePulseShapes.
  m_pulsevar->m_filenameLoCIS = m_filenameLoCIS;
  m_pulsevar->m_filenameLoCISDer = m_filenameLoCISDer;
  m_pulsevar->m_filenameSLoCIS = m_filenameSLoCIS;
  m_pulsevar->m_filenameSLoCISDer = m_filenameSLoCISDer;
  m_pulsevar->m_filenameHiCIS = m_filenameHiCIS;
  m_pulsevar->m_filenameHiCISDer = m_filenameHiCISDer;
  m_pulsevar->m_filenameSHiCIS = m_filenameSHiCIS;
  m_pulsevar->m_filenameSHiCISDer = m_filenameSHiCISDer;
  m_pulsevar->m_filenameLoPhys = m_filenameLoPhys;
  m_pulsevar->m_filenameHiPhys = m_filenameHiPhys;
  m_pulsevar->m_filenameLoPhysDer = m_filenameLoPhysDer;
  m_pulsevar->m_filenameHiPhysDer = m_filenameHiPhysDer;
  m_pulsevar->m_filenameLoLas = m_filenameLoLas;
  m_pulsevar->m_filenameHiLas = m_filenameHiLas;
  m_pulsevar->m_filenameLoLasDer = m_filenameLoLasDer;
  m_pulsevar->m_filenameHiLasDer = m_filenameHiLasDer;
  m_pulsevar->m_filenameSLeakLo = m_filenameSLeakLo;
  m_pulsevar->m_filenameLeakLo = m_filenameLeakLo;
  m_pulsevar->m_filenameDSLeakLo = m_filenameDSLeakLo;
  m_pulsevar->m_filenameDLeakLo = m_filenameDLeakLo;
  m_pulsevar->m_filenameSLeakHi = m_filenameSLeakHi;
  m_pulsevar->m_filenameLeakHi = m_filenameLeakHi;
  m_pulsevar->m_filenameDSLeakHi = m_filenameDSLeakHi;
  m_pulsevar->m_filenameDLeakHi = m_filenameDLeakHi;
  m_pulsevar->m_filenameOrigNoise = m_filenameOrigNoise;
  m_pulsevar->m_filenameNkNoise = m_filenameNkNoise;

  // Forward Wiener filter configuration to TileWienerFilterWeights.
  m_WFWeights->m_NSamples_Phys = m_WFNSamplesPhys;
  m_WFWeights->m_Luminosity = m_WFLuminosity;

  m_WFWeights->m_NSamples_Phys = info->m_nSamples; // to make sure that everything is consistent

  //=== Find the detector store service.
  CHECK( m_detStore.retrieve() );

  SmartIF<IGeoModelSvc> geoModel{service("GeoModelSvc")};
  if(!geoModel) {
    ATH_MSG_ERROR( "Could not locate GeoModelSvc" );
  } else {
    // check the DetDescr version
    std::string atlasVersion = geoModel->atlasVersion();
    int geo  = atlasVersion.compare(0,9,"ATLAS-GEO");
    int run1 = atlasVersion.compare(0,8,"ATLAS-R1");
    int ibl  = atlasVersion.compare(0,9,"ATLAS-IBL");
    int run2 = atlasVersion.compare(0,8,"ATLAS-R2");
    int comm = atlasVersion.compare(0,10,"ATLAS-Comm");
    int upg  = atlasVersion.compare(0,7,"ATLAS-P") ;
    int ver = 0;
    const auto comparisons={geo, run1, ibl, run2, comm, upg};
    bool nothing_found = std::none_of(comparisons.begin(), comparisons.end(), [](int zeroMeansFound){return zeroMeansFound==0;});
    GeoModel::GeoConfig geoConfig = geoModel->geoConfig();
    bool RUN2 = (nothing_found && (geoConfig==GeoModel::GEO_RUN2
                                   || geoConfig==GeoModel::GEO_RUN3
                                   || geoConfig==GeoModel::GEO_RUN4
                     )) || (run2 == 0); // calibration for all geometries >=RUN2 are the same as in RUN2

    if (geo == 0 || comm == 0 || ibl == 0 || run1 == 0 || run2 == 0 || upg == 0 || RUN2) {
      int pos = (atlasVersion.substr(9)).find('-');
      if(run1 == 0 || run2 == 0) pos = (atlasVersion.substr(13)).find('-') + 4;
      std::string geoVersion = atlasVersion.substr(pos+10,2)
                             + atlasVersion.substr(pos+13,2)
                             + atlasVersion.substr(pos+16,2);
      ver = atoi(geoVersion.c_str());
      ATH_MSG_DEBUG( "New ATLAS geometry detected: " << atlasVersion << " (" << geoVersion << ")"  << " version " << ver  );

      if (fabs(info->m_ttL1Calib - 4.1) < 0.001) {
        // They are TTL1Calib = 4.1 and TTL1NoiseSigma = 2.5 but should be 6.9 and 2.8 respectively.

        msg(MSG::INFO) << "Changing TTL1 calib from " << info->m_ttL1Calib;
        info->m_ttL1Calib = 6.9;
        msg(MSG::INFO) << " to " << info->m_ttL1Calib << endmsg;

        msg(MSG::INFO) << "Changing TTL1 noise sigma from " << info->m_ttL1NoiseSigma;
        info->m_ttL1NoiseSigma = 2.8;
        msg(MSG::INFO) << " to " << info->m_ttL1NoiseSigma << endmsg;
      }
    } else {
      ATH_MSG_DEBUG( "ATLAS geometry " << atlasVersion << " version " << ver );
    }

  }

  //=== Read in Shape profile of digits & TTL1
  CHECK( buildDigitsShapesHiLo(*info) );

  CHECK( buildTTL1Shapes(*info, m_TTL1ShapeFile, info->m_TTL1NBins, info->m_TTL1Time0Bin
                         , info->m_TTL1BinsPerX, info->m_TTL1FullShape, info->m_TTL1Shape) );


  CHECK( buildTTL1Shapes(*info, m_MuRcvShapeFile, info->m_MuRcvNBins, info->m_MuRcvTime0Bin
                         , info->m_MuRcvBinsPerX, info->m_MuRcvFullShape, info->m_MuRcvShape) );

  CHECK( buildTTL1Shapes(*info, m_MuL1ShapeFile, info->m_MuL1NBins, info->m_MuL1Time0Bin
                        , info->m_MuL1BinsPerX, info->m_MuL1FullShape, info->m_MuL1Shape) );

  //=== Read in coherent noise if required
  if (info->TileCoherNoise()) buildCovMatrix (*info);

  // put pointer to new TilePulseShapes in TileInfo
  // only if we want to use them (i.e. when we also read all calib files)
  info->m_pulseShapes = m_pulsevar;

  // point to WienerFilterWeights
  if (m_loadWienerFilterWeights) info->m_WienerFilterWeights=m_WFWeights;

  //=== Initialize and register TileInfo object
  CHECK( info->initialize() );

  CHECK( m_detStore->record(std::move(info), m_tileInfoName, false ) );
  ATH_MSG_DEBUG(  "Placed TileInfo object [" << m_tileInfoName << "] in the detector store."  );

  return StatusCode::SUCCESS;
}

//****************************************************************************
StatusCode TileInfoLoader::finalize() {
//****************************************************************************

  ATH_MSG_DEBUG( "TileInfoLoader::finalize()" );

  return Service::finalize();
}



////////////////////////////////////////////////////////////////////////////////
StatusCode TileInfoLoader::buildDigitsShapesHiLo(TileInfo& info) {

  ATH_MSG_DEBUG( "In buildDigitsShapesHiLo" );

  // Read in Shape profile of digits signal from external file.

  double It0;
  double dbuff;
  std::string line;

  // Hi gain
  ATH_MSG_DEBUG( " **** Start of High gain Pulse shapes in TileInfo " );

  // Find the full path to filename:
  std::string file_hi = PathResolver::find_file(m_digitsShapeFileHi, "DATAPATH");
  ATH_MSG_DEBUG( "Reading file  " << file_hi );

  //=== Test if files are there and can be opened
  if (file_hi.empty()) {
    ATH_MSG_ERROR( "Could not find input file " << m_digitsShapeFileHi.value() );
    return StatusCode::FAILURE;
  }

  std::ifstream shape_file_hi(file_hi.c_str());
  if (!shape_file_hi.is_open()) {
    ATH_MSG_ERROR( "Could not open input file " << file_hi );
    return StatusCode::FAILURE;
  }

  if (std::getline(shape_file_hi, line)) {
    ATH_MSG_DEBUG( endmsg << line );
  }

  if (std::getline(shape_file_hi, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsNBinsHi) == 1)
      ATH_MSG_DEBUG(  std::setw(3) << info.m_digitsNBinsHi << " number of bins in shaping function" );
  }

  if (std::getline(shape_file_hi, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsTime0BinHi) == 1)
      ATH_MSG_DEBUG( std::setw(3) << info.m_digitsTime0BinHi << " index of in-time bin" );
  }

  if (std::getline(shape_file_hi, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsBinsPerXHi) == 1)
      ATH_MSG_DEBUG( std::setw(3) << info.m_digitsBinsPerXHi << " bins per beam crossing" );
  }

  // Read in the Hi gain shape vector.
  info.m_digitsFullShapeHi.resize(info.m_digitsNBinsHi, 0.);
  size_t jt = 0;
  while (std::getline(shape_file_hi, line) && jt < info.m_digitsFullShapeHi.size()) {
    int nread = sscanf(line.c_str(), "%80lf %80lf", &It0, &dbuff);
    if (nread > 1) info.m_digitsFullShapeHi[jt++] = dbuff;
    ATH_MSG_VERBOSE( "t= " << It0 << " a= " << dbuff );
  }

  shape_file_hi.close();

  // Lo gain
  ATH_MSG_DEBUG( " **** Start of Low gain Pulse shapes in TileInfo " );

  // Find the full path to filename:
  std::string file_lo = PathResolver::find_file(m_digitsShapeFileLo, "DATAPATH");
  ATH_MSG_DEBUG( "Reading file  " << file_lo );

  //=== Test if files are there and can be opened
  if (file_lo.empty()) {
    ATH_MSG_ERROR( "Could not find input file " << m_digitsShapeFileLo.value() );
    return StatusCode::FAILURE;
  }

  std::ifstream shape_file_lo(file_lo.c_str());
  if (!shape_file_lo.is_open()) {
    ATH_MSG_ERROR( "Could not open input file " << file_lo  );
    return StatusCode::FAILURE;
  }

  if (std::getline(shape_file_lo, line)) {
    ATH_MSG_DEBUG( endmsg << line );
  }

  if (std::getline(shape_file_lo, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsNBinsLo) == 1)
      ATH_MSG_DEBUG( std::setw(3) << info.m_digitsNBinsLo << " number of bins in shaping function" );
  }

  if (std::getline(shape_file_lo, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsTime0BinLo) == 1)
      ATH_MSG_DEBUG( std::setw(3) << info.m_digitsTime0BinLo << " index of in-time bin" );
  }

  if (std::getline(shape_file_lo, line)) {
    if (sscanf(line.c_str(), "%80d", &info.m_digitsBinsPerXLo) == 1)
      ATH_MSG_DEBUG( std::setw(3) << info.m_digitsBinsPerXLo << " bins per beam crossing" );
  }

  // Read in the Lo gain shape vector.
  info.m_digitsFullShapeLo.resize(info.m_digitsNBinsLo, 0.);
  jt = 0;
  while (std::getline(shape_file_lo, line) && jt < info.m_digitsFullShapeLo.size()) {
    int nread = sscanf(line.c_str(), "%80lf %80lf", &It0, &dbuff);
    if (nread > 1) info.m_digitsFullShapeLo[jt++] = dbuff;
    ATH_MSG_VERBOSE( "t= " << It0 << " a= " << dbuff );
  }

  shape_file_lo.close();

  //find the greatest value in digitsFullShape (should equal info.m_digitsTime0Bin)
  int it0_hi = 0;
  int it0_lo = 0;
  double peak_hi = 0;
  double peak_lo = 0;
  for (int i = 0; i < info.m_digitsNBinsHi; i++) {
    if (info.m_digitsFullShapeHi[i] > peak_hi) {
      peak_hi = info.m_digitsFullShapeHi[i];
      it0_hi = i;
    }
  }

  for (int i = 0; i < info.m_digitsNBinsLo; i++) {
    if (info.m_digitsFullShapeLo[i] > peak_lo) {
      peak_lo = info.m_digitsFullShapeLo[i];
      it0_lo = i;
    }
  }

  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << " High Gain: Peak value = " << peak_hi
                    << " at bin = " << it0_hi
                    << ", info.m_digitsTime0BinHi = " << info.m_digitsTime0BinHi << endmsg;

    msg(MSG::DEBUG) << " Low Gain:  Peak value = " << peak_lo
                    << " at bin = " << it0_lo
                    << ", info.m_digitsTime0BinLow = " << info.m_digitsTime0BinLo << endmsg;
  }

  // Create vector of time profiles at beam crossings.
  info.m_digitsShapeHi.resize(info.m_nSamples, 0.);
  info.m_digitsShapeLo.resize(info.m_nSamples, 0.);
//  ATH_MSG_DEBUG( " Pasa el resize y va a entrar en el lopp" );

  for (int i = 0; i < info.m_nSamples; i++) {
    int j = info.m_digitsTime0BinHi + (i - info.m_iTrigSample) * info.m_digitsBinsPerXHi;
    if (j < 0) j = 0;
    if (j >= info.m_digitsNBinsHi) j = info.m_digitsNBinsHi-1;
    info.m_digitsShapeHi[i] = info.m_digitsFullShapeHi[j];
    int k = info.m_digitsTime0BinLo + (i - info.m_iTrigSample) * info.m_digitsBinsPerXLo;
    if (k < 0) k = 0;
    if (k >= info.m_digitsNBinsLo) k = info.m_digitsNBinsLo-1;
    info.m_digitsShapeLo[i] = info.m_digitsFullShapeLo[k];
  }

  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << " Shaping profile at beam crossings:  "
                    << " Number of samples = " << info.m_nSamples
                    << ", Sample at Time 0 = " << info.m_iTrigSample << endmsg;

    msg(MSG::DEBUG) << "  High gain Shape factor = ";
    for (int i = 0; i < info.m_nSamples; i++) {
      msg(MSG::DEBUG) << std::setiosflags(std::ios::fixed)
          << std::setiosflags(std::ios::showpoint) << std::setw(9) << std::setprecision(5)
          << info.m_digitsShapeHi[i];
    }

    msg(MSG::DEBUG) << endmsg;

    msg(MSG::DEBUG) << "  Low  gain Shape factor = ";
    for (int i = 0; i < info.m_nSamples; i++) {
      msg(MSG::DEBUG) << std::setiosflags(std::ios::fixed)
          << std::setiosflags(std::ios::showpoint) << std::setw(9) << std::setprecision(5)
          << info.m_digitsShapeLo[i];
    }
    msg(MSG::DEBUG) << endmsg;

  }

  // Create vector of derivatives from shaping vector.
  info.m_digitsDerivativeHi.resize(info.m_digitsNBinsHi, 0.);
  info.m_digitsDerivativeLo.resize(info.m_digitsNBinsLo, 0.);
  for (int i = 1; i < info.m_digitsNBinsHi - 1; i++) {
    info.m_digitsDerivativeHi[i] = (info.m_digitsFullShapeHi[i + 1]
        - info.m_digitsFullShapeHi[i - 1]) / 2;
  }

  for (int i = 1; i < info.m_digitsNBinsLo - 1; i++) {
    info.m_digitsDerivativeLo[i] = (info.m_digitsFullShapeLo[i + 1]
        - info.m_digitsFullShapeLo[i - 1]) / 2;
  }

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
StatusCode TileInfoLoader::buildTTL1Shapes(TileInfo& info,
                                           const std::string& ShapeFile,
                                           int &NBins, int &Time0Bin, int &BinsPerX
                                           , std::vector<double> &FullShape, std::vector<double> &Shape) {

  // Read in Shape profile of signal from external file.
  ATH_MSG_DEBUG( " **** Start of TTL1 shapes in TileInfo "  );

  // Find the full path to filename:
  std::string file = PathResolver::find_file(ShapeFile, "DATAPATH");
  ATH_MSG_DEBUG( "Reading file  " << file );

  if (file.empty()) {
    ATH_MSG_ERROR( "Could not find input file " << ShapeFile );
    return StatusCode::FAILURE;
  }

  std::ifstream shape_file(file.c_str());
  if (!shape_file.is_open()) {
    ATH_MSG_ERROR( "Could not open input file " << file );
    return StatusCode::FAILURE;
  }

  double It0;
  double dbuff;
  std::string line;

  if (std::getline(shape_file, line)) {
    ATH_MSG_DEBUG( endmsg << MSG::DEBUG << line );

  }

  if (std::getline(shape_file, line)) {
    if (sscanf(line.c_str(), "%80d", &NBins) == 1)
      ATH_MSG_DEBUG( std::setw(3) << NBins << " number of bins in shaping function" );
  }

  if (std::getline(shape_file, line)) {
    if (sscanf(line.c_str(), "%80d", &Time0Bin) == 1)
      ATH_MSG_DEBUG( std::setw(3) << Time0Bin << " index of in-time bin" );
  }

  if (std::getline(shape_file, line)) {
    if (sscanf(line.c_str(), "%80d", &BinsPerX) == 1)
      ATH_MSG_DEBUG( BinsPerX << " bins per beam crossing" );
  }

  // Read in the ttl1shape vector.
  FullShape.resize(NBins, 0.);
  size_t jt = 0;
  while (std::getline(shape_file, line) && jt < FullShape.size()) {
    int nread = sscanf(line.c_str(), "%80lf %80lf", &It0, &dbuff);
    if (nread > 1) FullShape[jt++] = dbuff;
    ATH_MSG_VERBOSE( "t= " << It0 << " a= " << dbuff );
  }

  shape_file.close();

  // Print out ttl1shape values.
  int nline = (NBins - 1) / 8 + 1;
  if (msgLvl(MSG::DEBUG)) {
    for (int jline = 0; jline < nline; jline++) {
      int j1 = 8 * jline;
      int j2 = j1 + 8;
      if (j2 > NBins) j2 = NBins;
      msg(MSG::DEBUG) << "  " << std::setw(3) << j1 << "  ";

      for (int i = j1; i < j2; i++) {
        msg(MSG::DEBUG) << std::setiosflags(std::ios::fixed)
            << std::setiosflags(std::ios::showpoint) << std::setw(9) << std::setprecision(5)
            << FullShape[i];
      }
      msg(MSG::DEBUG) << endmsg;
    }
  }

  //find the greatest value in ttl1shape (should equal Time0Bin)
  int it0 = 0;
  double peak = 0;
  for (int i = 0; i < NBins; i++) {
    if (FullShape[i] > peak) {
      peak = FullShape[i];
      it0 = i;
    }
  }

  ATH_MSG_DEBUG( " Peak value = " << peak
                << " at bin = " << it0
                << ", Time0Bin = " << Time0Bin );


  // Create vector of time profiles at beam crossings.
  Shape.resize(info.m_nSamples, 0.);
  for (int i = 0; i < info.m_nSamples; i++) {
    int j = Time0Bin + (i - info.m_iTrigSample) * BinsPerX;
    if (j < 0) j = 0;
    if (j >= NBins) j = NBins-1;
    Shape[i] = FullShape[j];
  }

  if (msgLvl(MSG::DEBUG)) {
    msg(MSG::DEBUG) << " TileInfo: profile at beam crossings:  "
                    << " Number of samples = " << info.m_nSamples
                    << ", Sample at Time 0 = " << info.m_iTrigSample << endmsg;

    msg(MSG::DEBUG) << "  Shape factor = ";
    for (int i = 0; i < info.m_nSamples; i++) {
      msg(MSG::DEBUG) << std::setiosflags(std::ios::fixed)
          << std::setiosflags(std::ios::showpoint) << std::setw(9) << std::setprecision(5)
          << Shape[i];
    }
    msg(MSG::DEBUG) << endmsg;
  }

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////
void TileInfoLoader::buildCovMatrix (TileInfo& info)
{
// author:F Spano' - beg.
// TOUPDATE:: In the future maybe read from a root file
// Read in Covariance Matrix for Hi Gain/Lo Gain - Barrel/Ext Barrel
// from four external files

// Changed by F. Veloso: full coverage of the Tilecal without using
// the same correlation matrix for all modules.

  char partstr[10];
  char gainstr[10];
  char buff[1024];
  std::string line;

  // Cicle over 4 partitions, 64 modules and 2 gains
  for (int part = 0; part < 4; ++part) {
    // Create DecoCovaria[part]
//    ATH_MSG_DEBUG( "begin DecoCovaria.push_back" );
    info.m_decoCovaria.push_back(std::vector<std::vector<TMatrixD*> >());
//    ATH_MSG_DEBUG( "DecoCovaria.push_back done" );
    for (int modu = 0; modu < 64; ++modu) {
      // Create DecoCovaria[part][modu]
//      ATH_MSG_DEBUG( "  begin DecoCovaria[part].push_back" );
      info.m_decoCovaria[part].push_back(std::vector<TMatrixD*>());
//      ATH_MSG_DEBUG( "  DecoCovaria[part].push_back done" );

      for (int gain = 0; gain < 2; ++gain) {
//        ATH_MSG_DEBUG( "  part=" << part << " modu=" << modu << " gain=" << gain );

        // Open file which corresponds to DecoCovaria[part][modu][gain]
        if (part == 0) sprintf(partstr, "EBA");
        else if (part == 1) sprintf(partstr, "LBA");
        else if (part == 2) sprintf(partstr, "LBC");
        else sprintf(partstr, "EBC");

        if (gain == 0) sprintf(gainstr, "Hi");
        else sprintf(gainstr, "Lo");

        sprintf(buff, "%s%s%02d%s.txt", m_DecoCovaFilePrefix.value().c_str(), partstr, modu + 1, gainstr);
        std::string file = PathResolver::find_file(buff, "DATAPATH");
//        ATH_MSG_DEBUG( "  buff=" << buff  << " file=" << file.c_str()  );

        if (file.empty()) {
          ATH_MSG_WARNING( "Could not find input file " << buff  );
        } else {
          ATH_MSG_DEBUG( "Reading file  " << file );

          std::ifstream cov_file(file.c_str());

          if (!cov_file.is_open()) {
            ATH_MSG_WARNING( "Can't read input files, setting tileCoherNoise flag to FALSE" );

            info.m_tileCoherNoise = false;
            return;
          } else {

            ATH_MSG_DEBUG( " **** Start of Decomposed Covariance Matrix Read Out" );

            if (std::getline(cov_file, line)) {
              ATH_MSG_DEBUG( "Matrix is being loaded: " << line );
            }

            //define Matrix dimension
            int dima = 0;
            if (std::getline(cov_file, line)) {
              if (sscanf(line.c_str(), "%80d", &dima) == 1)
                ATH_MSG_DEBUG( "The Dimension of the matrix is " << dima );
            }

            // define Matrix: at this level we cannot initialize it with a given one
            TMatrixD* pDecoCova = new TMatrixD(dima, dima);

            // load tokens to be searched for in a string
            char* word;
            char* saveptr;
            const char* TOKENS = { " \t\n" };

            // read Matrix
            int row = 0;
            while (std::getline(cov_file, line) && row < dima) {
              ATH_MSG_DEBUG( "line " << row << " is " << line );

              strncpy(buff, line.c_str(), 1023);
              for (int column = 0; column < dima; column++) {
                // Check for comment lines
                if (*buff == '!' || *buff == '*') continue;
                //
                if (column == 0) {
                  word = strtok_r(buff, TOKENS, &saveptr);
                } else {
                  word = strtok_r(nullptr, TOKENS, &saveptr);
                }
                double pippo = (word) ? atof(word) : 0.0;
                //            int nread = sscanf(buff, "%1f", &pippo);
                ATH_MSG_VERBOSE( "elem " << column << " is " << pippo );
                (*pDecoCova)(row, column) = pippo;
              }
              row++;
            }

            cov_file.close();
            // Fill DecoCovaria[part][modu][gain]
            info.m_decoCovaria[part][modu].push_back(pDecoCova);

          }
        }
      }
    }
  }
}
