/**
 * @file src/OutputConversionTool.cxx
 * @author zhaoyuan.cui@cern.ch
 */

#include "OutputConversionTool.h"
#include "FPGADataFormatUtilities.h"

StatusCode OutputConversionTool::initialize()
{
    ATH_MSG_INFO("Initializing OutputConversionTool tool");
    return StatusCode::SUCCESS;
}

StatusCode OutputConversionTool::decodeFPGAoutput(const std::vector<uint64_t> &bytestream, OutputConversion::FSM blockType) const
{
    // define the FSM
    OutputConversion::FSM state = OutputConversion::FSM::EventHeader;

    std::vector<uint64_t> header_words;
    std::vector<uint64_t> footer_words;
    std::vector<uint64_t> ghits_words;

    // Loop the bytestream
    for (const auto &word : bytestream)
    {
        ATH_MSG_DEBUG("word: " << std::hex << word << std::dec);
        switch (state)
        {
        case OutputConversion::FSM::EventHeader:
        {
            if (FPGADataFormatUtilities::get_bitfields_EVT_HDR_w1(word).flag != FPGADataFormatUtilities::EVT_HDR_FLAG && header_words.empty())
            {
                ATH_MSG_ERROR("The first word is not the event hearder! Something is wrong");
                state = OutputConversion::FSM::Error;
                break;
            }

            header_words.push_back(word);
            if (header_words.size() == 3)
            {
                FPGADataFormatUtilities::EVT_HDR_w1 header_w1 = FPGADataFormatUtilities::get_bitfields_EVT_HDR_w1(header_words[0]);
                FPGADataFormatUtilities::EVT_HDR_w2 header_w2 = FPGADataFormatUtilities::get_bitfields_EVT_HDR_w2(header_words[1]);
                FPGADataFormatUtilities::EVT_HDR_w3 header_w3 = FPGADataFormatUtilities::get_bitfields_EVT_HDR_w3(header_words[2]);

                // The block after the event header must be a module header
                // but it's set to unknown because the logic of determing the block
                // is grouped with the event footer and the last cluster/ghit
                state = OutputConversion::FSM::Unknown;

                // print all bit fileds of the the event header
                ATH_MSG_DEBUG("Event Header: ");
                ATH_MSG_DEBUG("\tflag: 0x" << std::hex << header_w1.flag << std::dec);
                ATH_MSG_DEBUG("\tl0id: " << header_w1.l0id);
                ATH_MSG_DEBUG("\tbcid: " << header_w1.bcid);
                ATH_MSG_DEBUG("\tspare: " << header_w1.spare);
                ATH_MSG_DEBUG("\trunnumber: " << header_w2.runnumber);
                ATH_MSG_DEBUG("\ttime: " << header_w2.time);
                ATH_MSG_DEBUG("\tstatus: " << header_w3.status);
                ATH_MSG_DEBUG("\tcrc: " << header_w3.crc);
                break;
            }
            break;
        }
        case OutputConversion::FSM::Unknown:
        {
            // Determine if this is a module header or event footer
            if (FPGADataFormatUtilities::get_bitfields_M_HDR_w1(word).flag == FPGADataFormatUtilities::M_HDR_FLAG)
            {
                auto module = FPGADataFormatUtilities::get_bitfields_M_HDR_w1(word);
                // decode the module header
                // print all bit fileds of the the module header
                ATH_MSG_DEBUG("Module Header: ");
                ATH_MSG_DEBUG("\tflag: 0x" << std::hex << module.flag << std::dec);
                ATH_MSG_DEBUG("\tmodid: " << module.modid);
                ATH_MSG_DEBUG("\tspare: " << module.spare);

                // The following block is determined by the blockType
                // It should be one of: PixelClusters, StripClusters, PixelL2G, StripL2G, GlobalHits
                switch (blockType)
                {
                case OutputConversion::FSM::PixelClusters:
                {
                    state = OutputConversion::FSM::PixelClusters;
                    auto pixel_module = FPGADataFormatUtilities::get_bitfields_PIXEL_MODULE(module.modid);

                    ATH_MSG_DEBUG("Pixel Module: ");
                    ATH_MSG_DEBUG("\tlayer: " << pixel_module.layer);
                    ATH_MSG_DEBUG("\tphi: " << pixel_module.phi);
                    ATH_MSG_DEBUG("\teta: " << pixel_module.eta);

                    break;
                }
                case OutputConversion::FSM::StripClusters:
                {
                    state = OutputConversion::FSM::StripClusters;
                    auto strip_module = FPGADataFormatUtilities::get_bitfields_STRIP_MODULE(module.modid);

                    ATH_MSG_DEBUG("Strip Module: ");
                    ATH_MSG_DEBUG("\tlayer: " << strip_module.layer);
                    ATH_MSG_DEBUG("\tphi: " << strip_module.phi);
                    ATH_MSG_DEBUG("\teta: " << strip_module.eta);
                    ATH_MSG_DEBUG("\tside: " << strip_module.side);

                    break;
                }
                case OutputConversion::FSM::PixelL2G:
                {
                    // The actual dataformat is the same as the GlobalHits
                    state = OutputConversion::FSM::GlobalHits;
                    auto pixel_module = FPGADataFormatUtilities::get_bitfields_PIXEL_MODULE(module.modid);

                    ATH_MSG_DEBUG("Pixel Module: ");
                    ATH_MSG_DEBUG("\tlayer: " << pixel_module.layer);
                    ATH_MSG_DEBUG("\tphi: " << pixel_module.phi);
                    ATH_MSG_DEBUG("\teta: " << pixel_module.eta);

                    break;
                }
                case OutputConversion::FSM::StripL2G:
                {
                    // The actual dataformat is the same as the GlobalHits
                    state = OutputConversion::FSM::GlobalHits;
                    auto strip_module = FPGADataFormatUtilities::get_bitfields_STRIP_MODULE(module.modid);

                    ATH_MSG_DEBUG("Strip Module: ");
                    ATH_MSG_DEBUG("\tlayer: " << strip_module.layer);
                    ATH_MSG_DEBUG("\tphi: " << strip_module.phi);
                    ATH_MSG_DEBUG("\teta: " << strip_module.eta);
                    ATH_MSG_DEBUG("\tside: " << strip_module.side);

                    break;
                }
                case OutputConversion::FSM::GlobalHits:
                {
                    state = OutputConversion::FSM::GlobalHits;
                    auto strip_module = FPGADataFormatUtilities::get_bitfields_STRIP_MODULE(module.modid);

                    ATH_MSG_DEBUG("Strip Module: ");
                    ATH_MSG_DEBUG("\tlayer: " << strip_module.layer);
                    ATH_MSG_DEBUG("\tphi: " << strip_module.phi);
                    ATH_MSG_DEBUG("\teta: " << strip_module.eta);
                    ATH_MSG_DEBUG("\tside: " << strip_module.side);

                    break;
                }
                default:
                {
                    ATH_MSG_ERROR("The blockType is not recognized! Something is wrong");
                    state = OutputConversion::FSM::Error;
                    break;
                }
                }
            }
            else if (FPGADataFormatUtilities::get_bitfields_EVT_FTR_w1(word).flag == FPGADataFormatUtilities::EVT_FTR_FLAG)
            {
                state = OutputConversion::FSM::EventFooter;
                footer_words.push_back(word);
                break;
            }
            else
            {
                ATH_MSG_ERROR("The word after the last cluster is not a module header or event footer! Something is wrong");
                state = OutputConversion::FSM::Error;
                break;
            }
            break;
        }
        case OutputConversion::FSM::PixelClusters:
        {
            // decode the clusters
            auto cluster = FPGADataFormatUtilities::get_bitfields_PIXEL_CLUSTER(word);
            ATH_MSG_DEBUG("Cluster: ");
            ATH_MSG_DEBUG("\tlast: " << cluster.last);
            ATH_MSG_DEBUG("\tcol_size: " << cluster.col_size);
            ATH_MSG_DEBUG("\tcol: " << cluster.col);
            ATH_MSG_DEBUG("\trow_size: " << cluster.row_size);
            ATH_MSG_DEBUG("\trow: " << cluster.row);
            ATH_MSG_DEBUG("\tclusterid: " << cluster.clusterid);
            ATH_MSG_DEBUG("\tspare: " << cluster.spare);

            // Determine if this is the last cluster
            if (cluster.last == 1)
            {
                state = OutputConversion::FSM::Unknown;
                break;
            }
            break;
        }
        case OutputConversion::FSM::StripClusters:
        {
            // First we need to determine if this 64 bit word contain the lower 32 bits of the cluster
            if (FPGADataFormatUtilities::get_dataformat_STRIP_CLUSTER_low32(word) == 0)
            {
                // We only need to consider the upper 32 bits
                auto upper = FPGADataFormatUtilities::get_dataformat_STRIP_CLUSTER_up32(word);
                ATH_MSG_DEBUG("Cluster upper 32 bits: " << std::hex << upper << std::dec);

                auto cluster = FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(upper);
                ATH_MSG_DEBUG("Cluster: ");
                ATH_MSG_DEBUG("\tlast: " << cluster.last);
                ATH_MSG_DEBUG("\trow: " << cluster.row);
                ATH_MSG_DEBUG("\tnstrips: " << cluster.nstrips);
                ATH_MSG_DEBUG("\tstrip_index: " << cluster.strip_index);
                ATH_MSG_DEBUG("\tclusterid: " << cluster.clusterid);
                ATH_MSG_DEBUG("\tspare: " << cluster.spare);

                // Determine if this is the last cluster
                if (cluster.last == 1)
                {
                    state = OutputConversion::FSM::Unknown;
                    break;
                }
                break;
            }
            else
            {
                // We need to consider both the upper and lower 32 bits
                auto cluster_low = FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(FPGADataFormatUtilities::get_dataformat_STRIP_CLUSTER_low32(word));
                auto cluster_up = FPGADataFormatUtilities::get_bitfields_STRIP_CLUSTER(FPGADataFormatUtilities::get_dataformat_STRIP_CLUSTER_up32(word));

                ATH_MSG_DEBUG("Cluster upper 32 bits: ");
                ATH_MSG_DEBUG("\tlast: " << cluster_up.last);
                ATH_MSG_DEBUG("\trow: " << cluster_up.row);
                ATH_MSG_DEBUG("\tnstrips: " << cluster_up.nstrips);
                ATH_MSG_DEBUG("\tstrip_index: " << cluster_up.strip_index);
                ATH_MSG_DEBUG("\tclusterid: " << cluster_up.clusterid);
                ATH_MSG_DEBUG("\tspare: " << cluster_up.spare);

                ATH_MSG_DEBUG("Cluster lower 32 bits: ");
                ATH_MSG_DEBUG("\tlast: " << cluster_low.last);
                ATH_MSG_DEBUG("\trow: " << cluster_low.row);
                ATH_MSG_DEBUG("\tnstrips: " << cluster_low.nstrips);
                ATH_MSG_DEBUG("\tstrip_index: " << cluster_low.strip_index);
                ATH_MSG_DEBUG("\tclusterid: " << cluster_low.clusterid);
                ATH_MSG_DEBUG("\tspare: " << cluster_low.spare);

                // Determine if this is the last cluster
                if (cluster_low.last == 1)
                {
                    state = OutputConversion::FSM::Unknown;
                    break;
                }
                break;
            }
        }
        case OutputConversion::FSM::GlobalHits:
        {
            ghits_words.push_back(word);
            if (ghits_words.size() == 2)
            {
                auto GHit_w1 = FPGADataFormatUtilities::get_bitfields_GHITZ_w1(ghits_words[0]);
                auto GHit_w2 = FPGADataFormatUtilities::get_bitfields_GHITZ_w2(ghits_words[1]);

                // print all bit fileds of the the global hit
                ATH_MSG_DEBUG("Global Hit: ");
                ATH_MSG_DEBUG("\tlast: " << GHit_w1.last);
                ATH_MSG_DEBUG("\tlyr: " << GHit_w1.lyr);
                ATH_MSG_DEBUG("\trad: " << std::hex << GHit_w1.rad << std::dec);
                ATH_MSG_DEBUG("\tphi: " << std::hex << GHit_w1.phi << std::dec);
                ATH_MSG_DEBUG("\tz: " << std::hex << GHit_w1.z << std::dec);
                ATH_MSG_DEBUG("\trow: " << GHit_w1.row);
                ATH_MSG_DEBUG("\tspare: " << GHit_w1.spare);
                ATH_MSG_DEBUG("\tcluster1: " << GHit_w2.cluster1);
                ATH_MSG_DEBUG("\tcluster2: " << GHit_w2.cluster2);
                ATH_MSG_DEBUG("\tspare: " << GHit_w2.spare);

                // clear the ghits_words
                ghits_words.clear();

                // Determine if this is the last
                if (GHit_w1.last == 1)
                {
                    state = OutputConversion::FSM::Unknown;
                    break;
                }
                break;
            }
            break;
        }
        case OutputConversion::FSM::EventFooter:
        {
            footer_words.push_back(word);
            if (footer_words.size() == 3)
            {
                // decode the event footer
                FPGADataFormatUtilities::EVT_FTR_w1 footer_w1 = FPGADataFormatUtilities::get_bitfields_EVT_FTR_w1(footer_words[0]);
                FPGADataFormatUtilities::EVT_FTR_w2 footer_w2 = FPGADataFormatUtilities::get_bitfields_EVT_FTR_w2(footer_words[1]);
                FPGADataFormatUtilities::EVT_FTR_w3 footer_w3 = FPGADataFormatUtilities::get_bitfields_EVT_FTR_w3(footer_words[2]);

                // print all bit fileds of the the event footer
                ATH_MSG_DEBUG("Event Footer: ");
                ATH_MSG_DEBUG("\tflag: 0x" << std::hex << footer_w1.flag << std::dec);
                ATH_MSG_DEBUG("\tspare: " << footer_w1.spare);
                ATH_MSG_DEBUG("\thdr_crc: " << footer_w1.hdr_crc);
                ATH_MSG_DEBUG("\terror_flags: " << footer_w2.error_flags);
                ATH_MSG_DEBUG("\tword_count: " << footer_w3.word_count);
                ATH_MSG_DEBUG("\tcrc: " << footer_w3.crc);

                // clear header and footer words
                header_words.clear();
                footer_words.clear();

                // reset the state to EventHeader
                // Caveat: this enables continue decoding the next event if TV contains multiple events
                state = OutputConversion::FSM::EventHeader;
                break;
            }
            break;
        }
        case OutputConversion::FSM::Error:
        {
            ATH_MSG_ERROR("FSM is in an error state! Something is wrong");
            break;
        }
        default:
            ATH_MSG_ERROR("FSM is not in a valid state! Something is wrong");
            break;
        }
    }
    return StatusCode::SUCCESS;
}

StatusCode OutputConversionTool::decodePixelClusters(const std::vector<uint64_t> &bytestream) const
{
    ATH_MSG_DEBUG("Decoding pixel clusters");
    return decodeFPGAoutput(bytestream, OutputConversion::FSM::PixelClusters);
}

StatusCode OutputConversionTool::decodeStripClusters(const std::vector<uint64_t> &bytestream) const
{
    ATH_MSG_DEBUG("Decoding strip clusters");
    return decodeFPGAoutput(bytestream, OutputConversion::FSM::StripClusters);
}

StatusCode OutputConversionTool::decodePixelL2G(const std::vector<uint64_t> &bytestream) const
{
    ATH_MSG_DEBUG("Decoding pixel L2G");
    return decodeFPGAoutput(bytestream, OutputConversion::FSM::PixelL2G);
}

StatusCode OutputConversionTool::decodeStripL2G(const std::vector<uint64_t> &bytestream) const
{
    ATH_MSG_DEBUG("Decoding strip L2G");
    return decodeFPGAoutput(bytestream, OutputConversion::FSM::StripL2G);
}

StatusCode OutputConversionTool::decodeSpacePoints(const std::vector<uint64_t> &bytestream) const
{
    ATH_MSG_DEBUG("Decoding space points");
    return decodeFPGAoutput(bytestream, OutputConversion::FSM::GlobalHits);
}
