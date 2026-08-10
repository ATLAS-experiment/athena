/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HGTD_DetDescrAlgs/PrintHGTDElements.h"
#include "AthenaKernel/IOVInfiniteRange.h"
#include "StoreGate/ReadCondHandle.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "GeoPrimitives/GeoPrimitives.h"

///////////////////////////////////////////////////////////////////

PrintHGTDElements::
PrintHGTDElements(const std::string& name,
                  ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}

///////////////////////////////////////////////////////////////////

StatusCode PrintHGTDElements::initialize()
{
    ATH_MSG_DEBUG("========================================");
    ATH_MSG_DEBUG("PrintHGTDElements initialize()");
    ATH_MSG_DEBUG("========================================");

    //------------------------------------------------------------
    // Initialize ReadCondHandle
    //------------------------------------------------------------
    ATH_CHECK(m_detEleCollKey.initialize());
    ATH_CHECK(m_alignStoreKey.initialize());
    //------------------------------------------------------------
    // Retrieve HGTD identifier helper
    //------------------------------------------------------------
    ATH_CHECK(detStore()->retrieve(m_hgtdId, "HGTD_ID"));

    //------------------------------------------------------------
    // Open output file
    //------------------------------------------------------------
    m_outfile.open(m_outputFile.value());

    if (!m_outfile.is_open()) {

        ATH_MSG_FATAL("Cannot open output file "
                      << m_outputFile.value());

        return StatusCode::FAILURE;
    }

    //------------------------------------------------------------
    // Header
    //------------------------------------------------------------
    m_outfile
        << "# hash"     << ' '
        << "endcap"     << ' '
        << "layer"      << ' '
        << "phi_module" << ' '
        << "eta_module" << ' '

        << "center_x"   << ' '
        << "center_y"   << ' '
        << "center_z"   << ' '

        << "Tx"         << ' '
        << "Ty"         << ' '
        << "Tz"         << ' '

        << "R00"        << ' '
        << "R01"        << ' '
        << "R02"        << ' '

        << "R10"        << ' '
        << "R11"        << ' '
        << "R12"        << ' '

        << "R20"        << ' '
        << "R21"        << ' '
        << "R22"
        << '\n';

    ATH_MSG_INFO("Output file = "
                 << m_outputFile.value());

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

StatusCode PrintHGTDElements::execute(const EventContext& ctx)
{
    //------------------------------------------------------------
    // Execute only once
    //------------------------------------------------------------
    if (!m_firstEvent)
        return StatusCode::SUCCESS;

    m_firstEvent = false;

    //------------------------------------------------------------
    // Retrieve detector elements
    //------------------------------------------------------------
    SG::ReadCondHandle<InDetDD::HGTD_DetectorElementCollection>
        handle(m_detEleCollKey, ctx);

    const auto* elements = handle.retrieve();

    ATH_MSG_DEBUG("DetectorElementCollection pointer = " << elements);
    //ATH_MSG_DEBUG("First element pointer = " << (*elements)[0]);

    if (!handle.isValid() || elements == nullptr) {

        ATH_MSG_FATAL("Cannot retrieve HGTD_DetectorElementCollection");

        return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Number of detector elements = "
                 << elements->size());

    unsigned int nModules = 0;
    
    /////
    SG::ReadCondHandle<GeoAlignmentStore> alignHandle(
        m_alignStoreKey,
        ctx);

    const GeoAlignmentStore* alignStore = *alignHandle;

    if (!alignHandle.isValid() || !alignStore) {
        ATH_MSG_FATAL("Cannot retrieve GeoAlignmentStore");
        return StatusCode::FAILURE;
    }

    //------------------------------------------------------------
    // Loop over detector elements
    //------------------------------------------------------------
    for (const auto* element : *elements) {

        if (!element)
            continue;

        //--------------------------------------------------------
        // Identifier
        //--------------------------------------------------------
        Identifier id = element->identify();

        IdentifierHash hash =
            element->identifyHash();

        int endcap =
            m_hgtdId->endcap(id);

        int layer =
            m_hgtdId->layer(id);

        int phi =
            m_hgtdId->phi_module(id);

        int eta =
            m_hgtdId->eta_module(id);

        //--------------------------------------------------------
        // Geometry
        //--------------------------------------------------------
        const Amg::Vector3D& center = element->center();
        const Amg::Transform3D nominal = element->defTransform();
        const Amg::Transform3D aligned = element->transform();

        const GeoVFullPhysVol* fpv = element->getMaterialGeom();
        const GeoTrf::Transform3D* absPos = alignStore->getAbsPosition(fpv);
        const GeoTrf::Transform3D* defPos = alignStore->getDefAbsPosition(fpv);

        if (defPos) {
            ATH_MSG_INFO("GeoAlignmentStore default position = ("
                        << defPos->translation().x() << ", "
                        << defPos->translation().y() << ", "
                        << defPos->translation().z() << ")");
        }

        if (absPos) {
            ATH_MSG_INFO("FOUND abs position");
            ATH_MSG_INFO("GeoAlignmentStore abs position = ("
                        << absPos->translation().x() << ", "
                        << absPos->translation().y() << ", "
                        << absPos->translation().z() << ")");
        }
        else {
            ATH_MSG_INFO("NO abs position for hash "
                        << hash.value());
        }

        if (!absPos) {
            const auto& geoNominal = fpv->getAbsoluteTransform();

            ATH_MSG_INFO("FALLBACK to GeoModel absolute transform");
            ATH_MSG_INFO("GeoModel absolute transform = ("
                        << geoNominal.translation().x() << ", "
                        << geoNominal.translation().y() << ", "
                        << geoNominal.translation().z() << ")");
            }

        //const Amg::Transform3D& tr = element->moduleTransform();
        const Amg::Transform3D& tr = element->transform();
        const Amg::Vector3D& T = tr.translation();
        const Amg::RotationMatrix3D& R = tr.rotation();

        ATH_MSG_INFO("Hash = " << element->identifyHash());

        ATH_MSG_INFO("Nominal = ("
                    << nominal.translation().x() << ", "
                    << nominal.translation().y() << ", "
                    << nominal.translation().z() << ")");

        ATH_MSG_INFO("Aligned = ("
                    << aligned.translation().x() << ", "
                    << aligned.translation().y() << ", "
                    << aligned.translation().z() << ")");

        if (absPos) {
            ATH_MSG_INFO("DetectorElement - GeoAlignmentStore = ("
                        << aligned.translation().x() - absPos->translation().x()
                        << ", "
                        << aligned.translation().y() - absPos->translation().y()
                        << ", "
                        << aligned.translation().z() - absPos->translation().z()
                        << ")");
        }

        ATH_MSG_INFO("Difference = ("
                    << aligned.translation().x()-nominal.translation().x()
                    << ", "
                    << aligned.translation().y()-nominal.translation().y()
                    << ", "
                    << aligned.translation().z()-nominal.translation().z()
                    << ")");

        ATH_MSG_INFO("CACHE CENTER hash = "
                    << hash.value()
                    << " x=" << center.x()
                    << " y=" << center.y()
                    << " z=" << center.z());

        //--------------------------------------------------------
        // Write output
        //--------------------------------------------------------
        m_outfile
          << hash.value() << " "
          << endcap << " "
          << layer << " "
          << phi << " "
          << eta << " "

          << center.x() << " "
          << center.y() << " "
          << center.z() << " "
          
          << T.x() << " "
          << T.y() << " "
          << T.z() << " "

          << R(0,0) << " "
          << R(0,1) << " "
          << R(0,2) << " "

          << R(1,0) << " "
          << R(1,1) << " "
          << R(1,2) << " "

          << R(2,0) << " "
          << R(2,1) << " "
          << R(2,2)

          << '\n';

        //--------------------------------------------------------
        // Print first few modules
        //--------------------------------------------------------
        if (nModules < 10) {

            ATH_MSG_INFO("hash = "
                         << hash.value()
                         << " layer = " << layer
                         << " phi = " << phi
                         << " eta = " << eta

                         << " center = (" 
                            << center.x() << ", "
                            << center.y() << ", "
                            << center.z() << ")"
                         << " T=("
                            << T.x() << ", "
                            << T.y() << ", "
                            << T.z() << ")");
                        
        }

        ++nModules;
    }

    ATH_MSG_INFO("Processed "
                 << nModules
                 << " HGTD detector elements");

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////

StatusCode PrintHGTDElements::finalize()
{
    ATH_MSG_INFO("========================================");
    ATH_MSG_INFO("PrintHGTDElements finalize()");
    ATH_MSG_INFO("========================================");

    if (m_outfile.is_open())
        m_outfile.close();

    return StatusCode::SUCCESS;
}