/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#include <cstdlib>
#include <format>
#include <stdio.h>
#include "TestTools/initGaudi.h"
#include <Identifier/IdentifierHash.h>
#include <xAODMuonViews/FillContainer.h>
#include <xAODMuonViews/ContainerMerge.h>

#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"

std::ostream& operator<<(std::ostream& ostr, const xAOD::MdtDriftCircle& dc) {
    ostr<<"hash: "<<dc.identifierHash()
        <<", layer:"<<static_cast<unsigned>(dc.tubeLayer())
        <<", tube: "<<static_cast<unsigned>(dc.driftTube())
        <<" -> mhash: "<<dc.measurementHash();
    return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const xAOD::MdtDriftCircleContainer& dcCont) {
    std::size_t n{0};
    std::cout<<"Container has "<<dcCont.size()<<" measurements. \n";
    for (const xAOD::MdtDriftCircle* dc : dcCont) {
        std::cout<<" --- "<<(++n)<<" "<<(*dc)<<std::endl;

    }
    return ostr;
}

int main () {
    using Handle_t = xAOD::FillContainer<xAOD::MdtDriftCircleContainer,
                                         xAOD::MdtDriftCircleAuxContainer>;
    
    Handle_t cont1{}, cont2{}, cont3{};
    
    int ret_code = EXIT_SUCCESS;
    const MuonR4::IdentifierSorter sorter{};


    auto isSorted= [&ret_code, &sorter](const xAOD::MdtDriftCircleContainer& cont){
        for (auto itr = cont.begin()+1; itr != cont.end(); ++itr) {
            for (auto itr1 = cont.begin(); itr1 != itr; ++itr1){
                if (!sorter(*itr1, *itr)) {
                    std::cerr<<"Container is unsorted "
                            <<(**itr1)<<" not before "<<(**itr)<<std::endl;
                    ret_code = EXIT_FAILURE;
                }
            }
        }
    };

    auto fillContainer = [&isSorted](Handle_t& cont, std::vector<IdentifierHash>&& chamberHashes){
        for (const IdentifierHash& hash : chamberHashes) {
            const int minLayer = (hash % 3) +1;
            const int maxLayer = 4;
            const int minTube = (hash % 7) + 1;
            const int maxTube = (hash % 11) + minTube;
            for (int tube = minTube; tube <= maxTube; ++ tube){
                for (int lay = minLayer; lay <= maxLayer; ++lay){
                    auto* dc = cont->push_back(std::make_unique<xAOD::MdtDriftCircle>());
                    dc->setMeasurement<1>(hash, xAOD::MeasVector<1>::Zero(), xAOD::MeasMatrix<1>::Identity());
                    dc->setLayer(lay);
                    dc->setTube(tube);
                }
            }
        }
        std::cout<<"Created "<<cont.get()<<" container. "<<(*cont)<<std::endl;
        isSorted(*cont);
    };

    auto isContained=[&ret_code](const xAOD::MdtDriftCircleContainer& primCont,
                                 ConstDataVector<xAOD::MdtDriftCircleContainer>& mergedCont){
        for (const xAOD::MdtDriftCircle* dc : primCont) {
            if (std::find(mergedCont.begin(), mergedCont.end(), dc) == mergedCont.end()){
                std::cerr<<"The measurement "<<(*dc)<<" does not appear in the merged container"<<std::endl;
                ret_code = EXIT_FAILURE;
            }
        }
    };

    fillContainer(cont1, std::vector<IdentifierHash>{1, 3, 5, 7, 9, 11 , 13 ,15 ,17});
    fillContainer(cont2, std::vector<IdentifierHash>{2, 10, 12, 14, 16, 18, 20, 22, 24});

    ConstDataVector<xAOD::MdtDriftCircleContainer> merge{SG::VIEW_ELEMENTS};
    xAOD::mergeContainer(merge, *cont1, *cont2);
    std::cout<<"Container merge is done\n"<<(*merge.asDataVector())<<std::endl;
    isSorted(*merge.asDataVector());
    isContained(*cont1, merge);
    isContained(*cont2, merge);
    

    fillContainer(cont3, std::vector<IdentifierHash>{4,6,8,26,28,30,100});
    xAOD::mergeContainer(merge, *cont3);
    std::cout<<"Third container merge is done\n"<<(*merge.asDataVector())<<std::endl;
    isSorted(*merge.asDataVector());
    isContained(*cont3, merge);
    return ret_code;
 }