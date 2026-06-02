/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkPixelDecodingPhaseIIRDOAlg.h"
#include "StoreGate/ReadHandle.h"
#include "eformat/ROBFragment.h"

#define ENABLE_TIMING = true;
#ifdef ENABLE_TIMING
#define SCOPED_TIMER(name, msg) ITkPixelDecodingPhaseIIRDO::ScopedTimer timer_##__LINE__(name, msg)
#else
#define SCOPED_TIMER(name, msg)
#endif

using namespace itksw::pix::endec;

ITkPixelDecodingPhaseIIRDOAlg::ITkPixelDecodingPhaseIIRDOAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
  
}


StatusCode ITkPixelDecodingPhaseIIRDOAlg::initialize()
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

StatusCode ITkPixelDecodingPhaseIIRDOAlg::execute(const EventContext& ctx) const
{
    //Timing
    SCOPED_TIMER("ITkPixelDecodingPhaseIIRDOAlg::execute", msg());

    //Retrieve the ROB IDs from cabling - dummy as of now, happens in initialize()
    SG::ReadCondHandle<ITkPixelCablingData> cablingData(m_pixelCablingKey, ctx);
    const ITkPixelCablingData* cabling = *cablingData;

    //Instantiate the decoder. Doing this once per event is fine and MT-safe
    //It needs a "callback" that implements methods required by concepts. This is
    //the drawback of using an external code (by ITk online sw). Each of these methods
    //is then called at appropriate places in the decoding. The decoder is configured
    //with data format options. The callback can do whatever with the decoded hits
    //e. g. print them on the screen or put them in a container as RDOs.
    DataFormat fmt;
    fmt.options.en_chip_id = true;
    fmt.options.en_eos = true;
    int container_list_size = 2;

    auto cont_coll = std::make_unique< PhaseIIPixelRawDataContainerMT>(m_idHelper->wafer_hash_max(), container_list_size);
    PhaseIIPixelRawDataContainerMT::ContainerPtr rdoCont = cont_coll->getNewContainerPtr();
    PixelCallbacksPhaseIIRDO::PhaseIIRDOCallback cb(cont_coll.get(), rdoCont, m_idHelper, msg());

    // Instantiate the output (PhaseII).
    rdoCont->reserve(m_n_rdos_est);

    DecCore<PixelCallbacksPhaseIIRDO::PhaseIIRDOCallback> core(fmt, cb);
    core.initialize();
    
    //Invoke ROBDataProviderService, fetch the concerned ROBs.
    std::vector<const eformat::ROBFragment<const uint32_t*>*> ROBs;
    m_robDataProviderSvc->getROBData(ctx, m_sourceIDs, ROBs);
    ATH_MSG_DEBUG("Retrieved " << ROBs.size() << " fragments");

    //First, sort the ROBs according to the module offline ID
    //This can work because we have 1 e-link per ROB for now
    //A new implementation will be required to do that per e-link in the future
    // (This is pending the DetectorResourceID implementation)

    std::vector<const eformat::ROBFragment<const uint32_t*>*> ROBs_sorted;
    unsigned int ROBIndex = 0;
    std::vector<std::pair<uint32_t, uint32_t> > rob_ind_offlineID;
    for (const auto& ROB : ROBs){
        ITkPixelCabling::ModuleInfo mi = cabling->offlineModuleInfo(ROB->rob_source_id());
        rob_ind_offlineID.emplace_back(std::pair<uint32_t, uint32_t> (ROBIndex, mi.id.get_identifier32().get_compact()));
        ROBIndex++;
    }
    //Lambda function to sort the vector of pairs according to the offline ID
    std::sort(rob_ind_offlineID.begin(), rob_ind_offlineID.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    //fill the sorted vector
    for (const auto& robID : rob_ind_offlineID){
        ROBs_sorted.emplace_back(ROBs[robID.first]);
    }    
    // End of the sorting of ROBs

    //We can now loop over the payloads of the ROB fragments retrieved earlier.
    //There's as of now a little annoyance that the decoder eats 64 bit frames
    //but the ROBs come in 32 bits. At least they have the same endian polarity.
    //Nevertheless, we need to rearrange them.
    std::vector<uint64_t> payload64;
    std::array<std::vector<uint64_t>, 4> split_streams;

    for (const auto& ROB : ROBs_sorted){
        const uint32_t* payload = ROB->rod_data();
        uint32_t length = ROB->rod_ndata();        

        //Translate the data into 64 bits. We know the length, so we can reserve
        //the space to avoid reallocation. Since the frames are always 64 bits split
        //into 32, they'll always be divisible by 2 without modulo.
        payload64.clear();
        payload64.resize(length / 2);
        for (uint32_t word = 0; word < length; word += 2){
            payload64[word / 2] = ((uint64_t)(payload[word]) << 32) | payload[word + 1];
        }


        //Need to set the correct offline ID for this ROB
        ITkPixelCabling::ModuleInfo mi = cabling->offlineModuleInfo(ROB->rob_source_id());
        cb.setOfflineID(mi.id.get_identifier32().get_compact());

        //Set transform type
        cb.setTransformType(mi.transform);

        //If this is a merged quad, it has all 4 chips in one ROB
        //In such case we need to decode them separately. Otherwise
        //It's a 1:1 correspondence

        if (mi.type != ITkPixelCabling::ModuleType::MergedQuad){
            cb.setChipID(0);
            core.decode(payload64);
        }
        else {
            //split according to chip ID, which is always in highest bits 1 and 2
            for (auto &v : split_streams) v.clear();
            for (const uint64_t& word64: payload64) split_streams[(word64 >> 61) & 0b11].push_back(word64);
            for (uint8_t chipID = 0; chipID < 4; chipID++){
                if (!split_streams[chipID].empty()){
                    cb.setChipID(chipID);
                    core.decode(split_streams[chipID]);
                }
            }
        }
    }
    cb.registerLastModule();


    //The container is filled by now. We can write it to SG
    SG::WriteHandle<PhaseIIPixelRawDataContainer> pixelRDOContainerHandle(m_pixelRDOKey, ctx);
    ATH_CHECK(pixelRDOContainerHandle.record(std::move(cont_coll)));

    return StatusCode::SUCCESS;
}

StatusCode ITkPixelDecodingPhaseIIRDOAlg::finalize(){

    return StatusCode::SUCCESS;
}