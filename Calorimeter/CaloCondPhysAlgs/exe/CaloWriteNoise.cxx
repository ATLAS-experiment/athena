////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// CHAI based filling of CaloNoise
///
/// Environment Variables needed:
/// - CHAI_CREST_SERVER: CREST server URL (default: http://crest-j23.cern.ch:8081/api-v6.0)
/// - CREST_AUTH_MODE: Authentication mode (default: JWT)
/// or use command line parameterts
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <climits>
#include <fstream>

#include <boost/program_options.hpp>

#include <chai/Database.h>
#include <chai/Container.h>
#include <chai/Log.h>
#include <chai/Types.h>
#include <CaloCondBlobObjs/CaloCondBlobFlt.h>

#define ENCODE_RUNLUMIBLOCK(run, lumiblock) (static_cast<uint64_t>(run) << 32) | static_cast<uint64_t>(lumiblock)
#define DECODE_RUN(since) static_cast<uint32_t>(since >> 32)
#define DECODE_LUMIBLOCK(since) static_cast<uint32_t>(since & 0xFFFFFFFF)
#define NUM_SCELLS 34048 
#define NORM_FACTOR_DT25 29.
#define NORM_FACTOR_DT50 53.


// Method to convert coral::Blob to BlobData
chai::BlobData blobToBlobData(const coral::Blob& coralBlob) {
    const void* data = coralBlob.startingAddress();
    long size = coralBlob.size();

    // Create vector and copy data
    std::vector<uint8_t> bytes(size);
    if (size > 0 && data != nullptr) {
        memcpy(bytes.data(), data, size);
    }

    return chai::BlobData(std::move(bytes));
}

coral::Blob BlobDataToBlob(const chai::BlobData& blob) {
   long size = blob.m_bytes.size();
   coral::Blob  data(size);
   if(size > 0) {
      memcpy(data.startingAddress(), blob.m_bytes.data(), size);
   }

   return data;
}
//coverity[UNCAUGHT_EXCEPT]
int main(int argc, char ** argv) {

     boost::program_options::options_description description( "Options" );
 
     description.add_options()
     ( "help,h", "produce help message" )
     ( "runstart,r", boost::program_options::value<uint32_t>()->default_value(0), "run number to start IOV" )
     ( "lbstart,l", boost::program_options::value<uint32_t>()->default_value(0), "lumi block to start IOV" )
     ( "runend,R", boost::program_options::value<uint32_t>()->default_value(0), "run number to end IOV (infty if 0)" )
     ( "lbend,L", boost::program_options::value<uint32_t>()->default_value(0), "lumi block to end IOV (infty if 0)" )
     ( "input,i", boost::program_options::value<std::string>()->default_value("calonoise.txt"), "name of input text file" )
     ( "output,t", boost::program_options::value<std::string>()->default_value(""), "name of output text file" )
     ( "mu,,", boost::program_options::value<uint32_t>()->default_value(0), "mu for which to fill noise" )
     ( "dt,,", boost::program_options::value<float>()->default_value(25.), "bunch spacing to use" )
     ( "folder,f", boost::program_options::value<std::string>()->default_value("/LAR/NoiseOfl/CellNoise"), "name of Folder" )
     ( "tag,a", boost::program_options::value<std::string>()->default_value("LARNoiseOflCellNoisenoise-mc16-EposA3-ofc25mu140-25ns"), "name of folder tag" )
     ( "crest,C", boost::program_options::value<std::string>()->default_value(""), "CREST URL string for storing" )
     ( "crestfs,c", boost::program_options::value<std::string>()->default_value("/tmp/crest"), "CREST FS string for storing" )
     ( "globalTag,g", boost::program_options::value<std::string>()->default_value(""), "Global tag in CREST" )
     ( "head,H", boost::program_options::bool_switch()->default_value(false), "Use HEAD tag" )
     ( "loglevel,o", boost::program_options::value<int>()->default_value(3), "chai logLevel" )
     ( "readonly,D", boost::program_options::bool_switch()->default_value(false), "Only read the DB" );

     boost::program_options::variables_map arguments;
     try {
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, description ), arguments );
        boost::program_options::notify( arguments );
     }
     catch ( boost::program_options::error & ex ) {
         std::cerr << ex.what() << std::endl;
         description.print( std::cout );
         return 1;
     }
 
     if ( arguments.count("help") ) {
        std::cout << "LAr noise filling  application" << std::endl;
        description.print( std::cout );
        return 0;
     }

    chai::setLogLevel(static_cast<chai::LogLevel>(arguments["loglevel"].as<int>()));

    std::string globalTag(arguments["globalTag"].as<std::string>());
    std::string folder(arguments["folder"].as<std::string>());
    std::string ftag(arguments["tag"].as<std::string>());
    bool isRead(arguments["readonly"].as<bool>());
    unsigned int runs(arguments["runstart"].as<uint32_t>());
    unsigned int rune=(arguments["runend"].as<uint32_t>()) > 0 ? arguments["runend"].as<uint32_t>() : UINT_MAX;
    unsigned int lbs=(arguments["lbstart"].as<uint32_t>());
    unsigned int lbe=(arguments["lbend"].as<uint32_t>()) > 0 ? arguments["lbend"].as<uint32_t>() : UINT_MAX;
    unsigned int mu=(arguments["mu"].as<uint32_t>());
    float dt=(arguments["dt"].as<float>());
    std::string server(arguments["crest"].as<std::string>());
    std::string fsdir(arguments["crestfs"].as<std::string>());

    if(!isRead){
       // read the input file
       std::vector<std::pair<float,float> > noisetable;
       std::ifstream inpf(arguments["input"].as<std::string>());
       // SC case
       // assuming onlId subHash gain noiseA noiseB
       unsigned int chanId, subh, igain;
       unsigned int prevId=0;
       float noisea=0, noiseb=0.;
       while(inpf >> chanId >> subh >> igain >> noisea >> noiseb) {
          if(igain != 0 || chanId <= prevId) { //something wrong
             std::cout<<"Wrong line in input file, starting with "<<chanId<<std::endl;
             continue;
          }
          noisetable.push_back(std::make_pair(noisea, noiseb));
          prevId=chanId; 
       } 
       if(noisetable.size() != NUM_SCELLS) {
          std::cout<<"Bad length of noise values, exiting !!"<<std::endl;
          return -1;
       } else {
          std::cout<<"Read out " << NUM_SCELLS << " noise values"<<std::endl;
          inpf.close();
       }
 
       // Create CaloBlobs to write out
       coral::Blob blob(5*sizeof(uint32_t)+2*noisetable.size()*sizeof(float));
       // filling the proper type
       uint32_t* pDestination = static_cast<uint32_t*> (blob.startingAddress());
       pDestination[0] = 1;
 
       CaloCondBlobFlt *flt = CaloCondBlobFlt::getInstance(blob);
       //Blob Defintion Vector
       std::vector<float> gVec;
       gVec.push_back(0.);//a
       gVec.push_back(0.);//b
       std::vector<std::vector<float> > defVec;
       defVec.push_back(gVec);
       flt->init(defVec,noisetable.size(),1);
 
       // Fill the flt
       for(unsigned h=0; h<noisetable.size(); ++h) {
          flt->setData(h,0,0,noisetable[h].first);
          if (mu > 0 && dt > 0) {
               // new normalization
               if (dt > 25)
                  flt->setData(h,0,1,noisetable[h].second / std::sqrt(mu/NORM_FACTOR_DT50*10.) ); // multiply by 10. to get units stored in cond DB.
               else
                  flt->setData(h,0,1,noisetable[h].second / std::sqrt(mu/NORM_FACTOR_DT25*10.) ); // multiply by 10. t o get units stored in cond DB.
          } else {
                  flt->setData(h,0,1,noisetable[h].second );
          }
       }
 
       // Build the connection string
       std::string connectionString = (server.size()==0) ? "crest_fs:" + fsdir : "crest:" + server;
 
       std::cout << "Connecting to: " << connectionString << std::endl << std::endl;
 
       try {
           // Connect to CREST database
           chai::Database db(connectionString);
 
           // Delete the dest. tag if it exists
           try {
               if (db.getTag(ftag)) {
                   db.deleteTag(ftag);
               }
           } catch (const std::exception& e) {
               // Ignore errors
           }
 
 
           // 1. Create a tag with payload specification
           std::cout << "Creating tag "<< ftag << std::endl;
 
           chai::PayloadSpec spec(
               chai::FieldSpec({
                               {"CaloCondBlob16M", chai::Blob}
                               }),
               chai::ChannelSpec({ {0,""} })
                                 );
 
           chai::Tag::Metadata chaiMD{
              .iovType=chai::Tag::IovType::RunNumberLumiBlock, 
              .objectType="crest-json-single-iov",
              .synchronization=chai::Tag::Synchronization::All, 
              .status=chai::Tag::Status::Unlocked,
              .nodeDescription =
                  chai::Tag::buildNodeDescription(
                       chai::Tag::IovType::RunNumberLumiBlock,
                       "CondAttrListCollection",
                       1238547719u
                  )
           };
           auto tag = db.createTag(
               ftag,
               "SC noise tag",
               spec,
               chaiMD      
           );
 
           std::cout << "Tag created: " << tag->getName() << std::endl;
           std::cout << "IOV Type: RunNumberLumiBlock" << std::endl;
 
           // 2. Create container 
 
           chai::Container container(spec);
 
           container[0].push(chai::BlobData(blobToBlobData(blob)));
 
           std::cout << "Container created with " << container.numChannels() << " channels" << std::endl;
 
           // 3. Write IOVs for selected run/lumiblock  interval
           std::cout << "Writing IOVs for selected run/lumiblock interval..." << std::endl;
 
           // IOV:
           uint64_t since = ENCODE_RUNLUMIBLOCK(runs, lbs);
           uint64_t until = ENCODE_RUNLUMIBLOCK(rune, lbe);
           tag->addPayload(container, since, until);
 
           std::cout << "All IOVs written successfully!" << std::endl << std::endl;
       } catch (const std::exception& e) {
           std::cerr << std::endl << "Error: " << e.what() << std::endl;
           return 1;
       }

    }
    try {
        // ====================================================================
        // READ SECTION
        // ====================================================================
        std::cout << "================================================================================" << std::endl;
        std::cout << "READ: Retrieving Tag and Reading Back Payloads" << std::endl;
        std::cout << "================================================================================" << std::endl;
        // Build the connection string
        std::string connectionString = (server.size()==0) ? "crest_fs:" + fsdir : "crest:" + server;
 
        std::cout << "Connecting to: " << connectionString << std::endl << std::endl;
 
        chai::Database db(connectionString);

        // 1. Get tag by name
        std::cout << "Getting tag by name "<<ftag<<" ..." << std::endl;
        auto retrievedTag = db.getTag(ftag);
        std::cout << "Tag retrieved: " << retrievedTag->getName() << std::endl;
        std::cout << "Description: " << retrievedTag->getDescription() << std::endl;
        std::cout << "Number of IOVs: " << retrievedTag->getSize() << std::endl;

        // 2. Read back IOVs from tag
        std::cout << "Iterating over IOVs and payloads..." << std::endl << std::endl;

        // 3. Open output file if asked
        std::ofstream onpf;
        if(arguments["output"].as<std::string>().size()>0) onpf.open(arguments["output"].as<std::string>());

        for (const auto& iov : retrievedTag->getIovs(0)) {
            uint64_t since = iov.getSince();
            uint32_t run = DECODE_RUN(since);
            uint32_t lumiblock = DECODE_LUMIBLOCK(since);

            std::cout << "IOV: Run " << run << ", Lumiblock " << lumiblock << " (encoded: " << since << ")" << std::endl;
            std::cout << "  Payload hash: " << iov.getPayloadHash().substr(0, 16) << "..." << std::endl;

            // 3. Get payload as a container
            auto payloadContainer = retrievedTag->getPayload(iov.getPayloadHash());

            std::cout << "  Payload has " << payloadContainer.numChannels() << " channel(s)" << std::endl;

            // 4. For each channel, print values and access by name
            for (uint64_t channelId : payloadContainer.channelIds()) {

                // Access by index (positional)
                if(payloadContainer.hasChannel(channelId)) {
                    const auto cblob = BlobDataToBlob(payloadContainer.get<chai::BlobData>(channelId,"CaloCondBlob16M"));
                    std::cout << "    Channel " << channelId << " size: " << cblob.size() << std::endl;
                    const CaloCondBlobFlt *rflt = CaloCondBlobFlt::getInstance(cblob);
                    std::cout << "nGains: " << rflt->getNGains() << " nChannels: " << rflt->getNChans() << std::endl;
                    for(unsigned h=0; h<rflt->getNChans(); ++h) {
                       std::cout<< h << " noisea: " << rflt->getData(h,0,0) << " noiseb: " << rflt->getData(h,0,1) << std::endl;
                    }
                } else {
                    std::cout << "    Channel " << channelId << " does not have data in this IOV" << std::endl;
                }
            }

            std::cout << "Done"<<std::endl;
        }

       } catch (const std::exception& e) {
           std::cerr << std::endl << "Error: " << e.what() << std::endl;
           return 1;
       }

    return 0;
}
