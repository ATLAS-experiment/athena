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
    if (!(m_pmap_vector_1st.size()>0))
    {
        ATH_MSG_FATAL("Error using 1st stage plane map no elements of vector made: " << m_pmap_vector_1st);
    }
    if (!m_numberOfPmaps)
        ATH_MSG_FATAL("Error with declared number of plane maps: " << m_pmap_path);
    if (m_numberOfPmaps != m_pmap_vector_1st.size())
        ATH_MSG_FATAL("Error using number of declared plane maps does not equal number of loaded plane maps: " << m_pmap_path<<"=/="<<m_pmap_vector_1st.size());
    for (int a = 0 ; a < m_pmap_vector_1st.size() ;a++)
    {
        if(!m_pmap_vector_1st.at(a))
            ATH_MSG_FATAL("Error using 1st stage plane map for slice: " << a <<" of "<< m_pmap_vector_1st.size());
    }
    //if (!m_pmap_1st)
    //    ATH_MSG_FATAL("Error using 1st stage plane map: " << m_pmap_path);
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

int FPGATrackSimMappingSvc::readPmapSize(std::ifstream& fileIn)
{
    //int numberOfPmaps;  
    std::string line;
    std::cout<<"~~PMAP"<<'\n';
    //const std::string & filepath, 
    //const std::string &  filepath = "/home/wcas/pmap.config";
        //int readPmapSize();
    /*
    std::ifstream fileIn(filepath);

    if (!fileIn.is_open())
    {
        //ANA_MSG_FATAL("Couldun't open " << filepath);
        std::cout<<"THE FILE:"<<filepath<<'\n';
        std::cout<<"Couldn't open \n";
        throw ("FPGATrackSimPlaneMap Couldn't open " + filepath);
    }
    //TODO KILL cout 
    std::cout<<"~~~~FILE PAHT INPIT pmap"<<filepath<<'\n';
    int nHeaderLines = 9;
    for (int i=0; i< nHeaderLines; i++){
        getline(fileIn, line);
    }
    */
    getline(fileIn, line);
    std::istringstream sline(line);
    sline >> m_numberOfPmaps;
    std::cout<<"\n ~~~NUM"<< m_numberOfPmaps<<'\n';
    if ( !(m_numberOfPmaps>0) ){
        std::cout<<"\n ~~IN~i div seekmi ti "<< m_numberOfPmaps<<'\n';
        ATH_MSG_FATAL("Number of Pmaps is set to" << m_numberOfPmaps);
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
        std::cout<<"\n \n TST FILE PATH"<<filepath<<'\n';
        if (!fin.is_open())
        {
            //ANA_MSG_FATAL("Couldn't open " << filepath);
            throw ("FPGATrackSimPlaneMap Couldn't open " + filepath);
        }
        //int pmapNumber = readPmapSize(fin);
        readPmapSize(fin);
        //TODO KILL USELESS COMMENTS
        //readTest(fin);
        //readTest(fin);
        //fin.seekg(0);
        //std::getline(fin, "14")
        //readTest(fin);
        //readTest(fin);
        //fin.close();m_pmap_vector_1st
        //std::vector<std::unique_ptr<FPGATrackSimPlaneMap>> planeMapVector;
        //std::vector<std::unique_ptr<FPGATrackSimPlaneMap>> m_pmap_vector_1st;

        ATH_MSG_DEBUG("Creating the 1st stage plane map");
        std::cout<<"\n line 131 \n";
        //m_pmap_1st = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(PathResolverFindCalibFile(m_pmap_path.value()), m_EvtSel->getRegionID(), 1, m_layerOverrides));
        for (int i = 0; i<m_numberOfPmaps; i++)
        {
            std::cout<<"\n test \n";
            m_pmap_vector_1st.push_back(std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(fin, m_EvtSel->getRegionID(), 1, m_layerOverrides)));
            std::cout<<"\n test DONE \n";
        }
        //std::unique_ptr<FPGATrackSimPlaneMap> m_pmap_1st = std::move(planeMapVector[0]);
        //m_pmap_1st = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(fin, m_EvtSel->getRegionID(), 1, m_layerOverrides));
        std::cout<<"\n line 133 \n";
        std::cout<<"\n line 134 \n";
        //fin.seekg(0);
        //fin.close();
        //fin.open(filepath);
        //readPmapSize(fin);
        //m_pmap_1st = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(fin, m_EvtSel->getRegionID(), 1, m_layerOverrides));
        std::cout<<"\n line 137 \n";
        fin.close();
        fin.open(filepath);
        readPmapSize(fin);
        
        ATH_MSG_DEBUG("Creating the 2nd stage plane map");
        //m_pmap_2nd = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(PathResolverFindCalibFile(m_pmap_path.value()), m_EvtSel->getRegionID(), 2));
        m_pmap_2nd = std::unique_ptr<FPGATrackSimPlaneMap>(new FPGATrackSimPlaneMap(fin, m_EvtSel->getRegionID(), 2));
        fin.close();
        std::cout<<"\n line 152 \n";

        ATH_MSG_DEBUG("Creating the 1st stage region map");
        //m_rmap_1st = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_1st.get(), PathResolverFindCalibFile(m_rmap_path.value())));
        m_rmap_1st = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_vector_1st.at(0).get(), PathResolverFindCalibFile(m_rmap_path.value())));

        std::cout<<"\n line 157 \n";
        ATH_MSG_DEBUG("Creating the 2nd stage region map");
        m_rmap_2nd = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_2nd.get(), PathResolverFindCalibFile(m_rmap_path.value())));

        std::cout<<"\n line 161 \n";
        ATH_MSG_DEBUG("Creating the sub-region map");
        //m_subrmap = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_1st.get(), PathResolverFindCalibFile(m_subrmap_path.value())));
        m_subrmap = std::unique_ptr<FPGATrackSimRegionMap>(new FPGATrackSimRegionMap(m_pmap_vector_1st.at(0).get(), PathResolverFindCalibFile(m_subrmap_path.value())));

        std::cout<<"\n line 165 \n";
        ATH_MSG_DEBUG("Setting the Modules LUT for Region Maps");
        m_rmap_1st->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));
        m_rmap_2nd->loadModuleIDLUT(PathResolverFindCalibFile(m_modulelut_path.value()));

        std::cout<<"\n line 171 \n";
        // We probably need two versions of this path for the second stage.
        ATH_MSG_DEBUG("Setting the average radius per logical layer for Region and Subregion Maps");
        m_rmap_1st->loadRadiiFile(PathResolverFindCalibFile(m_radii_path.value()));
        m_subrmap->loadRadiiFile(PathResolverFindCalibFile(m_radii_path.value()));
	
        std::cout<<"\n line 178 \n";
	ATH_MSG_DEBUG("Creating NN weighting map");
    std::cout<<"\n line 180 \n";
	if ( ! m_NNmap_path.empty() ) {
	  m_NNmap = std::make_unique<FPGATrackSimNNMap>(PathResolverFindCalibFile(m_NNmap_path.value()));
	} else {
	  m_NNmap = nullptr;
	}
    }
    std::cout<<"\n line 187 \n";
    ATH_CHECK(checkAllocs());
    std::cout<<"\n line 189 \n"<<"---"<<StatusCode::SUCCESS;
    return StatusCode::SUCCESS;
}


