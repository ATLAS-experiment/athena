/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PixelGeoModelXml/PixelGmxInterface.h"

#include <InDetGeoModelUtils/WaferTree.h>
#include <InDetReadoutGeometry/SiDetectorDesign.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetSimEvent/SiHitIdHelper.h>
#include <PixelReadoutGeometry/PixelDetectorManager.h>
#include <PixelReadoutGeometry/PixelModuleDesign.h>
#include <ReadoutGeometryBase/PixelDiodeMatrix.h>
#include <ReadoutGeometryBase/SiCommonItems.h>

#include <RDBAccessSvc/IRDBAccessSvc.h>
#include <RDBAccessSvc/IRDBRecord.h>
#include <RDBAccessSvc/IRDBRecordset.h>
#include <GeoModelRead/ReadGeoModel.h>
#include <GeoModelKernel/GeoFullPhysVol.h>

namespace
{
constexpr int PixelHitIndex{0};
}


namespace InDetDD
{

namespace ITk
{

PixelGmxInterface::PixelGmxInterface(PixelDetectorManager *detectorManager,
                                     SiCommonItems *commonItems,
                                     WaferTree *moduleTree)
  : AthMessaging("PixelGmxInterface"),
    m_detectorManager(detectorManager),
    m_commonItems(commonItems),
    m_moduleTree(moduleTree)
{}


int PixelGmxInterface::sensorId(std::map<std::string, int> &index) const
{
  // Return the Simulation HitID (nothing to do with "ATLAS Identifiers" aka "Offline Identifiers")

  // Check if identifier is valid
  // TODO: drop this check in the future
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PixelID in PixelGmxInterface::sensorId");
    return -1;
  }
  Identifier id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                          index["layer_wheel"],
                                          index["phi_module"],
                                          index["eta_module"]);
  IdentifierHash hashId = pixelIdHelper->wafer_hash(id);
  if (!hashId.is_valid()) {
    ATH_MSG_WARNING("Invalid hash for Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                    << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
    return -1;
  }

  // Compute the actuall SiHitId
  int hitIdOfModule = SiHitIdHelper::GetHelper()->buildHitId(PixelHitIndex,
                                                             index["barrel_endcap"],
                                                             index["layer_wheel"],
                                                             index["eta_module"],
                                                             index["phi_module"],
                                                             index["side"]);

  ATH_MSG_DEBUG("Index list: " << index["barrel_endcap"] << " " << index["layer_wheel"] << " "
                               << index["eta_module"] << " " << index["phi_module"] << " " << index["side"]);
  ATH_MSG_DEBUG("hitIdOfModule = " << std::hex << hitIdOfModule << std::dec);
  ATH_MSG_DEBUG(" bec = " << SiHitIdHelper::GetHelper()->getBarrelEndcap(hitIdOfModule)
                << " lay = " << SiHitIdHelper::GetHelper()->getLayerDisk(hitIdOfModule)
                << " eta = " << SiHitIdHelper::GetHelper()->getEtaModule(hitIdOfModule)
                << " phi = " << SiHitIdHelper::GetHelper()->getPhiModule(hitIdOfModule)
                << " side = " << SiHitIdHelper::GetHelper()->getSide(hitIdOfModule));

  return hitIdOfModule;
}


void PixelGmxInterface::addSensorType(const std::string& clas,
                                      const std::string& typeName,
                                      const std::map<std::string, std::string>& parameters)
{
  ATH_MSG_DEBUG("addSensorType called for class " << clas << ", typeName " << typeName);

  if (clas == "SingleChip_RD53" || clas == "QuadChip_RD53") {
    makePixelModule(typeName, parameters);
  } else {
    ATH_MSG_ERROR("addSensorType: unrecognised module class: " << clas);
    ATH_MSG_ERROR("No module design created");
  }
}


void PixelGmxInterface::makePixelModule(const std::string &typeName,
                                        const std::map<std::string, std::string> &parameters)
{
  int circuitsPerEta{2}; // row
  int circuitsPerPhi{2}; // column
  double thickness{0.150};
  double pitchEta{};
  double pitchPhi{};
  double pitchEtaLong{};
  double pitchPhiLong{};
  double pitchEtaEnd{};
  double pitchPhiEnd{};
  int nEtaLongPerSide{};
  int nPhiLongPerSide{};
  int nEtaEndPerSide{};
  int nPhiEndPerSide{};
  int rowsPerChip{};
  int columnsPerChip{};

  // unused
  InDetDD::CarrierType carrier{InDetDD::electrons};
  int readoutSide{1};
  bool is3D{false};

  bool phiSymmetric{true};
  bool etaSymmetric{true};
  bool depthSymmetric{true};

  // read parameters
  getParameter(typeName, parameters, "circuitsPerEta", circuitsPerEta);
  getParameter(typeName, parameters, "circuitsPerPhi", circuitsPerPhi);
  getParameter(typeName, parameters, "thickness", thickness);
  getParameter(typeName, parameters, "is3D", is3D);
  getParameter(typeName, parameters, "rows", rowsPerChip);
  getParameter(typeName, parameters, "columns", columnsPerChip);
  getParameter(typeName, parameters, "pitchEta", pitchEta);
  getParameter(typeName, parameters, "pitchPhi", pitchPhi);
  getParameter(typeName, parameters, "pitchEtaLong", pitchEtaLong);
  getParameter(typeName, parameters, "pitchPhiLong", pitchPhiLong);
  getParameter(typeName, parameters, "pitchEtaEnd", pitchEtaEnd);
  getParameter(typeName, parameters, "pitchPhiEnd", pitchPhiEnd);
  getParameter(typeName, parameters, "nPhiLongPerSide", nPhiLongPerSide);
  getParameter(typeName, parameters, "nEtaLongPerSide", nEtaLongPerSide);
  getParameter(typeName, parameters, "nPhiEndPerSide", nPhiEndPerSide);
  getParameter(typeName, parameters, "nEtaEndPerSide", nEtaEndPerSide);

  checkParameter(typeName, parameters, "phiSymmetric", phiSymmetric);
  checkParameter(typeName, parameters, "etaSymmetric", etaSymmetric);
  checkParameter(typeName, parameters, "depthSymmetric", depthSymmetric);

  //
  // Make Module Design and add to DetectorManager
  //
  std::shared_ptr<const PixelDiodeMatrix> fullMatrix = buildMatrix(pitchPhi, pitchEta,
                                                                   pitchPhiLong, pitchPhiEnd,
                                                                   pitchEtaLong, pitchEtaEnd,
                                                                   nPhiLongPerSide, nPhiEndPerSide,
                                                                   nEtaLongPerSide, nEtaEndPerSide,
                                                                   circuitsPerPhi, circuitsPerEta,
                                                                   columnsPerChip, rowsPerChip);

  ATH_MSG_DEBUG("fullMatrix = buildMatrix(" << pitchPhi << ", " << pitchEta << ", "
                                            << pitchPhiLong << ", " << pitchPhiEnd << ", "
                                            << pitchEtaLong << ", " << pitchEtaEnd << ", "
                                            << nPhiLongPerSide << ", " << nPhiEndPerSide << ", "
                                            << nEtaLongPerSide << ", " << nEtaEndPerSide << ", "
                                            << circuitsPerPhi << ", " << circuitsPerEta << ", "
                                            << columnsPerChip << ", " << rowsPerChip << ")");
  ATH_MSG_DEBUG("readout geo - design " << thickness << " "
                                        << circuitsPerPhi << " " << circuitsPerEta << " "
                                        << columnsPerChip << " " << rowsPerChip << " "
                                        << columnsPerChip << " " << rowsPerChip << " "
                                        << carrier << " " << readoutSide);

  //For optionally setting PixelBarrel,PixelEndcap,PixelInclined
  //(so far) primarily useful for the latter to avoid orientation warnings
  InDetDD::DetectorType detectorType{InDetDD::PixelBarrel};  // TODO: we should probably fail and not default to barrel here.
  int detectorTypeEnum = 0;
  if (checkParameter(typeName, parameters, "detectorType", detectorTypeEnum)) {
    if (detectorTypeEnum == 1) detectorType = InDetDD::PixelBarrel;
    else if (detectorTypeEnum == 2) detectorType = InDetDD::PixelEndcap;
    else if (detectorTypeEnum == 3) detectorType = InDetDD::PixelInclined;
  }

  auto design = std::make_unique<PixelModuleDesign>(thickness,
                                                    phiSymmetric, etaSymmetric, depthSymmetric,
                                                    circuitsPerPhi, circuitsPerEta,
                                                    columnsPerChip, rowsPerChip,
                                                    columnsPerChip, rowsPerChip,
                                                    fullMatrix, carrier,
                                                    readoutSide, is3D, detectorType);
  

  ATH_MSG_DEBUG("readout geo - design : " << design->width() << " " << design->length() << " " << design->thickness() << " " <<design->rows() << " " << design->columns());

  [[maybe_unused]] auto observePtr = m_detectorManager->addDesign(std::move(design));

  // Add to map for addModule routine
  m_geometryMap[typeName] = m_detectorManager->numDesigns() - 1;
}


void PixelGmxInterface::addSensor(const std::string& typeName,
                                  std::map<std::string, int> &index,
                                  int /*sensitiveId*/,
                                  GeoVFullPhysVol *fpv)
{
  //
  // Get the ATLAS "Offline" wafer identifier
  //
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *>(m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Failed dynamic_cast to PixelID in PixelGmxInterface::addSensor");
    return;
  }
  Identifier id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                          index["layer_wheel"],
                                          index["phi_module"],
                                          index["eta_module"]);
  IdentifierHash hashId = pixelIdHelper->wafer_hash(id);
  //
  //    Now do our best to check if this is a valid id. If either the gmx file is wrong, or the xml file
  //    defining the allowed id's is wrong, you can get disallowed id's. These cause a crash later
  //    if allowed through. To do the check, we ask for the hash-id of this id. Invalid ids give a
  //    special invalid hash-id (0xFFFFFFFF). But we don't exit the run, to help debug things quicker.
  //
  if (!hashId.is_valid()) {
    ATH_MSG_ERROR("Invalid id for sensitive module " << typeName << " volume with indices");
    for (const auto& [key, value] : index) {
      msg() << MSG::ERROR << key << " = " << value << "; ";
    }
    msg() << MSG::ERROR << endmsg;
    ATH_MSG_ERROR("Refusing to make it into a sensitive element. Incompatible gmx and identifier-xml files.");
    return;
  }

  //
  // Create the detector element and add to the DetectorManager
  //
  auto it = m_geometryMap.find(typeName);
  if(it == m_geometryMap.end()) {
    ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }
  const SiDetectorDesign *design = m_detectorManager->getDesign(it->second);
  ATH_MSG_VERBOSE("Adding sensor with design: " << typeName << " " << design);
  if (design == nullptr) {
    ATH_MSG_ERROR("addSensor: Error: Readout sensor type " << typeName << " not found.");
    throw std::runtime_error("readout sensor type " + typeName + " not found.");
  }

  m_detectorManager->addDetectorElement(new SiDetectorElement(id, design, fpv, m_commonItems));

  //
  // Build up a map-structure for numerology
  //
  Wafer module((unsigned int) hashId);
  std::string errorMessage("");
  if (!m_moduleTree->add(index["barrel_endcap"],
                         index["layer_wheel"],
                         index["eta_module"],
                         index["phi_module"],
                         module,
                         errorMessage)) {
    ATH_MSG_ERROR(errorMessage);
  }

  return;
}


std::shared_ptr<const PixelDiodeMatrix> PixelGmxInterface::buildMatrix(double phiPitch, double etaPitch,
                                                                       double phiPitchLong, double phiPitchEnd,
                                                                       double etaPitchLong, double etaPitchEnd,
                                                                       int nPhiLong, int nPhiEnd,
                                                                       int nEtaLong, int nEtaEnd,
                                                                       int circuitsPhi, int circuitsEta,
                                                                       int diodeColPerCirc, int diodeRowPerCirc) const
{
  // checking for unlogical values
  if (circuitsPhi < 1 or circuitsEta < 1) {
    ATH_MSG_WARNING("Number of circuits is 0");
    return nullptr;
  }
  if (diodeRowPerCirc < 1 or diodeColPerCirc < 1) {
    ATH_MSG_WARNING("Number of diodes per circuit is 0");
    return nullptr;
  }
  if (nPhiLong < 0 or nPhiEnd < 0 or nEtaLong < 0 or nEtaEnd < 0) {
    ATH_MSG_WARNING("Number of long/end cells per circuit is below 0");
    return nullptr;
  }

  // checking and correcting inconsistent values
  if (nPhiLong == 0 and not (phiPitchLong == 0.0 or phiPitchLong == phiPitch)) {
    ATH_MSG_DEBUG("nPhiLong is set to 0, but phiPitchLong is neither 0 nor phiPitch! Setting nPhiLong to 1");
    nPhiLong = 1;
  }
  if (nPhiEnd == 0 and not (phiPitchEnd == 0.0 or phiPitchEnd == phiPitch)) {
    ATH_MSG_DEBUG("nPhiEnd is set to 0, but phiPitchEnd is neither 0 nor phiPitch! Setting nPhiEnd to 1");
    nPhiEnd = 1;
  }
  if (nEtaLong == 0 and not (etaPitchLong == 0.0 or etaPitchLong == etaPitch)) {
    ATH_MSG_DEBUG("nEtaLong is set to 0, but etaPitchLong is neither 0 nor etaPitch! Setting nEtaLong to 1");
    nEtaLong = 1;
  }
  if (nEtaEnd == 0 and not (etaPitchEnd == 0.0 or etaPitchEnd == etaPitch)) {
    ATH_MSG_DEBUG("nEtaEnd is set to 0, but etaPitchEnd is neither 0 nor etaPitch! Setting nEtaEnd to 1");
    nEtaEnd = 1;
  }

  /*
    The naming of internal PixelDiodeMatrix cell objects here follows the
    convention of cell_XX, where X is N for normal, L for long or E for end.
    The first index denotes the phi direction, the second eta.
  */

  // creation of individual pixels
  std::shared_ptr<const PixelDiodeMatrix> cell_NN{};
  std::shared_ptr<const PixelDiodeMatrix> cell_NL{};
  std::shared_ptr<const PixelDiodeMatrix> cell_NE{};
  std::shared_ptr<const PixelDiodeMatrix> cell_LN{};
  std::shared_ptr<const PixelDiodeMatrix> cell_LL{};
  std::shared_ptr<const PixelDiodeMatrix> cell_LE{};
  std::shared_ptr<const PixelDiodeMatrix> cell_EN{};
  std::shared_ptr<const PixelDiodeMatrix> cell_EL{};
  std::shared_ptr<const PixelDiodeMatrix> cell_EE{};

  // only filling long/end pixels if needed
  cell_NN = PixelDiodeMatrix::construct(phiPitch, etaPitch);
  if (nEtaLong > 0) {cell_NL = PixelDiodeMatrix::construct(phiPitch, etaPitchLong);}
  if (nEtaEnd > 0)  {cell_NE = PixelDiodeMatrix::construct(phiPitch, etaPitchEnd);}

  if (nPhiLong > 0) {
    cell_LN = PixelDiodeMatrix::construct(phiPitchLong, etaPitch);
    if (nEtaLong > 0) {cell_LL = PixelDiodeMatrix::construct(phiPitchLong, etaPitchLong);}
    if (nEtaEnd > 0)  {cell_LE = PixelDiodeMatrix::construct(phiPitchLong, etaPitchEnd);}
  }
  if (nPhiEnd > 0) {
    cell_EN = PixelDiodeMatrix::construct(phiPitchEnd, etaPitch);
    if (nEtaLong > 0) {cell_EL = PixelDiodeMatrix::construct(phiPitchEnd, etaPitchLong);}
    if (nEtaEnd > 0)  {cell_EE = PixelDiodeMatrix::construct(phiPitchEnd, etaPitchEnd);}
  }

  // creation of long/end cell blocks (in case there are more then one long/end per cicuit)
  if (nPhiLong > 1) {
    if (cell_LN) {cell_LN = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_LN, nPhiLong, nullptr);}
    if (cell_LL) {cell_LL = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_LL, nPhiLong, nullptr);}
    if (cell_LE) {cell_LE = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_LE, nPhiLong, nullptr);}
  }
  if (nPhiEnd > 1) {
    if (cell_EN) {cell_EN = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_EN, nPhiEnd, nullptr);}
    if (cell_EL) {cell_EL = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_EL, nPhiEnd, nullptr);}
    if (cell_EE) {cell_EE = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, nullptr, cell_EE, nPhiEnd, nullptr);}
  }
  if (nEtaLong > 1) {
    if (cell_NL) {cell_NL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_NL, nEtaLong, nullptr);}
    if (cell_LL) {cell_LL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_LL, nEtaLong, nullptr);}
    if (cell_EL) {cell_EL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_EL, nEtaLong, nullptr);}
  }
  if (nEtaEnd > 1) {
    if (cell_NE) {cell_NE = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_NE, nEtaEnd, nullptr);}
    if (cell_LE) {cell_LE = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_LE, nEtaEnd, nullptr);}
    if (cell_EE) {cell_EE = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, nullptr, cell_EE, nEtaEnd, nullptr);}
  }

  /*
    The naming of internal PixelDiodeMatrix cell objects here follows the
    convention of row_XY, where X is for phi N, L or E as before.
    Y is for eta:
    - L for a lower chip
    - M for a middle chip
    - U for an upper chip
    The first index denotes the phi direction, the second eta.
    If just one index is given, it is phi and eta is a full row.
  */

  // putting together the single chip rows (eta direction)
  std::shared_ptr<const PixelDiodeMatrix> fullChipRow_N{};
  std::shared_ptr<const PixelDiodeMatrix> fullChipRow_L{};
  std::shared_ptr<const PixelDiodeMatrix> fullChipRow_E{};
  if (circuitsEta == 1) {
    // special case of just one circuit in eta direction (no long cells, just end)
    fullChipRow_N = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_NE, std::move(cell_NN) , diodeColPerCirc - 2*nEtaEnd, cell_NE);
    if (cell_LN) {fullChipRow_L = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_LE, std::move(cell_LN), diodeColPerCirc - 2*nEtaEnd, cell_LE);}
    if (cell_EN) {fullChipRow_E = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_EE, std::move(cell_EN), diodeColPerCirc - 2*nEtaEnd, cell_EE);}
  } else {
    // rows of individual chips
    auto singleChipRow_NL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_NE, cell_NN, diodeColPerCirc -nEtaEnd  -nEtaLong, cell_NL);
    auto singleChipRow_NM = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_NL, cell_NN, diodeColPerCirc -nEtaLong -nEtaLong, cell_NL);
    auto singleChipRow_NU = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(cell_NL), std::move(cell_NN), diodeColPerCirc -nEtaLong -nEtaEnd,  std::move(cell_NE));

    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_LL{};
    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_LM{};
    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_LU{};
    if (cell_LN) {
      singleChipRow_LL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_LE, cell_LN, diodeColPerCirc -nEtaEnd  -nEtaLong, cell_LL);
      singleChipRow_LM = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_LL, cell_LN, diodeColPerCirc -nEtaLong -nEtaLong, cell_LL);
      singleChipRow_LU = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(cell_LL), cell_LN, diodeColPerCirc -nEtaLong -nEtaEnd,  std::move(cell_LE));
    }

    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_EL{};
    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_EM{};
    std::shared_ptr<const PixelDiodeMatrix> singleChipRow_EU{};
    if (cell_EN) {
      singleChipRow_EL = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_EE, cell_EN, diodeColPerCirc -nEtaEnd  -nEtaLong, cell_EL);
      singleChipRow_EM = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, cell_EL, cell_EN, diodeColPerCirc -nEtaLong -nEtaLong, cell_EL);
      singleChipRow_EU = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(cell_EL), cell_EN, diodeColPerCirc -nEtaLong -nEtaEnd,  std::move(cell_EE));
    }

    // putting together the single chip rows
    if (circuitsEta == 2) {
      // special case of no middle chips in eta (just lower and upper)
      fullChipRow_N = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_NL), std::move(singleChipRow_NU), 1, nullptr);
      if (cell_LN) {fullChipRow_L = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_LL), std::move(singleChipRow_LU), 1, nullptr);}
      if (cell_EN) {fullChipRow_E = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_EL), std::move(singleChipRow_EU), 1, nullptr);}
    } else {
      fullChipRow_N = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_NL), std::move(singleChipRow_NM), circuitsEta-2, std::move(singleChipRow_NU) );
      if (cell_LN) {fullChipRow_L = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_LL) , std::move(singleChipRow_LM), circuitsEta-2, std::move(singleChipRow_LU));}
      if (cell_EN) {fullChipRow_E = PixelDiodeMatrix::construct(PixelDiodeMatrix::etaDir, std::move(singleChipRow_EL), std::move(singleChipRow_EM), circuitsEta-2, std::move(singleChipRow_EU));}
    }
  }

  // combining the full eta rows to the full Matrix
  std::shared_ptr<const PixelDiodeMatrix> fullMatrix{};
  if (circuitsPhi == 1) {
    // special case of just one circuit in eta direction (no long cells, just end)
    fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, fullChipRow_E, std::move(fullChipRow_N), diodeRowPerCirc - 2*nPhiEnd, fullChipRow_E);
  } else {
    // columns of individual chips
    auto singleChipCol_L = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, fullChipRow_E, fullChipRow_N, diodeRowPerCirc -nPhiEnd  -nPhiLong, fullChipRow_L);
    auto singleChipCol_M = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, fullChipRow_L, fullChipRow_N, diodeRowPerCirc -nPhiLong -nPhiLong, fullChipRow_L);
    auto singleChipCol_U = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, std::move(fullChipRow_L), std::move(fullChipRow_N) , diodeRowPerCirc -nPhiLong -nPhiEnd,  std::move(fullChipRow_E));

    // putting together the single chip rows
    if (circuitsPhi == 2) {
      // special case of no middle chips in phi (just lower and upper)
      fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir, std::move(singleChipCol_L), std::move(singleChipCol_U), 1, nullptr);
    } else {
      fullMatrix = PixelDiodeMatrix::construct(PixelDiodeMatrix::phiDir,  std::move(singleChipCol_L), std::move(singleChipCol_M), circuitsPhi-2, std::move(singleChipCol_U));
    }
  }

  return fullMatrix;
}

void PixelGmxInterface::buildReadoutGeometryFromSqlite(IRDBAccessSvc * rdbAccessSvc,GeoModelIO::ReadGeoModel* sqlreader){

    const std::array<std::string,2> sensorTypes{"QuadChip_RD53","SingleChip_RD53"};
    const std::array<std::string,17> rd53_ParamNames{"circuitsPerEta","circuitsPerPhi","columns","detectorType","is3D","nEtaEndPerSide","nEtaLongPerSide","nPhiEndPerSide","nPhiLongPerSide","pitchEta","pitchEtaEnd","pitchEtaLong","pitchPhi","pitchPhiEnd","pitchPhiLong","rows","thickness"};
    
    for(const std::string & sType:sensorTypes){
       IRDBRecordset_ptr rd53 = rdbAccessSvc->getRecordsetPtr(sType,"");
       if(rd53->size() !=0){
          for (unsigned int iR =0;iR<rd53->size();iR++){
            std::map<std::string,std::string> rd53_Map;
            for(const std::string & paramName:rd53_ParamNames){
            std::string paramValue = (*rd53)[iR]->getString(paramName);
            rd53_Map[paramName] = std::move(paramValue);
        }
           std::string rd35_Name = (*rd53)[iR]->getString("SensorType");
           makePixelModule(rd35_Name,rd53_Map);
          } 
       }
    else ATH_MSG_WARNING("Could not retrieve "<<sType<<" table");
    }

    //Now, loop over the FullPhysVols and create the SiDetectorElements
    //lots of string parsing...
    const std::array<std::string,5> fields{"barrel_endcap","layer_wheel","phi_module","eta_module","side"}; 
    //First, find which name the tables are in the file under (depends upon the plugin used to create the input file)
    //sort these in order of precedence - ITkPlugin, then ITkPixelPlugin, then GeoModelXMLPlugin
    const std::array<std::string,3> publishers({"ITk","ITkPixel","GeoModelXML"});
    //The below is a map of string keys which will contain all the Identifier/DetElement relevant info, and the associated FullPhysVol
    // (once filled from the published table in the SQLite)
    std::map<std::string, GeoFullPhysVol*> mapFPV;
    for (auto & iPub : publishers){
        //setting the "checkTable" option to true, so that an empty map will be returned if not found and we can try the next one
         mapFPV = sqlreader->getPublishedNodes<std::string, GeoFullPhysVol*>(iPub,true);
         if (!mapFPV.empty()) {
            ATH_MSG_DEBUG("Using FPV tables from publisher "<<iPub);
            break;
         }
    }
    if (mapFPV.empty()) ATH_MSG_ERROR("Could not find any FPV tables under the expected names: "<<publishers);

    for (const auto&[fullPhysVolInfoString, fullPhysVolPointer] : mapFPV){
        //find the name of the corresponding detector design type
        size_t startRG = fullPhysVolInfoString.find("RD53_");
        if(startRG==std::string::npos){
            ATH_MSG_DEBUG("GeoFullPhysVol "<<fullPhysVolInfoString<<" does not have the expected format. Skipping");
            continue;
            } 
        std::string typeName = fullPhysVolInfoString.substr(startRG);
        std::map<std::string, int> index;
        for (const std::string & field:fields){
        size_t first = fullPhysVolInfoString.find(field+"_");
        size_t last = fullPhysVolInfoString.find('_',first+field.size()+1);//start looking only after end of first delimiter (plus 1 for the "_" appended) ends
        if(first==std::string::npos || last==std::string::npos){
            ATH_MSG_DEBUG("Could not extract "<<field<<" from "<<fullPhysVolInfoString<<". Skipping");
            continue;
            } 
        std::string strNew = fullPhysVolInfoString.substr(first+field.size()+1,last-(first+field.size()+1));
        index[field] = std::stoi(strNew);
        }

        addSensor(typeName,index,0,fullPhysVolPointer);
    }

}

void PixelGmxInterface::addAlignable(int level,
                                     std::map<std::string, int> &index,
                                     GeoVFullPhysVol *fpv,
                                     GeoAlignableTransform *transform)
{
  ATH_MSG_DEBUG("addAlignable called");
  const PixelID *pixelIdHelper = dynamic_cast<const PixelID *> (m_commonItems->getIdHelper());
  if (not pixelIdHelper){
    ATH_MSG_ERROR("Dynamic cast to PixelID failed in PixelGmxInterface::addAlignable");
    return;
  }
  Identifier id;
  switch (level) {
    case 0:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 index["side"]);
      break;
    case 1:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 index["phi_module"],
                                 index["eta_module"],
                                 0);
      break;
    case 2:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 index["layer_wheel"],
                                 0,
                                 0,
                                 0);
      break;
    case 3:
      id = pixelIdHelper->wafer_id(index["barrel_endcap"],
                                 0,
                                 0,
                                 0,
                                 0);
      break;
    default:
      throw GaudiException("Unknown level " + std::to_string(level) + " for alignment in addAlignable",
                           "PixelGmxInterface::addAlignable", StatusCode::FAILURE);
      break;
  }
  m_detectorManager->addAlignableTransform(level, id, transform, fpv);
}


} // namespace ITk
} // namespace InDetDD
