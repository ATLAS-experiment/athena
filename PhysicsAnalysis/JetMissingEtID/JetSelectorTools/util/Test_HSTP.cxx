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
#include "AthContainers/AuxElement.h"




int main(int /*argc*/, char* argv[])
{
    // Test for checknig name in AODS: check if JZ in name

    asg::AnaToolHandle<PMGTools::IPMGCrossSectionTool> pmgTool( "PMGTools::PMGCrossSectionTool/PMGCrossSectionTool");
    if (pmgTool.retrieve().isFailure()) std::cerr << "Failed to retrieve PMGCrossSectionTool" << std::endl;
    pmgTool->readInfosFromFiles( {PathResolverFindCalibFile("dev/PMGTools/PMGxsecDB_mc21.txt")});

    if (!xAOD::Init().isSuccess()) std::cerr << "Failed to initialize xAOD" << std::endl;

    xAOD::TEvent event(xAOD::TEvent::kClassAccess);

    std::unique_ptr<TFile> inputFile(TFile::Open(argv[1], "READ"));
    if (!inputFile || inputFile->IsZombie())         std::cerr << "Failed to open input file" << std::endl;
    if (event.readFrom(inputFile.get()).isFailure()) std::cerr << "Failed to read input file" << std::endl;

    const Long64_t nEntries = event.getEntries();
    for (Long64_t entry = 0; entry < std::min(nEntries, Long64_t{100}); ++entry) {

        if (event.getEntry(entry) < 0)  break;

        const xAOD::EventInfo* eventInfo = nullptr;
        if (!event.retrieve(eventInfo, "EventInfo").isSuccess()) return 5;

        bool needHSTP                = false;
        const int dsid               = eventInfo->mcChannelNumber();
        const std::string sampleName = pmgTool->getSampleName(dsid);

        if (const char* p = strstr(sampleName.c_str(), "JZ")) needHSTP = (p[2] >= '0' && p[2] <= '9');

        std::cout << "Run " << eventInfo->runNumber()
                  << ", Event " << eventInfo->eventNumber()
                  << ", needHSTP = " << needHSTP  << std::endl;
    }



    // testing passHSTPFilter Decoration

    // open and read output file 
    xAOD::TEvent event_output(xAOD::TEvent::kClassAccess);
    std::unique_ptr<TFile> outputFile(TFile::Open(argv[2], "READ"));

    if (!outputFile || outputFile->IsZombie())               std::cerr << "Failed to open input file" << std::endl;
    if (event_output.readFrom(outputFile.get()).isFailure()) std::cerr << "Failed to read input file" << std::endl;


    // initalize tool
    const std::string jetContainerName = "AntiKt4TruthDressedWZJets";
    const std::string jetPUContainerName = "InTimeAntiKt4TruthJets";
    auto tool = std::make_unique<ECUtils::EventCleaningTool>("EventCleaningTool");
    if (tool->setProperty("JetContainer", jetContainerName).isFailure()) std::cerr << "Failed to set JetContainer" << std::endl;
    if (tool->initialize().isFailure()) std::cerr << "Failed to initialize the tool" << std::endl;


    Long64_t nEntries2 = event_output.getEntries();
    for (Long64_t entry = 0; entry < nEntries2; ++entry) {

        if (event_output.getEntry(entry) < 0) break;

        const xAOD::EventInfo* eventInfo = nullptr;
        if (!event_output.retrieve(eventInfo, "EventInfo").isSuccess()) break;


        // access decoration decision
        static const SG::ConstAccessor<char> passHSTPFilterAcc("passHSTPFilter");
        int passHSTPFilter = static_cast<int>(passHSTPFilterAcc(*eventInfo) != 0);

        // check using the built in tool fn 
        const xAOD::JetContainer* jets   = nullptr;
        const xAOD::JetContainer* puJets = nullptr;
        if (!event_output.retrieve(puJets, jetPUContainerName).isSuccess())return 7;
        if (!event_output.retrieve(jets,     jetContainerName).isSuccess())return 8;
        bool passFilter = tool->passHSTPFilter(jets, puJets);

        // check using manual calculation
        bool passFiltercheck = true;
        double maxHsJetPt = 5000; 
        double maxHsPUJetPt = 0; 
        for (const auto thisJet : *jets){
          if (thisJet->pt() > maxHsJetPt) maxHsJetPt = thisJet->pt();
        }
        for (const auto thisPUJet : *puJets){ 
          if (thisPUJet->pt() > maxHsJetPt){
            maxHsPUJetPt = thisPUJet->pt();
            passFiltercheck = false;
          }
        } 

        std::cout << "Run " << eventInfo->runNumber()
                  << ", Event " << eventInfo->eventNumber()
                  << ", HSTP Decoration = " << passHSTPFilter 
                  << ", HSTP Tool = " << passFilter 
                  << ", HSTP Manual = " << passFiltercheck; 
                  if (!passFiltercheck) std::cout << " Leading HS Jet pT: " << maxHsJetPt << " Leading PU Jet pT: "  << maxHsPUJetPt << std::endl;
                  else                  std::cout << std::endl;
    }
}

/*

TODo:
Documentaion 
  - add comments 
  - fix up the formating of every file effected
  - update Jet twiki
Confirm the decoration is being filled
  - Re-enlist Test_HSTP, now reading in the output DAODs and printing the results.



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


Test_HSTP /eos/user/m/mbsmith/QT_AODs/Individual_Files/DAOD_JETM2_MC23a.36803052._000487.pool.root.1 /afs/cern.ch/user/m/mbsmith/HSTP_athena/run/DAOD_JETM1.art.pool.root

Derivation_tf.py --inputAODFile /eos/user/m/mbsmith/QT_AODs/Individual_Files/MC23e_Dijet_AOD.46808784._000193.pool.root.1 --outputDAODFile Dijet.art.pool.root --formats JETM2 --maxEvents 10 > Out.txt 2>&1 

Derivation_tf.py --inputAODFile /eos/user/m/mbsmith/QT_AODs/Individual_Files/MinBias_AOD.28353412._002336.pool.root.1 --outputDAODFile minBias.art.pool.root --formats JETM2 --maxEvents 10 > OutMinBias.txt 2>&1 


/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/PMGTools/PMGxsecDB_mc23.txt
*/





// typedef enum { LocalSearch, RecursiveSearch } SearchType;
// typedef enum { PR_regular_file, PR_directory } PR_file_type;

// std::string PathResolver::find_calib_file(const std::string& logical_file_name){
//   checkForDev(asgMsg(), logical_file_name);

//   if (logical_file_name.starts_with("root://")) {
//     //xrootd access .. try to open file ...
//     std::unique_ptr<TFile> fTmp{TFile::Open(logical_file_name.c_str())};
//     if (!fTmp || fTmp->IsZombie()) {
//       msg(MSG::WARNING) << "Could not open " << logical_file_name << endmsg;
//       return {};
//     }
//     return logical_file_name;
//   }

//   std::string path_list;
//   System::getEnv( "CALIBPATH", path_list );

//   std::string out( "" );
//   bf::path lfn( logical_file_name );
//   PR_find( lfn, path_list, PR_regular_file, LocalSearch, out );

//   if (out.empty()) {
//     msg(MSG::WARNING) << "Could not locate " << logical_file_name << endmsg;
//   }
//   return out;
// }


//  static bool PR_find( const bf::path& file, const string& search_list, PR_file_type file_type,
//                       PathResolver::SearchType search_type, string& result ) {

//    bool found( false );

//    // look for file as specified first

//    try {
//      if ( ( file_type == PR_regular_file && is_regular_file( file ) ) ||
//           ( file_type == PR_directory && is_directory( file ) ) ) {
//        result = bf::system_complete( file ).string();
//        return true;
//      }
//    } catch ( const bf::filesystem_error& /*err*/ ) {}

//    // assume that "." is always part of the search path, so check locally first

//    try {
//      bf::path local = bf::initial_path() / file;
//      if ( ( file_type == PR_regular_file && is_regular_file( local ) ) ||
//           ( file_type == PR_directory && is_directory( local ) ) ) {
//        result = bf::system_complete( file ).string();
//       return true;
//     }
//   } catch ( const bf::filesystem_error& /*err*/ ) {}

//   // iterate through search list
//   vector<string> spv;
//   split( spv, search_list, boost::is_any_of( path_separator ), boost::token_compress_on );
//   for ( const auto& itr : spv ) {

//     bf::path fp = itr / file;

//     try {
//       if ( ( file_type == PR_regular_file && is_regular_file( fp ) ) ||
//            ( file_type == PR_directory && is_directory( fp ) ) ) {
//         result = bf::system_complete( fp ).string();
//         return true;
//       }
//     } catch ( const bf::filesystem_error& /*err*/ ) {}

//     // if recursive searching requested, drill down
//     if ( search_type == PathResolver::RecursiveSearch && is_directory( bf::path( itr ) ) ) {

//       bf::recursive_directory_iterator end_itr;
//       try {
//         for ( bf::recursive_directory_iterator ritr( itr ); ritr != end_itr; ++ritr ) {

//           // skip if not a directory
//           if ( !is_directory( bf::path( *ritr ) ) ) { continue; }

//           bf::path fp2 = bf::path( *ritr ) / file;
//           if ( ( file_type == PR_regular_file && is_regular_file( fp2 ) ) ||
//                ( file_type == PR_directory && is_directory( fp2 ) ) ) {
//             result = bf::system_complete( fp2 ).string();
//             return true;
//           }
//         }
//       } catch ( const bf::filesystem_error& /*err*/ ) {}
//     }
//   }

//   return found;
// }