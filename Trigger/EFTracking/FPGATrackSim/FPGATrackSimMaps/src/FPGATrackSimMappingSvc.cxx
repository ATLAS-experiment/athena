// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "PathResolver/PathResolver.h"

FPGATrackSimMappingSvc::FPGATrackSimMappingSvc(const std::string& name, ISvcLocator*svc) :
    base_class(name, svc),
    m_EvtSel("FPGATrackSimEventSelectionSvc", name)
{
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
    if (m_pmap_vector_1st.empty())
    {
        ATH_MSG_FATAL("Error using 1st stage plane map no elements of vector made: " << m_pmap_vector_1st);
        return StatusCode::FAILURE;
    }
    if (!m_numberOfPmaps){
        ATH_MSG_FATAL("Error with declared number of plane maps: " << m_pmap_path);
        return StatusCode::FAILURE;
    }
    if (m_numberOfPmaps != (m_pmap_vector_1st.size())){
        ATH_MSG_FATAL("Error using number of declared plane maps does not equal number of loaded plane maps: " << m_pmap_path<<"=/="<<m_pmap_vector_1st.size());
        return StatusCode::FAILURE;
    }
    for (size_t a = 0 ; a < m_pmap_vector_1st.size() ;a++)
    {
        if(!m_pmap_vector_1st.at(a)){
            ATH_MSG_FATAL("Error using 1st stage plane map for slice: " << a <<" of "<< m_pmap_vector_1st.size());
            return StatusCode::FAILURE;
        }
    }
    if (m_pmap_vector_2nd.empty())
    {
        ATH_MSG_FATAL("Error using 2nd stage plane map no elements of vector made: " << m_pmap_vector_2nd);
        return StatusCode::FAILURE;
    }
    if (!m_numberOfPmaps){
        ATH_MSG_FATAL("Error with declared number of plane maps: " << m_pmap_path);
        return StatusCode::FAILURE;
    }
    if (m_numberOfPmaps != (m_pmap_vector_2nd.size())){
        ATH_MSG_FATAL("Error using number of declared plane maps does not equal number of loaded plane maps: " << m_pmap_path<<"=/="<<m_pmap_vector_2nd.size());
        return StatusCode::FAILURE;
    }
    for (size_t a = 0 ; a < m_pmap_vector_2nd.size() ;a++)
    {
        if(!m_pmap_vector_2nd.at(a)){
            ATH_MSG_FATAL("Error using 1st stage plane map for slice: " << a <<" of "<< m_pmap_vector_2nd.size());
            return StatusCode::FAILURE;
        }
    }
    if (!m_rmap_1st){
        ATH_MSG_FATAL("Error creating region map for 1st stage from: " << m_rmap_path);
        return StatusCode::FAILURE;
    }
    if (!m_rmap_2nd){
        ATH_MSG_FATAL("Error creating region map for 2nd stage from: " << m_rmap_path);
        return StatusCode::FAILURE;
    }
    if (!m_subrmap){
        ATH_MSG_FATAL("Error creating sub-region map from: " << m_subrmap_path);
        return StatusCode::FAILURE;
    }
    if (!m_subrmap_2nd){
        ATH_MSG_FATAL("Error creating second stage sub-region map from: " << m_subrmap_path);
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}


std::string FPGATrackSimMappingSvc::getFakeNNMapString() const {
    if (m_NNmap_fake != nullptr) {
        return m_NNmap_fake->getNNMap();
    }
    else{
        return ""; // Handle null case appropriately
    }
}

std::string FPGATrackSimMappingSvc::getParamNNMapString() const {
    if (m_NNmap_param != nullptr) {
        return m_NNmap_param->getNNMap();
    }
    else{
        return ""; // Handle null case appropriately
    }
}


int FPGATrackSimMappingSvc::countPmapSize(std::ifstream& fileIn)
{
    std::string line;

    getline(fileIn, line);
    std::istringstream sline(line);
    std::string geoKeyCheck;
    sline >> geoKeyCheck;
    m_numberOfPmaps = 1;
    while (getline(fileIn,line)){
        std::istringstream sline(line);
        std::string geoKeyCandidate;
        sline >> geoKeyCandidate;
        if(geoKeyCheck.compare(geoKeyCandidate)==0){
            m_numberOfPmaps++;
        }
    }
    return m_numberOfPmaps;
}
StatusCode FPGATrackSimMappingSvc::initialize()
{
    ATH_CHECK(m_EvtSel.retrieve());
    ATH_CHECK(checkInputs());

    if (m_mappingType.value() == "FILE")
    {
        const std::string & filepath = PathResolverFindCalibFile(m_pmap_path.value());
        std::ifstream fin(filepath);
        if (!fin.is_open())
        {
            ATH_MSG_DEBUG("Couldn't open " << filepath);
            throw ("FPGATrackSimPlaneMap Couldn't open " + filepath);
        }
        
        countPmapSize(fin);
        fin.close();
        fin.open(filepath);
        ATH_MSG_DEBUG("Creating the 1st stage plane map");
        for (size_t i = 0; i<m_numberOfPmaps; i++)
        {
            m_pmap_vector_1st.emplace_back(std::make_unique<FPGATrackSimPlaneMap>(fin, m_EvtSel->getRegionID(), 1, m_layerOverrides));
        }
        
        fin.close();
        fin.open(filepath);
        ATH_MSG_DEBUG("Creating the 2nd stage plane map");
        for (size_t i = 0; i<m_numberOfPmaps; i++)
        {
            m_pmap_vector_2nd.emplace_back(std::make_unique<FPGATrackSimPlaneMap>(fin, m_EvtSel->getRegionID(), 2, m_layerOverrides));
        }
        fin.close();

        ATH_MSG_DEBUG("Creating the 1st stage region map");
        m_rmap_1st = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_vector_1st, PathResolverFindCalibFile(m_rmap_path.value())));

        ATH_MSG_DEBUG("Creating the 2nd stage region map");
        m_rmap_2nd = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_vector_2nd, PathResolverFindCalibFile(m_rmap_path.value())));

        ATH_MSG_DEBUG("Creating the sub-region map");
        m_subrmap = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_vector_1st, PathResolverFindCalibFile(m_subrmap_path.value())));

        ATH_MSG_DEBUG("Creating the 2nd stage sub-region map");
        m_subrmap_2nd = std::make_unique<FPGATrackSimRegionMap>(m_pmap_vector_2nd, PathResolverFindCalibFile(m_subrmap_path.value()));

        ATH_MSG_DEBUG("Setting the Modules LUT for Region Maps");
        m_rmap_1st->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));
        m_rmap_2nd->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));

        // We probably need two versions of this path for the second stage.
        ATH_MSG_DEBUG("Setting the average radius per logical layer for Region and Subregion Maps");
        m_rmap_1st->loadRadiiFile(PathResolverFindCalibFile(m_radii_path.value()));
        m_rmap_2nd->loadRadiiFile(PathResolverFindCalibFile(m_radii_path.value()));	
        m_subrmap->loadRadiiFile(PathResolverFindCalibFile(m_radii_path.value()));
	
        ATH_MSG_DEBUG("Creating NN weighting map");
        ATH_MSG_INFO("MappingSVc using " << m_NNmap_path_fake.value() << " and " << m_NNmap_path_param.value());
        if ( ! m_NNmap_path_fake.empty() ) {
            m_NNmap_fake = std::make_unique<FPGATrackSimNNMap>(PathResolverFindCalibFile(m_NNmap_path_fake.value()));
        } else {
            m_NNmap_fake = nullptr;
        }

        if ( ! m_NNmap_path_param.empty() ) {
            m_NNmap_param = std::make_unique<FPGATrackSimNNMap>(PathResolverFindCalibFile(m_NNmap_path_param.value()));
        } else {
            m_NNmap_param = nullptr;
        }
    }
    ATH_CHECK(checkAllocs());
    return StatusCode::SUCCESS;
}


