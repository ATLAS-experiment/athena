// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRoadUnionTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date November 20th, 2020
 * @brief See header file.
 */


#include "FPGATrackSimRoadUnionTool.h"
//one of the includes are needed below or all of them dont know for sure
//TODO WW
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHoughTransformTool.h"

#include <sstream>
#include <cmath>
#include <algorithm>

FPGATrackSimRoadUnionTool::FPGATrackSimRoadUnionTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc),
    m_tools(this)
{
    declareInterface<IFPGATrackSimRoadFinderTool>(this);
    declareProperty("tools", m_tools, "Array of FPGATrackSimRoadFinderTools");
}


StatusCode FPGATrackSimRoadUnionTool::initialize()
{
    // Retrieve
    ATH_MSG_INFO("Using " << m_tools.size() << " tools");
    ATH_CHECK(m_tools.retrieve());

    if (m_tools.empty()) {
      ATH_MSG_FATAL("initialize() Tool list empty");
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimRoadUnionTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) 
{
    
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    //find and make vector of hists associated with layer
    std::vector<std::vector<const FPGATrackSimHit*>> sliceHits(m_tools.size());
    const FPGATrackSimPlaneMap *pmap = nullptr;
    //std::cout<<"~~~~~~~~~~~~~~~~~~~~~"<<m_tools.size()<<"~~~~~~ \n";
    int toolNum = 0;//same as sliceNum
    for (auto & tool : m_tools)
    {
        pmap = m_FPGATrackSimMapping->PlaneMap_1st(toolNum);
        auto* subrmap = m_FPGATrackSimMapping->SubRegionMap();
        for (auto & iHit:hits)
        {
            //std::cout<<"****testSR"<<tool->getSubRegion()<<"\n";
            //if (tool->getSubRegion() >= 0 && !m_FPGATrackSimMapping->SubRegionMap()->isInRegion(tool->getSubRegion(), *iHit)) continue;
            ATH_MSG_INFO("PRE INSERT SliceNum"<<tool->getSubRegion()<<"  NumberOfHits"<<sliceHits[toolNum].size() );


            FPGATrackSimHit* hitCopy = new FPGATrackSimHit(*iHit);
            ATH_MSG_INFO("hitCopyLL_PRE  :"<<hitCopy->getLayer());
            pmap->map(*hitCopy);
            ATH_MSG_INFO("hitCopyLL_AFTER:"<<hitCopy->getLayer());
            if (hitCopy->getLayer()>=0)
            {
                if ((subrmap->isInRegion(tool->getSubRegion(), *hitCopy))) {
                    sliceHits[toolNum].push_back(hitCopy);
                }
            }
            else{
                delete hitCopy;
            }

            /*
            if ((subrmap->isInRegion(tool->getSubRegion(), *iHit))) {
                FPGATrackSimHit* hitCopy = new FPGATrackSimHit(*iHit);
                ATH_MSG_INFO("hitCopyLL_PRE  :"<<hitCopy->getLayer());
                pmap->map(*hitCopy);
                ATH_MSG_INFO("hitCopyLL_AFTER:"<<hitCopy->getLayer());
                if (hitCopy->getLayer()>=0)
                {
                    sliceHits[toolNum].push_back(hitCopy);
                }
                else{
                    delete hitCopy;
                }
//                continue;//TODO WHY IS THIS HERE WW
            }
            */
            ATH_MSG_INFO("~PRE2 INSERT SliceNum"<<tool->getSubRegion()<<"  NumberOfHits"<<sliceHits[toolNum].size() );
        }   
        toolNum++;  
    }
    roads.clear();
    std::cout<<m_tools.size()<<"\n";
    for (auto & tool : m_tools)
    {
       // std::cout<<"SUBR:"<<tool->getSubRegion()<<"\n";
        //std::vector<FPGATrackSimRoad*> r;
        std::vector<std::shared_ptr<const FPGATrackSimRoad>> r;
//        std::cout<<"slice"<<tool->getSubRegion()<<" hitNum:"<<sliceHits[tool->getSubRegion()].size()<<"\n";
        ATH_CHECK(tool->getRoads(sliceHits[tool->getSubRegion()], r));
        //ATH_CHECK(tool->getRoads(hits, r));
        ATH_MSG_INFO("SliceNum"<<tool->getSubRegion()<<"  NumberOfHits"<<sliceHits[tool->getSubRegion()].size() );
        roads.insert(roads.end(), std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()));
    }
    /*
    // WW need to make p map call fore region and pass filter hits 
    std::vector<std::vector<const FPGATrackSimHit*>> sliceHits_filter(m_tools.size());
    const FPGATrackSimPlaneMap *pmap = nullptr;

    for (int iLayer = 0; iLayer<sliceHits.size(); iLayer++){
        pmap = m_FPGATrackSimMapping->PlaneMap_1st(iLayer);
        std::cout<<"old_slice"<<iLayer<<" hitNum:"<<sliceHits.at(iLayer).size()<<"\n";

        for (int iHit = 0; iHit<sliceHits.at(iLayer).size(); iHit++){
            //check this?
            std::cout<<"TorF:"<<!(sliceHits.at(iLayer).at(iHit)->isMapped())<<'\n';
            if ((sliceHits.at(iLayer).at(iHit)->isMapped())){
                sliceHits_filter.at(iLayer).push_back(new FPGATrackSimHit(*sliceHits.at(iLayer).at(iHit)));
                FPGATrackSimHit tempHitCopy = *sliceHits.at(iLayer).at(iHit);
                pmap->map(tempHitCopy);//TODO there has to be a better way of doing this 
            }
            //std::cout<<iLayer<<sliceHits.size()<<sliceHits.at(iLayer).size();

        }
    }
    
    roads.clear();
    std::cout<<m_tools.size()<<"\n";
    for (auto & tool : m_tools)
    {
       // std::cout<<"SUBR:"<<tool->getSubRegion()<<"\n";
        std::vector<FPGATrackSimRoad*> r;
        std::cout<<"slice"<<tool->getSubRegion()<<" hitNum:"<<sliceHits_filter[tool->getSubRegion()].size()<<"\n";
        ATH_CHECK(tool->getRoads(sliceHits_filter[tool->getSubRegion()], r));
        //ATH_CHECK(tool->getRoads(hits, r));
        roads.insert(roads.end(), std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()));
    }
    */
    //TODO WW DELETE PRE FILT HIT LIST 

    return StatusCode::SUCCESS;
}
