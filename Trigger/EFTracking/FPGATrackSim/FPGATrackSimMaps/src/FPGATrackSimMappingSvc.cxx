// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "PathResolver/PathResolver.h"

FPGATrackSimMappingSvc::FPGATrackSimMappingSvc(const std::string& name, ISvcLocator*svc) :
    AthService(name, svc),
    m_EvtSel("FPGATrackSimEventSelectionSvc", name)
{
}


StatusCode FPGATrackSimMappingSvc::queryInterface(const InterfaceID& riid, void** ppvIf)
{
    if (interfaceID() == riid)
        *ppvIf = dynamic_cast< FPGATrackSimMappingSvc* > (this);
    else if (IFPGATrackSimMappingSvc::interfaceID() == riid)
        *ppvIf = dynamic_cast<IFPGATrackSimMappingSvc*> (this);
    else
        return AthService::queryInterface(riid, ppvIf);

    addRef();
    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimMappingSvc::checkInputs()
{
    if (m_pmap_path.value().empty())
        ATH_MSG_FATAL("Main plane map definition missing");
    else if (m_rmap_path.value().empty())
        ATH_MSG_FATAL("Missing region map path");
    else if (m_modulelut_path.value().empty())
        ATH_MSG_FATAL("Module LUT file is missing");
    else
        return StatusCode::SUCCESS;

    return StatusCode::FAILURE;
}


StatusCode FPGATrackSimMappingSvc::checkAllocs()
{
    if (!m_pmap_1st)
        ATH_MSG_FATAL("Error using 1st stage plane map: " << m_pmap_path);
    if (!m_pmap_2nd)
        ATH_MSG_FATAL("Error using 2nd stage plane map: " << m_pmap_path);
    if (!m_rmap_1st)
        ATH_MSG_FATAL("Error creating region map for 1st stage from: " << m_rmap_path);
    if (!m_rmap_2nd)
        ATH_MSG_FATAL("Error creating region map for 2nd stage from: " << m_rmap_path);
    if (!m_subrmap)
        ATH_MSG_FATAL("Error creating sub-region map from: " << m_subrmap_path);

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimMappingSvc::initialize()
{
    ATH_CHECK(m_EvtSel.retrieve());
    ATH_CHECK(checkInputs());

    if (m_mappingType.value() == "FILE")
    {
        ATH_MSG_DEBUG("Creating the 1st stage plane map");
        m_pmap_1st = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(PathResolverFindCalibFile(m_pmap_path.value()), m_EvtSel->getRegionID(), 1, m_layerOverrides));

        ATH_MSG_DEBUG("Creating the 2nd stage plane map");
        m_pmap_2nd = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(PathResolverFindCalibFile(m_pmap_path.value()), m_EvtSel->getRegionID(), 2));

        ATH_MSG_DEBUG("Creating the 1st stage region map");
        m_rmap_1st = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_1st.get(), PathResolverFindCalibFile(m_rmap_path.value())));

        ATH_MSG_DEBUG("Creating the 2nd stage region map");
        m_rmap_2nd = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_2nd.get(), PathResolverFindCalibFile(m_rmap_path.value())));

        ATH_MSG_DEBUG("Creating the sub-region map");
        m_subrmap = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_1st.get(), PathResolverFindCalibFile(m_subrmap_path.value())));

        ATH_MSG_DEBUG("Setting the Modules LUT for Region Maps");
        m_rmap_1st->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));
        m_rmap_2nd->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));
	
	ATH_MSG_DEBUG("Creating NN weighting map");
	if ( ! m_NNmap_path.empty() ) {
	  m_NNmap = std::make_unique<FPGATrackSimNNMap>(PathResolverFindCalibFile(m_NNmap_path.value()));
	} else {
	  m_NNmap = nullptr;
	}
    }

    ATH_CHECK(checkAllocs());

    return StatusCode::SUCCESS;
}


