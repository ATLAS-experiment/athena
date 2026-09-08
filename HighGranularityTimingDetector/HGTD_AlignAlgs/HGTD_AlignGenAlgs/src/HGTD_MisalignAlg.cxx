/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_MisalignAlg.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "HGTD_AlignGenTools/IHGTD_AlignDBTool.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "AthenaKernel/IOVTime.h"

#include <fstream>

StatusCode HGTD_MisalignAlg::initialize()
{
    ATH_MSG_INFO("=== HGTD_MisalignAlg initialize ===");

    //------------------------------------------------------------
    // Retrieve services
    //------------------------------------------------------------
    ATH_CHECK(m_rndmSvc.retrieve());
    ATH_CHECK(m_alignDBTool.retrieve());

    ATH_MSG_DEBUG("Retrieved HGTD_AlignDBTool");
    ATH_MSG_DEBUG("AlignDBTool = " << m_alignDBTool.typeAndName());
    ATH_MSG_DEBUG("WriteToDB   = " << m_writeToDB);
    ATH_MSG_DEBUG("SQLiteTag   = " << m_sqliteTag);
    ATH_MSG_DEBUG("RandomStream = " << m_randomStream);

    //------------------------------------------------------------
    // Retrieve HGTD identifier helper
    //------------------------------------------------------------
    ATH_CHECK(detStore()->retrieve(m_hgtdIdHelper,"HGTD_ID"));

    //------------------------------------------------------------
    // Retrieve HGTD detector manager
    //------------------------------------------------------------
    ATH_CHECK(detStore()->retrieve(m_hgtdManager, "HGTD"));
    ATH_MSG_DEBUG("Retrieved HGTD_DetectorManager");

    //------------------------------------------------------------
    // Open output file
    //------------------------------------------------------------
    m_outfile.open(m_outputFile.value());

    if (!m_outfile.is_open()) {

        ATH_MSG_ERROR(
            "Cannot open output file "
            << m_outputFile.value());

        return StatusCode::FAILURE;
    }
    ATH_MSG_DEBUG("Output file = " << m_outputFile.value());

    //------------------------------------------------------------
    // ASCII validation output
    //------------------------------------------------------------
    m_outfile
        << "# hash "
        << "endcap "
        << "layer "
        << "phi_module "
        << "eta_module "
        << "local_dx "
        << "local_dy "
        << "local_dz "
        << "center_x "
        << "center_y "
        << "center_z "
        << "misaligned_x "
        << "misaligned_y "
        << "misaligned_z\n";

    SmartIF<ITHistSvc> histSvc{
        Gaudi::svcLocator()->service("THistSvc")};
    ATH_CHECK(histSvc.isValid());

    m_tree = new TTree("HGTDMisalignment", "HGTD alignment validation");

    ATH_CHECK(
        histSvc->regTree("/CREATEMISALIGN/HGTDMisalignment", m_tree));

    //------------------------------------------------------------
    // Validation tree branches
    // These quantities are used to validate the generated
    // misalignment and compare the nominal and shifted geometry.
    //------------------------------------------------------------

    m_tree->Branch("hash", &m_hash, "hash/i");
    
    // HGTD detector identifier
    m_tree->Branch("endcap", &m_endcap, "endcap/I");
    m_tree->Branch("layer", &m_layer, "layer/I");
    m_tree->Branch("phi_module", &m_phiModule, "phi_module/I");
    m_tree->Branch("eta_module", &m_etaModule, "eta_module/I");

    // Applied local misalignment
    m_tree->Branch("local_dx", &m_localDx, "local_dx/F");
    m_tree->Branch("local_dy", &m_localDy, "local_dy/F");
    m_tree->Branch("local_dz", &m_localDz, "local_dz/F");

    // Nominal global center
    m_tree->Branch("center_x", &m_centerX, "center_x/F");
    m_tree->Branch("center_y", &m_centerY, "center_y/F");
    m_tree->Branch("center_z", &m_centerZ, "center_z/F");

    // Misaligned global center
    m_tree->Branch("misaligned_x", &m_shiftedX, "misaligned_x/F");
    m_tree->Branch("misaligned_y", &m_shiftedY, "misaligned_y/F");
    m_tree->Branch("misaligned_z", &m_shiftedZ, "misaligned_z/F");

    //------------------------------------------------------------
    // Print configuration
    //------------------------------------------------------------
    ATH_MSG_DEBUG("MisalignMode      = " << m_mode.value());
    ATH_MSG_DEBUG("ShiftX            = " << m_shiftX.value());
    ATH_MSG_DEBUG("ShiftY            = " << m_shiftY.value());
    ATH_MSG_DEBUG("ShiftZ            = " << m_shiftZ.value());
    ATH_MSG_DEBUG("SigmaX            = " << m_sigmaX.value());
    ATH_MSG_DEBUG("SigmaY            = " << m_sigmaY.value());
    ATH_MSG_DEBUG("SigmaZ            = " << m_sigmaZ.value());
    ATH_MSG_DEBUG("ApplyTranslation  = " << m_applyTranslation.value());

    return StatusCode::SUCCESS;
}

StatusCode HGTD_MisalignAlg::execute(const EventContext&)
{
    ATH_MSG_DEBUG("HGTD_MisalignAlg execute()");

    ++m_nEvents;

    //------------------------------------------------------------
    // Nothing to do if misalignment is disabled
    //------------------------------------------------------------
    if (!m_applyTranslation.value()) {
        ATH_MSG_DEBUG("Translation disabled");
        return StatusCode::SUCCESS;
    }

    //------------------------------------------------------------
    // Generate the alignment only once
    //------------------------------------------------------------
    if (!m_firstEvent) {
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Generating HGTD misalignment");

    //------------------------------------------------------------
    // Create a fresh alignment database
    //------------------------------------------------------------
    if (m_createFreshDB) {

        ATH_MSG_DEBUG("Creating HGTD alignment database");

        ATH_CHECK(m_alignDBTool->createDB());

        m_createFreshDB = false;
    }

    //------------------------------------------------------------
    // Generate all module misalignments
    //------------------------------------------------------------
    ATH_CHECK(GenerateMisalignment());

    //------------------------------------------------------------
    // Write AlignableTransforms
    //------------------------------------------------------------
    ATH_MSG_DEBUG("Writing HGTD alignment constants");

    if (m_writeToDB) {
        ATH_MSG_DEBUG("Calling outputObjs()");
        ATH_CHECK(m_alignDBTool->outputObjs());

        ATH_MSG_DEBUG("Finished outputObjs()");
        ATH_MSG_DEBUG("Calling fillDB()");

    //------------------------------------------------------------
    // Write SQLite database
    //------------------------------------------------------------
        ATH_CHECK(
            m_alignDBTool->fillDB(
                m_sqliteTag,
                IOVTime::MINRUN,
                IOVTime::MINEVENT,
                IOVTime::MAXRUN,
                IOVTime::MAXEVENT));
        
        ATH_MSG_DEBUG("Finished fillDB()");
    }
    else {
        ATH_MSG_DEBUG("WriteToDB = FALSE, skipping database writing.");
    }

    //------------------------------------------------------------
    // Do not regenerate on later events
    //------------------------------------------------------------
    m_firstEvent = false;

    return StatusCode::SUCCESS;
}

StatusCode HGTD_MisalignAlg::GenerateMisalignment()
{
   const auto* elements = m_hgtdManager->getDetectorElementCollection();

    if (!elements) {
        ATH_MSG_ERROR("HGTD detector element collection is null");
        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Number of HGTD detector elements = "
                 << elements->size());

    ATH_MSG_DEBUG("Misalignment mode = "
                 << m_mode.value());

    //------------------------------------------------------------
    // Random generators
    //------------------------------------------------------------
    Rndm::Numbers gaussX(
        m_rndmSvc.operator->(),
        Rndm::Gauss(0., m_sigmaX.value()));

    Rndm::Numbers gaussY(
        m_rndmSvc.operator->(),
        Rndm::Gauss(0., m_sigmaY.value()));

    Rndm::Numbers gaussZ(
        m_rndmSvc.operator->(),
        Rndm::Gauss(0., m_sigmaZ.value()));

    int count = 0;

    //------------------------------------------------------------
    // Loop over all HGTD detector elements
    //------------------------------------------------------------
    for (const auto* element : *elements) {

        if (!element)
            continue;

        Identifier id = element->identify();
        IdentifierHash hash = element->identifyHash();

        //--------------------------------------------------------
        // Detector identifier information
        //--------------------------------------------------------
        m_hash = hash.value();
        m_endcap = m_hgtdIdHelper->endcap(id);
        m_layer = m_hgtdIdHelper->layer(id);
        m_phiModule = m_hgtdIdHelper->phi_module(id);
        m_etaModule = m_hgtdIdHelper->eta_module(id);

        //--------------------------------------------------------
        // Misalignment parameters
        //--------------------------------------------------------
        double dx = 0.0;
        double dy = 0.0;
        double dz = 0.0;

        if (m_mode.value() == 0) {
            // no misalignment
        }
        else if (m_mode.value() == 1) {
            // Constant translation
            dx = m_shiftX.value();
            dy = m_shiftY.value();
            dz = m_shiftZ.value();
        }
        else if (m_mode.value() == 2) {
            // Random Gaussian translation
            dx = gaussX();
            dy = gaussY();
            dz = gaussZ();
        }
        else {
            ATH_MSG_WARNING("Unknown MisalignMode = "
                    << m_mode.value()
                    << ". No misalignment applied.");
            continue;
        }
        //--------------------------------------------------------
        // Update alignment constants
        //--------------------------------------------------------
        bool ok =
            m_alignDBTool->tweakTrans(
                id,
                1,
                Amg::Vector3D(dx, dy, dz),
                0.,
                0.,
                0.);

        if (!ok) {

            ATH_MSG_WARNING(
                "Failed to update module "
                << hash);

            continue;
        }

        //--------------------------------------------------------
        // Fill validation tree
        // For the current implementation only translations are
        // applied, therefore the shifted position is simply
        // nominal position + translation.
        //--------------------------------------------------------
        
        m_localDx = dx;
        m_localDy = dy;
        m_localDz = dz;

        //--------------------------------------------------------
        // Nominal module position
        //--------------------------------------------------------
        const Amg::Vector3D& center = element->center();
        
        m_centerX = center.x();
        m_centerY = center.y();
        m_centerZ = center.z();

        m_shiftedX = m_centerX + m_localDx;
        m_shiftedY = m_centerY + m_localDy;
        m_shiftedZ = m_centerZ + m_localDz;

        //--------------------------------------------------------
        // Print first few modules
        //--------------------------------------------------------
        if (count < 10) {

            ATH_MSG_DEBUG(
                "Module "
                << hash
                << " -> shift ("
                << dx << ", "
                << dy << ", "
                << dz << ")");
        }

        //--------------------------------------------------------
        // Save to text file
        //--------------------------------------------------------
        m_outfile
        << m_hash << " "
        << m_endcap << " "
        << m_layer << " "
        << m_phiModule << " "
        << m_etaModule << " "
        << m_localDx << " "
        << m_localDy << " "
        << m_localDz << " "
        << m_centerX << " "
        << m_centerY << " "
        << m_centerZ << " "
        << m_shiftedX << " "
        << m_shiftedY << " "
        << m_shiftedZ
        << '\n';

        m_tree->Fill();

        ++count;
    }

    ATH_MSG_DEBUG(
        "Processed "
        << count
        << " HGTD modules");

    return StatusCode::SUCCESS;
}

StatusCode HGTD_MisalignAlg::finalize()
{
    ATH_MSG_INFO("HGTD_MisalignAlg finalize()");

    //------------------------------------------------------------
    // Close output file
    //------------------------------------------------------------
    if (m_outfile.is_open()) {
        m_outfile.close();
    }
     
    ATH_CHECK(m_alignDBTool->outputObjs());
    
    ATH_CHECK(
        m_alignDBTool->fillDB(
            m_sqliteTag,
            IOVTime::MINRUN,
            IOVTime::MINEVENT,
            IOVTime::MAXRUN,
            IOVTime::MAXEVENT
        )
    );

    ATH_MSG_INFO("Processed " << m_nEvents << " event(s)");

    return StatusCode::SUCCESS;
}
