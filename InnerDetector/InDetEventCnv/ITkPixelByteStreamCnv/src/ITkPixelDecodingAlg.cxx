/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingAlg.h"
#include "StoreGate/ReadHandle.h"
#include "eformat/ROBFragment.h"

#define ENABLE_TIMING = true;
#ifdef ENABLE_TIMING
#define SCOPED_TIMER(name, msg) ITkPixelDecoding::ScopedTimer timer_##__LINE__(name, msg)
#else
#define SCOPED_TIMER(name, msg)
#endif


using namespace itksw::pix::endec;

ITkPixelDecodingAlg::ITkPixelDecodingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
  
}


StatusCode ITkPixelDecodingAlg::initialize()
{
    ATH_CHECK(m_robDataProviderSvc.retrieve());

    ATH_CHECK(m_pixelCablingKey.initialize());

    ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));

    ATH_CHECK(m_pixelRDOKey.initialize());

    //this should go into the cabling likely...
    for (size_t hash = 0; hash < m_idHelper->wafer_hash_max(); hash++){
        m_sourceIDs.push_back(m_idHelper->wafer_id(hash).get_identifier32().get_compact() | 0b00 );
        m_sourceIDs.push_back(m_idHelper->wafer_id(hash).get_identifier32().get_compact() | 0b10 );
        m_sourceIDs.push_back(m_idHelper->wafer_id(hash).get_identifier32().get_compact() | 0b01 );
        m_sourceIDs.push_back(m_idHelper->wafer_id(hash).get_identifier32().get_compact() | 0b11 );
    }

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingAlg::execute(const EventContext& ctx) const
{
    //Timing
    SCOPED_TIMER("ITkPixelDecodingAlg::execute", msg());

    //Retrieve the ROB IDs from cabling - dummy as of now, happens in initialize()
    SG::ReadCondHandle<ITkPixelCablingData> cablingData(m_pixelCablingKey, ctx);
    const ITkPixelCablingData* cabling = *cablingData;

    //Instantiate the output. RN using the old pixel RDO, nothing fancier than that
    std::unique_ptr<PixelRDO_Container> rdoCont = std::make_unique<PixelRDO_Container>(m_idHelper->wafer_hash_max());

    //Instantiate the decoder. Doing this once per event is fine and MT-safe
    //It needs a "callback" that implements methods required by concepts. This is
    //the drawback of using an external code (by ITk online sw). Each of these methods
    //is then called at appropriate places in the decoding. The decoder is configured
    //with data format options. The callback can do whatever with the decoded hits
    //e. g. print them on the screen or put them in a container as RDOs.
    DataFormat fmt;
    fmt.options.en_chip_id = true;
    fmt.options.en_eos = true;
    PixelCallbacks::RDOCallback cb(rdoCont.get(), m_idHelper, msg());
    DecCore<PixelCallbacks::RDOCallback> core(fmt, cb);
    
    //Invoke ROBDataProviderService, fetch the concerned ROBs.
    std::vector<const eformat::ROBFragment<const uint32_t*>*> ROBs;
    m_robDataProviderSvc->getROBData(ctx, cabling->sourceIDs(), ROBs);
    ATH_MSG_DEBUG("Retrieved " << ROBs.size() << " fragments");

    //The ROB/ROD payload contains a chain of [FE header 0] [FE data 0] ... [FE header N] [FE data N]
    //we need to sort these header-data pairs in each ROB payload according to the module ID to
    //fill each module exactly once.
    //from the GBT fragment building algorithm documentation (https://gitlab.cern.ch/atlas-tdaq-software/swrod#gbt-fragment-building-algorithm)
    // "size: 16-bit value that contains a total size (in 4-byte words) of the data packet including the size of the header itself."
    //i. e. the first 16 bits of the first word allow us to identify where the next FE starts, etc. We can jump once through
    //the payload to separate the FE data and store the moduleID, {start idx, size} map, sort according to the key and then
    //read in that order from the underlying array without copying.


    //We can now loop over the payloads of the ROB fragments retrieved earlier.
    //There's as of now a little annoyance that the decoder eats 64 bit frames
    //but the ROBs come in 32 bits. At least they have the same endian polarity.
    //Nevertheless, we need to rearrange them.

    for (const auto& ROB : ROBs){
        //This loops over ROB fragments identified by sourceID

        //First, get the payload that contains the above-mentioned GBT fragment structure
        const uint32_t* payload = ROB->rod_data();
        uint32_t length = ROB->rod_ndata();

        //now starting from the 0th word, map out the GBT fragment beginnings
        //the map needs to be sorted by the low bits of detectorResourceID
        auto comp = [](const uint32_t& a, const uint32_t& b){
            uint32_t aLSB = a & 0x00FFFFFF;
            uint32_t bLSB = b & 0x00FFFFFF;
            if (aLSB != bLSB) return aLSB < bLSB;
            return a < b;
        };
        
        std::map<uint32_t, std::pair<size_t, uint16_t>, decltype(comp)> GBTFragments(comp);
        
        size_t idx = 0;
        while (idx < length){
            uint16_t GBTFragmentSize    = (payload[idx] & 0xFFFF0000) >> 16;
            uint32_t detectorResourceID = payload[idx + 1];
            //this is where we want to map detectorResourceID on trueDetectorResourceID
            GBTFragments.insert({detectorResourceID, {idx, GBTFragmentSize}});
            idx += GBTFragmentSize;
        }

        //at this point the GBT fragments are sorted by the moduleID, so they can
        //be added to the EDM (each module can be filled once). Before sorting,
        //it could in principle happen that two chips from module A would have
        //a GBT fragment from a chip belonging to module B in between.

        //Now we can loop over the sorted chips and fill them, always changing
        //the module we're currently dealing with

        uint32_t moduleID = 0xFFFFFFFF;
        for (const auto& [detectorResourceID, range] : GBTFragments){
            
            //are we still filling the same module?
            uint32_t currentModuleID = ITkPixelCabling::dridToModuleID(detectorResourceID);
            if (moduleID != currentModuleID){
                moduleID = currentModuleID;
                cb.setOfflineID(currentModuleID);
                cb.setTransformType(cabling->transformType(currentModuleID));
            }
            cb.setChipID(ITkPixelCabling::dridToChipID(detectorResourceID));

            //Translate the data into 64 bits. We know the length, so we can reserve
            //the space to avoid reallocation. Since the frames are always 64 bits split
            //into 32, they'll always be divisible by 2 without modulo. First 2 words are
            //the GBT fragment header, hence range.first + 2
            std::vector<uint64_t> payload64;
            payload64.resize((range.second - 2) / 2);
            size_t payloadIdx = 0;
            for (uint32_t word = range.first + 2; word < range.first + range.second; word += 2){
                payload64[payloadIdx] = ((uint64_t)(payload[word]) << 32) | payload[word + 1];
                payloadIdx++;
            }

            //Decode!
            core.initialize();
            core.decode(payload64);
            core.finalize();

        }

    }

    //The container is filled by now. We can write it to SG
    SG::WriteHandle<PixelRDO_Container> pixelRDOContainerHandle(m_pixelRDOKey, ctx);
    ATH_CHECK(pixelRDOContainerHandle.record(std::move(rdoCont)));

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingAlg::finalize(){
    return StatusCode::SUCCESS;
}