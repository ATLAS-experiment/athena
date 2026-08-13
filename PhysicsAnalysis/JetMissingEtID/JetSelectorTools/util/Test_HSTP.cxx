#include <memory>

#include "TFile.h"

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

#include "xAODJet/JetContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include "JetSelectorTools/EventCleaningTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetSelectorTools/Helpers.h"

#include "AsgTools/AnaToolHandle.h"
#include "PMGAnalysisInterfaces/IPMGCrossSectionTool.h"
#include "PathResolver/PathResolver.h"




int main(int /*argc*/, char* argv[])
{

    auto tool = std::make_unique<ECUtils::EventCleaningTool>("EventCleaningTool");

    const bool doHSTPFiltering = true;
    const std::string jetContainerName = "AntiKt4TruthDressedWZJets";
    const std::string jetPUContainerName = "InTimeAntiKt4TruthJets";

    asg::AnaToolHandle<PMGTools::IPMGCrossSectionTool> pmgTool(
        "PMGTools::PMGCrossSectionTool/PMGCrossSectionTool");

    if (pmgTool.retrieve().isFailure()) {
        std::cerr << "Failed to retrieve PMGCrossSectionTool" << std::endl;
        return 1;
    }

    pmgTool->readInfosFromFiles(
        {PathResolverFindCalibFile("dev/PMGTools/PMGxsecDB_mc21.txt")});

    if (tool->setProperty("doHSTPFiltering", doHSTPFiltering).isFailure()) {
        std::cerr << "Failed to set filterHSTP" << std::endl;
        return 1;
    }

    if (tool->setProperty("JetContainer", jetContainerName).isFailure()) {
        std::cerr << "Failed to set JetContainer" << std::endl;
        return 2;
    }

    if (tool->initialize().isFailure()) {
        std::cerr << "Failed to initialize the tool" << std::endl;
        return 4;
    }

    if (!xAOD::Init().isSuccess()) {
        std::cerr << "Failed to initialize xAOD" << std::endl;
        return 1;
    }

    xAOD::TEvent event(xAOD::TEvent::kClassAccess);

    std::unique_ptr<TFile> inputFile(TFile::Open(argv[1], "READ"));
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Failed to open input file" << std::endl;
        return 1;
    }

    if (event.readFrom(inputFile.get()).isFailure()) {
        std::cerr << "Failed to read input file" << std::endl;
        return 1;
    }

    const Long64_t nEntries = event.getEntries();

    for (Long64_t entry = 0;
        entry < std::min(nEntries, Long64_t{100});
        ++entry) {

        if (event.getEntry(entry) < 0) {
            break;
        }

        const xAOD::EventInfo* eventInfo = nullptr;
        if (!event.retrieve(eventInfo, "EventInfo").isSuccess()) {
            return 5;
        }

        bool needHSTP = false;

        const int dsid = eventInfo->mcChannelNumber();
        const std::string sampleName = pmgTool->getSampleName(dsid);

        std::cout << sampleName;

        if (const char* p = strstr(sampleName.c_str(), "JZ")) {
            needHSTP = (p[2] >= '0' && p[2] <= '9');
        }

        std::cout << needHSTP << std::endl;

        const xAOD::JetContainer* jets = nullptr;
        if (!event.retrieve(jets, jetContainerName).isSuccess()) {
            return 6;
        }

        const xAOD::JetContainer* puJets = nullptr;
        if (!event.retrieve(puJets, jetPUContainerName).isSuccess()) {
            return 7;
        }

        const bool passFilter = tool->passHSTPFilter(jets, puJets);

        if (jets->empty()) {
            continue;
        }

        std::cout << "Run " << eventInfo->runNumber()
                  << ", Event " << eventInfo->eventNumber()
                  << ", Njets = " << jets->size()
                  << ", NPUjets = " << puJets->size()
                  << ", HSTP = " << passFilter << std::endl;
    }

}

/*

Questions to ask:

# On return to environment 

cd ~/HSTP_athena/build
setupATLAS
asetup --restore
cd ~/HSTP_athena/run


# build

cd ~/HSTP_athena/build
cmake -DATLAS_PACKAGE_FILTER_FILE=../package_filters.txt ../athena/Projects/WorkDir
make -j4
source x86_64-el9-gcc14-opt/setup.sh
cd ~/HSTP_athena/run
cmake --build $TestArea


Test_HSTP /eos/user/m/mbsmith/QT_AODs/Individual_Files/mc23g_DAOD_PHYS.50426169._000534.pool.root.1

Derivation_tf.py --inputAODFile /eos/user/m/mbsmith/QT_AODs/Individual_Files/MC23e_Dijet_AOD.46808784._000193.pool.root.1 --outputDAODFile art.pool.root --formats JETM1 --maxEvents 10 > Out.txt 2>&1 


*/
