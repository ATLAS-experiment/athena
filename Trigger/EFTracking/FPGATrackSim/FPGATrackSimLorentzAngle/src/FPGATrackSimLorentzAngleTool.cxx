// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimLorentzAngle/FPGATrackSimLorentzAngleTool.h"

#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

FPGATrackSim::LorentzAngleTool::LorentzAngleTool(const std::string& algname,
    const std::string& name, const IInterface* ifc)
    : AthAlgTool(algname, name, ifc) {
}

StatusCode FPGATrackSim::LorentzAngleTool::initialize() {
    ATH_MSG_DEBUG("Initializing FPGATrackSimLorentzAngleTool");

    // Retrieve managers and ID helpers
    ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
    ATH_CHECK(detStore()->retrieve(m_SCTId, "SCT_ID"));
    ATH_CHECK(detStore()->retrieve(m_pixelManager, "ITkPixel"));
    ATH_CHECK(detStore()->retrieve(m_SCTManager, "ITkStrip"));

    ATH_CHECK(m_lorentzAngleToolPixel.retrieve(EnableTool{m_useAthenaLorentzAngleTools}));
    ATH_CHECK(m_lorentzAngleToolStrip.retrieve(EnableTool{m_useAthenaLorentzAngleTools}));

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSim::LorentzAngleTool::updateHitPosition(FPGATrackSimHit & hit) const {
    float shift = 0.0;
    const IdentifierHash& hash = hit.getIdentifierHash();
    Amg::Vector2D localPos(hit.getPhiCoord(), hit.getEtaCoord());
    if(hit.isPixel()){
        // Get the shift value
        if (m_useAthenaLorentzAngleTools) {
            shift = m_lorentzAngleToolPixel->getLorentzShift(hash, Gaudi::Hive::currentContext());
        }
        else {
            shift = getLorentzAngleShift(hit);
        }
        if (m_shiftGlobalPosition) {
            // Get the pixel detector element
            const InDetDD::SiDetectorElement* siDE = m_pixelManager->getDetectorElement(hash);
            Identifier wafer_id = m_pixelId->wafer_id(hash);
            Identifier hit_id = m_pixelId->pixel_id(wafer_id, hit.getPhiIndex(), hit.getEtaIndex());
            // Get the cell from the ID
            InDetDD::SiCellId cell = siDE->cellIdFromIdentifier(hit_id);
            if (!cell.isValid()) {
                ATH_MSG_DEBUG("Pixel cell not valid for hitID " << hit_id);
                return StatusCode::FAILURE;
            }
            // Get the position of the cell
            InDetDD::SiLocalPosition silPos(siDE->rawLocalPositionOfCell(cell));
            // Amg::Vector2D localPos(silPos);

            localPos[Trk::locX] += shift; // apply the Lorentz angle shift
            InDetDD::SiCellId newCell = siDE->cellIdOfPosition(localPos); // find the new cell corresponding to the shifted position
            // InDetDD::SiLocalPosition newSilPos(siDE->rawLocalPositionOfCell(newCell)); 
            Amg::Vector3D newGlobalPos = siDE->globalPosition(localPos); // get the new global position
            if (newCell.phiIndex() < 0) {
                ATH_MSG_ERROR("Pixel new cell not valid for hitID " << hit_id << " with shift " << shift << " and localPosX " << localPos[Trk::locX] << " and localPosY " << localPos[Trk::locY] << " and stripIndex " << newCell.phiIndex() << ". Setting hit to module's edge.");
                return StatusCode::FAILURE;
            }
            // Update the hit's global position
            hit.setX(newGlobalPos[Amg::x]);
            hit.setY(newGlobalPos[Amg::y]);
            hit.setZ(newGlobalPos[Amg::z]);
        }

        // update the hit's local position
        hit.setPhiCoord(hit.getPhiCoord()+shift);
    }
    else{
        // Get the shift value
        if (m_useAthenaLorentzAngleTools) {
            shift = m_lorentzAngleToolStrip->getLorentzShift(hash, Gaudi::Hive::currentContext());
        }
        else {
            shift = getLorentzAngleShift(hit);
        }

        if (m_shiftGlobalPosition) { // TODO: more tests, debugging and validation needed for this in case we actually want it
            // Get the Strip detector element
            const InDetDD::SiDetectorElement* siDE = m_SCTManager->getDetectorElement(hash);
            Identifier wafer_id = m_SCTId->wafer_id(hash);
            Identifier hit_id = m_SCTId->strip_id(wafer_id, static_cast<int>(hit.getPhiIndex()));
            // Get the cell from the ID
            InDetDD::SiCellId cell = siDE->cellIdFromIdentifier(hit_id);
            if (!cell.isValid()) {
                ATH_MSG_DEBUG("SCT cell not valid for hitID " << hit_id);
                return StatusCode::FAILURE;
            }
            // Get the position of the cell
            InDetDD::SiLocalPosition silPos(siDE->rawLocalPositionOfCell(cell));
            // Amg::Vector2D localPos(silPos);
            localPos[Trk::locX] += shift; // apply the Lorentz angle shift

            if (!hit.isBarrel()) {
                const InDetDD::StripStereoAnnulusDesign* design = (static_cast<const InDetDD::StripStereoAnnulusDesign*>(&siDE->design()));
                localPos = design->localPositionOfCellPC(siDE->cellIdOfPosition(localPos));
            }

            // Update the hit's global position
            Amg::Vector3D newGlobalPos = siDE->globalPosition(localPos);
            hit.setX(newGlobalPos[Amg::x]);
            hit.setY(newGlobalPos[Amg::y]);
            hit.setZ(newGlobalPos[Amg::z]);
        }
        // update the hit's local position
        hit.setPhiCoord(hit.getPhiCoord()+shift);
    }
    return StatusCode::SUCCESS;
}


float FPGATrackSim::LorentzAngleTool::getLorentzAngleShift(const FPGATrackSimHit & hit) const {
    if (hit.isPixel()) {
        if(hit.isBarrel()){
            return getPixelBarrelShift(hit.getLayerDisk(true), hit.getEtaModule());
        }
        else{
            return getPixelEndcapShift(hit.getLayerDisk(true), hit.getPhiModule(), hit.getEtaModule());
        }
    } else if (hit.isStrip()) {
        if (hit.isBarrel()) {
            const InDetDD::SiDetectorElement* sielement = m_SCTManager->getDetectorElement(hit.getIdentifierHash());
            return getStripBarrelShift(sielement->isStereo(), hit.getLayerDisk(true), hit.getEtaModule());
        } else {
            return getStripEndcapShift(hit.getLayerDisk(true), hit.getEtaModule(), hit.getZ());
        }
    } else {
        throw std::runtime_error( std::string(typeid(*this).name()) + ": Invalid hit type: neither Pixel nor Strip.");
    }
}


float FPGATrackSim::LorentzAngleTool::getPixelEndcapShift(unsigned layerDisk, unsigned phiModule, int etaModule) const{
    const float endcapShiftTable[23][9] = {
//layers: 0  | 1  | 2  | 3  | 4  | 5  | 6  | 7  | 8
        {0.0, 0.0, -0.000006, 0.004646, -0.000112, 0.006308, -0.000151, 0.006828, -0.000189}, // etaModule 0
        {0.0, 0.0, -0.000006, 0.004627, -0.000129, 0.006291, -0.000185, 0.006811, -0.000226}, // etaModule 1
        {0.0, 0.0, -0.000007, 0.004601, -0.000151, 0.006268, -0.000233, 0.006789, -0.000275}, // etaModule 2
        {0.0, 0.0, -0.000008, 0.004562, -0.000178, 0.006240, -0.000301, 0.006764, -0.000342}, // etaModule 3
        {0.0, 0.0, -0.000009, 0.004503, -0.000212, 0.006202, -0.000394, 0.006731, -0.000432}, // etaModule 4
        {0.0, 0.0, -0.000010, 0.004409, -0.000256, 0.006153, -0.000511, 0.006689, -0.000549}, // etaModule 5
        {0.0, NAN, -0.000011, NAN, -0.000309, 0.006087, -0.000610, 0.006636, -0.000682}, // etaModule 6
        {0.0, NAN, -0.000012, NAN, -0.000370, 0.005995, -0.000607, 0.006569, -0.000781}, // etaModule 7
        {0.0, NAN, -0.000014, NAN, -0.000427, NAN, NAN, 0.006483, -0.000764}, // etaModule 8
        {0.0, NAN, -0.000016, NAN, -0.000459, NAN, NAN, NAN, NAN}, // etaModule 9
        {0.0, NAN, -0.000018, NAN, -0.000454, NAN, NAN, NAN, NAN}, // etaModule 10
        {0.0, NAN, -0.000021, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 11
        {0.0, NAN, -0.000024, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 12
        {0.0, NAN, -0.000028, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 13
        {0.0, NAN, -0.000033, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 14
        {NAN, NAN, -0.000039, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 15
        {NAN, NAN, -0.000047, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 16
        {NAN, NAN, -0.000056, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 17
        {NAN, NAN, -0.000069, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 18
        {NAN, NAN, -0.000085, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 19
        {NAN, NAN, -0.000105, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 20
        {NAN, NAN, -0.000124, NAN, NAN, NAN, NAN, NAN, NAN}, // etaModule 21
        {NAN, NAN, -0.000134, NAN, NAN, NAN, NAN, NAN, NAN}  // etaModule 22
    };
    if (etaModule > 22 || layerDisk > 8) {
        throw std::domain_error("Pixel Endcap invalid layerDisk or etaModule value: " + std::to_string(layerDisk) + ", " + std::to_string(etaModule));
    }
    float shift = endcapShiftTable[etaModule][layerDisk];
    if (std::isnan(shift)) {
        throw std::domain_error("Pixel Endcap invalid shift value: NAN for layerDisk " + std::to_string(layerDisk) + " and etaModule " + std::to_string(etaModule));
    }
    const bool evenPhi = phiModule % 2 == 0;
    switch (layerDisk) {
    case 2:
        return evenPhi ? -shift : shift;
    case 3:
    case 5:
    case 7:
        return evenPhi ? shift : -shift;
    case 4:
        if (phiModule <= 15)
            return evenPhi ? shift : -shift;
        else
            return evenPhi ? -shift : shift;
    case 6:
        if (phiModule <= 21)
            return evenPhi ? shift : -shift;
        else
            return evenPhi ? -shift : shift;
    case 8:
        if (phiModule <= 25)
            return evenPhi ? shift : -shift;
        else
            return evenPhi ? -shift : shift;
    default:
        return shift;
    }
}

float FPGATrackSim::LorentzAngleTool::getPixelBarrelShift(unsigned layerDisk, int etaModule) const {
    const float pixelBarrelShiftTable[25][5] = {
//layers: 0  |  1  |  2  |  3  |  4
        {0.0, NAN, NAN, NAN, NAN}, // etaModule -12
        {0.0, NAN, NAN, NAN, NAN}, // etaModule -11
        {0.0, NAN, NAN, NAN, NAN}, // etaModule -10
        {0.0, NAN, -0.011976, -0.011982, -0.011989}, // etaModule -9
        {0.0, NAN, -0.011989, -0.011995, -0.012002}, // etaModule -8
        {0.0, NAN, -0.011998, -0.012004, -0.012011}, // etaModule -7
        {0.0, -0.006069, -0.012008, -0.012013, -0.012020}, // etaModule -6
        {0.0, -0.006073, -0.012015, -0.012021, -0.012027}, // etaModule -5
        {0.0, -0.006076, -0.012021, -0.012026, -0.012033}, // etaModule -4
        {0.0, -0.006078, -0.012026, -0.012031, -0.012038}, // etaModule -3
        {0.0, -0.006080, -0.012029, -0.012034, -0.012041}, // etaModule -2
        {0.0, -0.006081, -0.012030, -0.012036, -0.012042}, // etaModule -1
        {NAN, NAN, NAN, NAN, NAN}, // etaModule 0
        {0.0, -0.006081, -0.012030, -0.012036, -0.012042}, // etaModule 1
        {0.0, -0.006080, -0.012029, -0.012034, -0.012041}, // etaModule 2
        {0.0, -0.006078, -0.012026, -0.012031, -0.012038}, // etaModule 3
        {0.0, -0.006076, -0.012021, -0.012026, -0.012033}, // etaModule 4
        {0.0, -0.006073, -0.012015, -0.012021, -0.012027}, // etaModule 5
        {0.0, -0.006069, -0.012008, -0.012013, -0.012020}, // etaModule 6
        {0.0, NAN, -0.011998, -0.012004, -0.012011}, // etaModule 7
        {0.0, NAN, -0.011989, -0.011995, -0.012002}, // etaModule 8
        {0.0, NAN, -0.011976, -0.011982, -0.011989}, // etaModule 9
        {0.0, NAN, NAN, NAN, NAN}, // etaModule 10
        {0.0, NAN, NAN, NAN, NAN}, // etaModule 11
        {0.0, NAN, NAN, NAN, NAN}  // etaModule 12
        };

    if (layerDisk >= 5 || std::abs(etaModule) >= 13 || std::isnan(pixelBarrelShiftTable[etaModule + 12][layerDisk])) {
        throw std::domain_error("Invalid pixel barrel layerDisk or etaModule value: " + std::to_string(layerDisk) + ", " + std::to_string(etaModule));
    }

    return pixelBarrelShiftTable[etaModule + 12][layerDisk];
}



float FPGATrackSim::LorentzAngleTool::getStripBarrelShift(bool isStereo, unsigned layerDisk, int etaModule) const {
    const float shiftTable[57][4] = {
//layers: 0  | 1 | 2 | 3
        {NAN, NAN, NAN, NAN}, // Invalid data for etaModule 0
        {0.045516, 0.045631, 0.045813, 0.046054}, // etaModule 1
        {0.045514, 0.045629, 0.045809, 0.046058}, // etaModule 2
        {0.045512, 0.045628, 0.045803, 0.046068}, // etaModule 3
        {0.045506, 0.045623, 0.045788, 0.046069}, // etaModule 4
        {0.045501, 0.045618, 0.045770, 0.046068}, // etaModule 5
        {0.045492, 0.045611, 0.045744, 0.046054}, // etaModule 6
        {0.045481, 0.045600, 0.045715, 0.046040}, // etaModule 7
        {0.045470, 0.045590, 0.045672, 0.046007}, // etaModule 8
        {0.045459, 0.045580, 0.045627, 0.045975}, // etaModule 9
        {0.045442, 0.045563, 0.045572, 0.045932}, // etaModule 10
        {0.045423, 0.045547, 0.045509, 0.045879}, // etaModule 11
        {0.045405, 0.045531, 0.045445, 0.045829}, // etaModule 12
        {0.045387, 0.045514, 0.045361, 0.045756}, // etaModule 13
        {0.045362, 0.045491, 0.045277, 0.045687}, // etaModule 14
        {0.045334, 0.045466, 0.045177, 0.045602}, // etaModule 15
        {0.045308, 0.045441, 0.045072, 0.045514}, // etaModule 16
        {0.045279, 0.045414, 0.044956, 0.045417}, // etaModule 17
        {0.045252, 0.045389, 0.044831, 0.045310}, // etaModule 18
        {0.045217, 0.045357, 0.044694, 0.045193}, // etaModule 19
        {0.045180, 0.045321, 0.044547, 0.045066}, // etaModule 20
        {0.045140, 0.045285, 0.044390, 0.044928}, // etaModule 21
        {0.045102, 0.045250, 0.044219, 0.044775}, // etaModule 22
        {0.045063, 0.045214, 0.044038, 0.044615}, // etaModule 23
        {0.045019, 0.045171, 0.043846, 0.044438}, // etaModule 24
        {0.044968, 0.045123, 0.043635, 0.044247}, // etaModule 25
        {0.044919, 0.045077, 0.043423, 0.044054}, // etaModule 26
        {0.044869, 0.045031, 0.043186, 0.043849}, // etaModule 27
        {0.044821, 0.044985, 0.042944, 0.043647}, // etaModule 28
        {0.044757, 0.044925, NAN, NAN}, // etaModule 29
        {0.044697, 0.044868, NAN, NAN}, // etaModule 30
        {0.044636, 0.044811, NAN, NAN}, // etaModule 31
        {0.044576, 0.044754, NAN, NAN}, // etaModule 32
        {0.044508, 0.044689, NAN, NAN}, // etaModule 33
        {0.044435, 0.044620, NAN, NAN}, // etaModule 34
        {0.044362, 0.044551, NAN, NAN}, // etaModule 35
        {0.044290, 0.044482, NAN, NAN}, // etaModule 36
        {0.044212, 0.044408, NAN, NAN}, // etaModule 37
        {0.044126, 0.044327, NAN, NAN}, // etaModule 38
        {0.044041, 0.044246, NAN, NAN}, // etaModule 39
        {0.043956, 0.044166, NAN, NAN}, // etaModule 40
        {0.043866, 0.044082, NAN, NAN}, // etaModule 41
        {0.043768, 0.043989, NAN, NAN}, // etaModule 42
        {0.043670, 0.043895, NAN, NAN}, // etaModule 43
        {0.043572, 0.043801, NAN, NAN}, // etaModule 44
        {0.043467, 0.043703, NAN, NAN}, // etaModule 45
        {0.043356, 0.043597, NAN, NAN}, // etaModule 46
        {0.043244, 0.043492, NAN, NAN}, // etaModule 47
        {0.043134, 0.043387, NAN, NAN}, // etaModule 48
        {0.043007, 0.043268, NAN, NAN}, // etaModule 49
        {0.042883, 0.043151, NAN, NAN}, // etaModule 50
        {0.042760, 0.043033, NAN, NAN}, // etaModule 51
        {0.042629, 0.042911, NAN, NAN}, // etaModule 52
        {0.042485, 0.042775, NAN, NAN}, // etaModule 53
        {0.042347, 0.042646, NAN, NAN}, // etaModule 54
        {0.042205, 0.042511, NAN, NAN}, // etaModule 55
        {0.042056, 0.042370, NAN, NAN}, // etaModule 56
        };

    if (layerDisk >= 4 || std::abs(etaModule) >= 57 || std::isnan(shiftTable[std::abs(etaModule)][layerDisk]) ) {
        throw std::domain_error("Invalid strip layerDisk or etaModule value in barrel: " + std::to_string(layerDisk) + ", " + std::to_string(etaModule)); 
    }

    return isStereo ? -shiftTable[std::abs(etaModule)][layerDisk] : shiftTable[std::abs(etaModule)][layerDisk];
}

float FPGATrackSim::LorentzAngleTool::getStripEndcapShift(unsigned layerDisk, int etaModule, float z) const {
    double shiftTable[18][6] = {
//layers:  0  | 1  | 2  | 3  | 4  | 5
        {0.001532, 0.001943, 0.002601, 0.003425, 0.004022, 0.003900}, // etaModule 0
        {0.001611, 0.002046, 0.002741, 0.003622, 0.004256, 0.004117}, // etaModule 1
        {0.001706, 0.002165, 0.002916, 0.003855, 0.004553, 0.004404}, // etaModule 2
        {0.001813, 0.002305, 0.003109, 0.004136, 0.004905, 0.004738}, // etaModule 3
        {0.001908, 0.002428, 0.003275, 0.004384, 0.005211, 0.005028}, // etaModule 4
        {0.001990, 0.002529, 0.003421, 0.004591, 0.005480, 0.005278}, // etaModule 5
        {0.002073, 0.002635, 0.003582, 0.004824, 0.005791, 0.005558}, // etaModule 6
        {0.002143, 0.002732, 0.003699, 0.005004, 0.006037, 0.005801}, // etaModule 7
        {0.002217, 0.002832, 0.003855, 0.005236, 0.006343, 0.006094}, // etaModule 8
        {0.002317, 0.002960, 0.004039, 0.005527, 0.006743, 0.006465}, // etaModule 9
        {0.002422, 0.003088, 0.004240, 0.005842, 0.007188, 0.006864}, // etaModule 10
        {0.002510, 0.003202, 0.004413, 0.006121, 0.007592, 0.007247}, // etaModule 11
        {0.002586, 0.003308, 0.004562, 0.006376, 0.007972, 0.007595}, // etaModule 12
        {0.002671, 0.003424, 0.004734, 0.006648, 0.008408, 0.007988}, // etaModule 13
        {0.002788, 0.003581, 0.004976, 0.007081, 0.009110, 0.008616}, // etaModule 14
        {0.002927, 0.003757, 0.005252, 0.007604, 0.010043, 0.009421}, // etaModule 15
        {0.003025, 0.003911, 0.005482, 0.008059, 0.010959, 0.010173}, // etaModule 16
        {0.003139, 0.004051, 0.005707, 0.008498, 0.011985, 0.010993}  // etaModule 17
    };

    if (layerDisk >= 6 || etaModule >= 18 || etaModule<0 || std::isnan(shiftTable[etaModule][layerDisk]) ) {
        throw std::domain_error("Invalid strip layerDisk or etaModule value on endcaps: " + std::to_string(layerDisk) + ", " + std::to_string(etaModule)); 
    }

    return z > 0.0 ? shiftTable[std::abs(etaModule)][layerDisk] : -shiftTable[std::abs(etaModule)][layerDisk];
}
