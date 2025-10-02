/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
 /**
 * @file src/OutputConversionTool.cxx
 * @author zhaoyuan.cui@cern.ch
 */

#include "EFTrackingFPGAUtility/OutputConversionTool.h"
#include "EFTrackingFPGAUtility/FPGADataFormatUtilities.h"

StatusCode OutputConversionTool::initialize()
{
    ATH_MSG_INFO("Initializing OutputConversionTool tool");
    return StatusCode::SUCCESS;
}

StatusCode OutputConversionTool::decodeFPGAoutput(const std::vector<uint64_t> &bytestream,
                                                  EFTrackingTransient::Metadata *metadata,
                                                  EFTrackingTransient::PixelClusterAuxInput *pcAux,
                                                  EFTrackingTransient::StripClusterAuxInput *scAux,
                                                  OutputConversion::FSM blockType) const
{
    using namespace FPGADataFormatUtilities;
    // define the FSM
    OutputConversion::FSM state = OutputConversion::FSM::EventHeader;

    std::vector<uint64_t> header_words;
    std::vector<uint64_t> footer_words;
    std::vector<uint64_t> ghits_words;
    std::vector<uint64_t> gtracks_words;
    std::vector<uint64_t> slices_words;
    std::vector<uint64_t> pixel_edm_words;
    std::vector<uint64_t> strip_edm_words;

    unsigned int counter = 0;

    // Loop the bytestream
    for (const auto &word : bytestream)
    {
        ATH_MSG_DEBUG("word: " << std::hex << word << std::dec);
        switch (state)
        {
        case OutputConversion::FSM::EventHeader:
        {
            if (get_bitfields_EVT_HDR_w1(word).flag != EVT_HDR_FLAG && header_words.empty())
            {
                ATH_MSG_ERROR("The first word is not the event hearder! Something is wrong");
                state = OutputConversion::FSM::Error;
                break;
            }

            header_words.push_back(word);
            if (header_words.size() == 3)
            {
                EVT_HDR_w1 header_w1 = get_bitfields_EVT_HDR_w1(header_words[0]);
                EVT_HDR_w2 header_w2 = get_bitfields_EVT_HDR_w2(header_words[1]);
                EVT_HDR_w3 header_w3 = get_bitfields_EVT_HDR_w3(header_words[2]);

                // The data format has be evloing. The logic here can change dramatically
                // The block after the event header can be one of the following: Slice, Module, Road, Track, cluster EDM
                // For now, we only consider the cluster EDM
                switch (blockType)
                {
                case OutputConversion::FSM::PixelEDM:
                {
                    state = OutputConversion::FSM::PixelEDM;
                    if(pcAux == nullptr)
                    {
                        ATH_MSG_ERROR("The PixelClusterAuxInput is not provided to decode function! Something is wrong");
                        state = OutputConversion::FSM::Error;
                        break;
                    }
                    break;
                }
                case OutputConversion::FSM::StripEDM:
                {
                    state = OutputConversion::FSM::StripEDM;
                    if(scAux == nullptr)
                    {
                        ATH_MSG_ERROR("The StripClusterAuxInput is not provided to decode function! Something is wrong");
                        state = OutputConversion::FSM::Error;
                        break;
                    }
                    break;
                }
                case OutputConversion::FSM::GTracks:
                {
                    state = OutputConversion::FSM::GTracks;
                    break;
                }
                case OutputConversion::FSM::Slices:
                {
                    state = OutputConversion::FSM::Slices;
                    break;
                }
                default:
                {
                    ATH_MSG_ERROR("The blockType is not recognized! Something is wrong");
                    state = OutputConversion::FSM::Error;
                    break;
                }
                }

                // print all bit fileds of the the event header
                if (msgLvl(MSG::DEBUG))
                {
                    ATH_MSG_DEBUG("Event Header: ");
                    ATH_MSG_DEBUG("\tflag: 0x" << std::hex << header_w1.flag << std::dec);
                    ATH_MSG_DEBUG("\tl0id: " << header_w1.l0id);
                    ATH_MSG_DEBUG("\tbcid: " << header_w1.bcid);
                    ATH_MSG_DEBUG("\tspare: " << header_w1.spare);
                    ATH_MSG_DEBUG("\trunnumber: " << header_w2.runnumber);
                    ATH_MSG_DEBUG("\ttime: " << header_w2.time);
                    ATH_MSG_DEBUG("\tstatus: " << header_w3.status);
                    ATH_MSG_DEBUG("\tcrc: " << header_w3.crc);
                }
                break;
            }
            break;
        }
        case OutputConversion::FSM::Unknown:
        {
            // Determine if this is a module header or event footer
            if (get_bitfields_M_HDR_w1(word).flag == M_HDR_FLAG)
            {
                auto module = get_bitfields_M_HDR_w1(word);
                // decode the module header
                // print all bit fileds of the the module header
                if (msgLvl(MSG::DEBUG))
                {
                    ATH_MSG_DEBUG("Module Header: ");
                    ATH_MSG_DEBUG("\tflag: 0x" << std::hex << module.flag << std::dec);
                    ATH_MSG_DEBUG("\tmodid: " << module.modid);
                    ATH_MSG_DEBUG("\tmodhash: " << module.modhash);
                    ATH_MSG_DEBUG("\tspare: " << module.spare);
                }

                // The following block is determined by the actual content
                // Right now only the spacepoints are considered. This will likely change in future iteration.
                switch (blockType)
                {
                case OutputConversion::FSM::GlobalHits:
                {
                    state = OutputConversion::FSM::GlobalHits;
                    break;
                }
                case OutputConversion::FSM::Slices:
                {
                    state = OutputConversion::FSM::GlobalHits;
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
            // Determine if this is a module header or event footer
            else if (get_bitfields_GTRACK_HDR_w1(word).flag == GTRACK_HDR_FLAG)
            {
                state = OutputConversion::FSM::GTracks;
                gtracks_words.push_back(word);
                break;
            }
            else if (get_bitfields_EVT_FTR_w1(word).flag == EVT_FTR_FLAG)
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
        case OutputConversion::FSM::GlobalHits:
        {
            ghits_words.push_back(word);
            if (ghits_words.size() == 2)
              {
                auto Ghit_w1 = get_bitfields_GHITZ_w1(ghits_words[0]);
                auto Ghit_w2 = get_bitfields_GHITZ_w2(ghits_words[1]);
                 ATH_MSG_DEBUG("Global Hits: ");
                 ATH_MSG_DEBUG("GHIT W1: ");
                 ATH_MSG_DEBUG("\tlast: " << Ghit_w1.last/GHITZ_W1_LAST_mf);
                 ATH_MSG_DEBUG("\tlyr: " << Ghit_w1.lyr/GHITZ_W1_LYR_mf);
                 ATH_MSG_DEBUG("\trad: " << Ghit_w1.rad/GHITZ_W1_RAD_mf);
                 ATH_MSG_DEBUG("\tphi: " << Ghit_w1.phi/GHITZ_W1_PHI_mf);
                 ATH_MSG_DEBUG("\tz: " << Ghit_w1.z/GHITZ_W1_Z_mf);
                 ATH_MSG_DEBUG("\tlastofslice: " << Ghit_w1.lastofslice/GHITZ_W1_LASTOFSLICE_mf);
                 ATH_MSG_DEBUG("\tspare: " << Ghit_w1.spare/GHITZ_W1_SPARE_mf);
                 ATH_MSG_DEBUG("GHIT W2: ");
                 ATH_MSG_DEBUG("\tcluster1: " << Ghit_w2.cluster1/GHITZ_W2_CLUSTER1_mf);
                 ATH_MSG_DEBUG("\tcluster2: " << Ghit_w2.cluster2/GHITZ_W2_CLUSTER2_mf);
                 ATH_MSG_DEBUG("\trow: " << Ghit_w2.row/GHITZ_W2_ROW_mf);
                 ATH_MSG_DEBUG("\tspare: " << Ghit_w2.spare/GHITZ_W2_SPARE_mf);
                ghits_words.clear();
                if (Ghit_w1.last == 1)
                  {
                    state = OutputConversion::FSM::Unknown;
                    break;
                  }
                break;
            }
            break;
        }
        case OutputConversion::FSM::PixelEDM:
        {
            pixel_edm_words.push_back(word);
            // Read in 10 consecutive words
            if (pixel_edm_words.size() == 10)
            {
                // decode the pixel EDM
                // For pixel EDM aux, we need
                // id, idHash, localPosition, localCovariance, rdoList
                // channelsInPhi, channelsInEta, width in Eta
                // global position
                // totalToT
                pcAux->idHash.push_back(get_bitfields_EDM_PIXELCLUSTER_w1(pixel_edm_words[0]).id_hash);


                // check if the rdo list word is empty, there are only 4 words in the pixel EDM
                auto rdo_w1 = get_bitfields_EDM_PIXELCLUSTER_w2(pixel_edm_words[1]).rdo_list_w1;
                auto rdo_w2 = get_bitfields_EDM_PIXELCLUSTER_w3(pixel_edm_words[2]).rdo_list_w2;
                auto rdo_w3 = get_bitfields_EDM_PIXELCLUSTER_w4(pixel_edm_words[3]).rdo_list_w3;
                auto rdo_w4 = get_bitfields_EDM_PIXELCLUSTER_w5(pixel_edm_words[4]).rdo_list_w4;

                unsigned short current_rdo_size = 0;
                if (rdo_w1 != 0)
                {
                    pcAux->rdoList.push_back(rdo_w1);
                    current_rdo_size++;
                }
                if (rdo_w2 != 0)
                {
                    pcAux->rdoList.push_back(rdo_w2);
                    current_rdo_size++;
                }
                if (rdo_w3 != 0)
                {
                    pcAux->rdoList.push_back(rdo_w3);
                    current_rdo_size++;
                }
                if (rdo_w4 != 0)
                {
                    pcAux->rdoList.push_back(rdo_w4);
                    current_rdo_size++;
                }

                pcAux->localPosition.push_back(to_real_EDM_PIXELCLUSTER_w6_localposition_x(get_bitfields_EDM_PIXELCLUSTER_w6(pixel_edm_words[5]).localposition_x));
                pcAux->localPosition.push_back(to_real_EDM_PIXELCLUSTER_w6_localposition_y(get_bitfields_EDM_PIXELCLUSTER_w6(pixel_edm_words[5]).localposition_y));
                pcAux->channelsInPhi.push_back(get_bitfields_EDM_PIXELCLUSTER_w6(pixel_edm_words[5]).channels_in_phi);
                pcAux->channelsInEta.push_back(get_bitfields_EDM_PIXELCLUSTER_w6(pixel_edm_words[5]).channels_in_eta);
                pcAux->widthInEta.push_back(to_real_EDM_PIXELCLUSTER_w6_width_in_eta(get_bitfields_EDM_PIXELCLUSTER_w6(pixel_edm_words[5]).width_in_eta));

                pcAux->localCovariance.push_back(to_real_EDM_PIXELCLUSTER_w7_localcovariance_xx(get_bitfields_EDM_PIXELCLUSTER_w7(pixel_edm_words[6]).localcovariance_xx));
                pcAux->localCovariance.push_back(to_real_EDM_PIXELCLUSTER_w7_localcovariance_yy(get_bitfields_EDM_PIXELCLUSTER_w7(pixel_edm_words[6]).localcovariance_yy));

                pcAux->globalPosition.push_back(to_real_EDM_PIXELCLUSTER_w8_globalposition_x(get_bitfields_EDM_PIXELCLUSTER_w8(pixel_edm_words[7]).globalposition_x));
                pcAux->globalPosition.push_back(to_real_EDM_PIXELCLUSTER_w8_globalposition_y(get_bitfields_EDM_PIXELCLUSTER_w8(pixel_edm_words[7]).globalposition_y));

                pcAux->id.push_back(get_bitfields_EDM_PIXELCLUSTER_w9(pixel_edm_words[8]).identifier);

                pcAux->globalPosition.push_back(to_real_EDM_PIXELCLUSTER_w10_globalposition_z(get_bitfields_EDM_PIXELCLUSTER_w10(pixel_edm_words[9]).globalposition_z));
                pcAux->totalToT.push_back(get_bitfields_EDM_PIXELCLUSTER_w10(pixel_edm_words[9]).total_tot);


                metadata->pcRdoIndex[counter] = current_rdo_size;
                counter++;

                // Determine if this is the last cluster
                if (get_bitfields_EDM_PIXELCLUSTER_w10(pixel_edm_words[9]).lastword == 1)
                {
                    metadata->pcRdoIndexSize = counter;
                    metadata->numOfPixelClusters = counter;
                    state = OutputConversion::FSM::EventFooter;
                }
                pixel_edm_words.clear();

                // print all bit fileds of the the pixel EDM
                if (msgLvl(MSG::DEBUG))
                {
                    ATH_MSG_DEBUG("Pixel EDM: ");
                    ATH_MSG_DEBUG("\tid: " << pcAux->id.back());
                    ATH_MSG_DEBUG("\tidHash: " << pcAux->idHash.back());
                    ATH_MSG_DEBUG("\tlocalPosition_x: " << pcAux->localPosition.at(pcAux->localPosition.size() - 2));
                    ATH_MSG_DEBUG("\tlocalPosition_y: " << pcAux->localPosition.back());
                    ATH_MSG_DEBUG("\tlocalCovariance_xx: " << pcAux->localCovariance.at(pcAux->localCovariance.size() - 2));
                    ATH_MSG_DEBUG("\tlocalCovariance_yy: " << pcAux->localCovariance.back());
                    ATH_MSG_DEBUG("\tglobalPosition_x: " << pcAux->globalPosition.at(pcAux->globalPosition.size() - 3));
                    ATH_MSG_DEBUG("\tglobalPosition_y: " << pcAux->globalPosition.at(pcAux->globalPosition.size() - 2));
                    ATH_MSG_DEBUG("\tglobalPosition_z: " << pcAux->globalPosition.back());
                    ATH_MSG_DEBUG("\tchannelsInPhi: " << pcAux->channelsInPhi.back());
                    ATH_MSG_DEBUG("\tchannelsInEta: " << pcAux->channelsInEta.back());
                    ATH_MSG_DEBUG("\twidthInEta: " << pcAux->widthInEta.back());
                    ATH_MSG_DEBUG("\ttotalToT: " << pcAux->totalToT.back());
                }
                break;
            }

            break;
        }
        case OutputConversion::FSM::StripEDM:
        {
            strip_edm_words.push_back(word);
            // Read in 9 consecutive words
            if (strip_edm_words.size() == 9 and scAux)
            {
                scAux->idHash.push_back(get_bitfields_EDM_STRIPCLUSTER_w1(strip_edm_words[0]).id_hash);

                scAux->id.push_back(get_bitfields_EDM_STRIPCLUSTER_w8(strip_edm_words[7]).identifier);

                scAux->localPosition.push_back(to_real_EDM_STRIPCLUSTER_w6_localposition_x(get_bitfields_EDM_STRIPCLUSTER_w6(strip_edm_words[5]).localposition_x));
                scAux->localCovariance.push_back(to_real_EDM_STRIPCLUSTER_w6_localcovariance_xx(get_bitfields_EDM_STRIPCLUSTER_w6(strip_edm_words[5]).localcovariance_xx));

                scAux->globalPosition.push_back(to_real_EDM_STRIPCLUSTER_w7_globalposition_x(get_bitfields_EDM_STRIPCLUSTER_w7(strip_edm_words[6]).globalposition_x));
                scAux->globalPosition.push_back(to_real_EDM_STRIPCLUSTER_w7_globalposition_y(get_bitfields_EDM_STRIPCLUSTER_w7(strip_edm_words[6]).globalposition_y));
                scAux->channelsInPhi.push_back(get_bitfields_EDM_STRIPCLUSTER_w7(strip_edm_words[6]).channels_in_phi);

                scAux->globalPosition.push_back(to_real_EDM_STRIPCLUSTER_w9_globalposition_z(get_bitfields_EDM_STRIPCLUSTER_w9(strip_edm_words[8]).globalposition_z));
               
                // check if the rdo list word is empty, there are only 4 words in the strip EDM
                auto rdo_w1 = get_bitfields_EDM_STRIPCLUSTER_w2(strip_edm_words[1]).rdo_list_w1;
                auto rdo_w2 = get_bitfields_EDM_STRIPCLUSTER_w3(strip_edm_words[2]).rdo_list_w2;
                auto rdo_w3 = get_bitfields_EDM_STRIPCLUSTER_w4(strip_edm_words[3]).rdo_list_w3;
                auto rdo_w4 = get_bitfields_EDM_STRIPCLUSTER_w5(strip_edm_words[4]).rdo_list_w4;

                unsigned short current_rdo_size = 0;
                if (rdo_w1 != 0)
                {
                    scAux->rdoList.push_back(rdo_w1);
                    current_rdo_size++;
                }
                if (rdo_w2 != 0)
                {
                    scAux->rdoList.push_back(rdo_w2);
                    current_rdo_size++;
                }
                if (rdo_w3 != 0)
                {
                    scAux->rdoList.push_back(rdo_w3);
                    current_rdo_size++;
                }
                if (rdo_w4 != 0)
                {
                    scAux->rdoList.push_back(rdo_w4);
                    current_rdo_size++;
                }

                metadata->scRdoIndex[counter] = current_rdo_size;
                counter++;

                // Determine if this is the last cluster
                if (get_bitfields_EDM_STRIPCLUSTER_w9(strip_edm_words[8]).lastword == 1)
                {
                    metadata->scRdoIndexSize = counter;
                    metadata->numOfStripClusters = counter;
                    state = OutputConversion::FSM::EventFooter;
                }
                strip_edm_words.clear();

                // print all bit fileds of the the strip EDM
                if (msgLvl(MSG::DEBUG))
                {
                    ATH_MSG_DEBUG("Strip EDM: ");
                    ATH_MSG_DEBUG("\tidHash: " << scAux->idHash.back());
                    ATH_MSG_DEBUG("\tid: " << scAux->id.back());
                    ATH_MSG_DEBUG("\tlocalPosition_x: " << scAux->localPosition.back());
                    ATH_MSG_DEBUG("\tlocalCovariance_xx: " << scAux->localCovariance.back());
                    ATH_MSG_DEBUG("\tglobalPosition_x: " << scAux->globalPosition.at(scAux->globalPosition.size() - 3));
                    ATH_MSG_DEBUG("\tglobalPosition_y: " << scAux->globalPosition.at(scAux->globalPosition.size() - 2));
                    ATH_MSG_DEBUG("\tglobalPosition_z: " << scAux->globalPosition.back());
                    ATH_MSG_DEBUG("\tchannelsInPhi: " << scAux->channelsInPhi.back());
                    ATH_MSG_DEBUG("\trdoList: ");
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
                EVT_FTR_w1 footer_w1 = get_bitfields_EVT_FTR_w1(footer_words[0]);
                EVT_FTR_w2 footer_w2 = get_bitfields_EVT_FTR_w2(footer_words[1]);
                EVT_FTR_w3 footer_w3 = get_bitfields_EVT_FTR_w3(footer_words[2]);

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
                counter = 0;

                // reset the state to EventHeader
                // Caveat: this enables continue decoding the next event if TV contains multiple events
                state = OutputConversion::FSM::EventHeader;
                break;
            }
            break;
        }
        case OutputConversion::FSM::GTracks:
        {
            gtracks_words.push_back(word);
            if (gtracks_words.size() == 3)
                {
                    GTRACK_HDR_w1 gtrack_w1 = get_bitfields_GTRACK_HDR_w1(gtracks_words[0]);
                    GTRACK_HDR_w2 gtrack_w2 = get_bitfields_GTRACK_HDR_w2(gtracks_words[1]);
                    GTRACK_HDR_w3 gtrack_w3 = get_bitfields_GTRACK_HDR_w3(gtracks_words[2]);
                    ATH_MSG_DEBUG("GTrack: ");
                    ATH_MSG_DEBUG("\tflag: 0x" << std::hex << gtrack_w1.flag/GTRACK_HDR_W1_FLAG_mf << std::dec);
                    ATH_MSG_DEBUG("\ttype: " << gtrack_w1.type/GTRACK_HDR_W1_TYPE_mf);
                    ATH_MSG_DEBUG("\tetaregion: " << gtrack_w1.eta_region/GTRACK_HDR_W1_ETA_REGION_mf);
                    ATH_MSG_DEBUG("\tphiregion: " << gtrack_w1.phi_region/GTRACK_HDR_W1_PHI_REGION_mf);
                    ATH_MSG_DEBUG("\tbinphi: " << gtrack_w1.phi_bin/GTRACK_HDR_W1_PHI_BIN_mf);
                    ATH_MSG_DEBUG("\tbinz: " << gtrack_w1.z_bin/GTRACK_HDR_W1_Z_BIN_mf);
                    ATH_MSG_DEBUG("\tsecondstage: " << gtrack_w1.second_stage/GTRACK_HDR_W1_SECOND_STAGE_mf);
                    ATH_MSG_DEBUG("\tlayerbitmask: " << gtrack_w1.layer_bitmask/GTRACK_HDR_W1_LAYER_BITMASK_mf);
                    ATH_MSG_DEBUG("\tscore: " << gtrack_w2.score/GTRACK_HDR_W2_SCORE_mf);
                    ATH_MSG_DEBUG("\td0: " << gtrack_w2.d0/GTRACK_HDR_W2_D0_mf);
                    ATH_MSG_DEBUG("\tz0: " << gtrack_w2.z0/GTRACK_HDR_W2_Z0_mf );
                    ATH_MSG_DEBUG("\tspare: " << gtrack_w2.spare/GTRACK_HDR_W2_SPARE_mf);
                    ATH_MSG_DEBUG("\tqoverpt: " << gtrack_w3.qoverpt/GTRACK_HDR_W3_QOVERPT_mf);
                    ATH_MSG_DEBUG("\tphi: " << gtrack_w3.phi/GTRACK_HDR_W3_PHI_mf);
                    ATH_MSG_DEBUG("\teta: " << gtrack_w3.eta/GTRACK_HDR_W3_ETA_mf);
                    ATH_MSG_DEBUG("\tspare: " << gtrack_w3.spare/GTRACK_HDR_W3_SPARE_mf);
                    gtracks_words.clear();
                    state = OutputConversion::FSM::GlobalHits;
                    break;
                }
                break;
        }
        case OutputConversion::FSM::Slices:
        {
            slices_words.push_back(word);
            SLICE_HDR_w1 slice_w1 = get_bitfields_SLICE_HDR_w1(slices_words[0]);
            
            ATH_MSG_DEBUG("Slices: ");
            ATH_MSG_DEBUG("\tflag: 0x" << std::hex << slice_w1.flag/GTRACK_HDR_W1_FLAG_mf << std::dec);
            ATH_MSG_DEBUG("\tsliceid: " << slice_w1.sliceid/SLICE_HDR_W1_SLICEID_mf);
            ATH_MSG_DEBUG("\tetaregion: " << slice_w1.eta_region/SLICE_HDR_W1_ETA_REGION_mf);
            ATH_MSG_DEBUG("\tphiregion: " << slice_w1.phi_region/SLICE_HDR_W1_PHI_REGION_mf);
            ATH_MSG_DEBUG("\tspare: " << slice_w1.spare/SLICE_HDR_W1_SPARE_mf);
            slices_words.clear();
            state = OutputConversion::FSM::Unknown;
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

StatusCode OutputConversionTool::decodePixelEDM(const std::vector<uint64_t> &bytestream,
                                                EFTrackingTransient::Metadata *metadata,
                                                EFTrackingTransient::PixelClusterAuxInput &pcAux) const
{
    ATH_MSG_DEBUG("Decoding pixel EDM");
    return decodeFPGAoutput(bytestream, metadata, &pcAux, nullptr, OutputConversion::FSM::PixelEDM);
}

StatusCode OutputConversionTool::decodeStripEDM(const std::vector<uint64_t> &bytestream,
                                                EFTrackingTransient::Metadata *metadata,
                                                EFTrackingTransient::StripClusterAuxInput &scAux) const
{
    ATH_MSG_DEBUG("Decoding strip EDM");
    return decodeFPGAoutput(bytestream, metadata, nullptr, &scAux, OutputConversion::FSM::StripEDM);
}

StatusCode OutputConversionTool::decodeSpacePoints(const std::vector<uint64_t> &bytestream,
                                                   EFTrackingTransient::Metadata *metadata) const
{
    ATH_MSG_DEBUG("Decoding space points");
    return decodeFPGAoutput(bytestream, metadata, nullptr, nullptr, OutputConversion::FSM::GlobalHits);
}

StatusCode OutputConversionTool::decodeSlices(const std::vector<uint64_t> &bytestream,
                                              EFTrackingTransient::Metadata *metadata) const
{
    ATH_MSG_DEBUG("Decoding slices");
    return decodeFPGAoutput(bytestream, metadata, nullptr, nullptr, OutputConversion::FSM::Slices);
}

StatusCode OutputConversionTool::decodeGTracks(const std::vector<uint64_t> &bytestream,
                                               EFTrackingTransient::Metadata *metadata) const
{
    ATH_MSG_DEBUG("Decoding tracks");
    return decodeFPGAoutput(bytestream, metadata, nullptr, nullptr, OutputConversion::FSM::GTracks);
}
