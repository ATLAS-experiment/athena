/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCondAlg/RpcCondDbAlg.h"

#include "AthenaKernel/IOVInfiniteRange.h"

// constructor
RpcCondDbAlg::RpcCondDbAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthCondAlgorithm(name, pSvcLocator) {}

// Initialize
StatusCode RpcCondDbAlg::initialize() {
    ATH_MSG_DEBUG("initializing " << name());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_readKey_folder_mc_deadElements.initialize(!m_readKey_folder_mc_deadElements.empty()));
    return StatusCode::SUCCESS;
}

template <class WriteCont>
    StatusCode RpcCondDbAlg::addCondDependency(const EventContext& ctx,
                                               const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                                               SG::WriteCondHandle<WriteCont>& writeHandle) const {
        if (key.empty()) {
            return StatusCode::SUCCESS;
        }
        SG::ReadCondHandle<CondAttrListCollection> readCondHandle{key, ctx};
        ATH_CHECK(readCondHandle.isValid());
        writeHandle.addDependency(readCondHandle);
        return StatusCode::SUCCESS;
    }

// execute
StatusCode RpcCondDbAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("execute " << name());

    // launching Write Cond Handle
    SG::WriteCondHandle<RpcCondDbData> writeHandle{m_writeKey, ctx};
    if (writeHandle.isValid()) {
        ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                                    << " In theory this should not be called, but may happen"
                                    << " if multiple concurrent events are being processed out of order.");
        return StatusCode::SUCCESS;
    }
    writeHandle.addDependency(IOVInfiniteRange::infiniteMixed());
    ATH_CHECK(addCondDependency(ctx, m_readKey_folder_mc_deadElements, writeHandle));

    std::unique_ptr<RpcCondDbData> writeCdo{std::make_unique<RpcCondDbData>()};
    // data and MC
    ATH_CHECK(loadMcElementStatus(ctx, *writeCdo));

    ATH_CHECK(writeHandle.record(std::move(writeCdo)));
    ATH_MSG_DEBUG("Recorded new " << writeHandle.key() << " with range " << writeHandle.getRange() << " into Conditions Store");

    return StatusCode::SUCCESS;
}
// loadMcElementStatus
StatusCode RpcCondDbAlg::loadMcElementStatus(const EventContext& ctx,  RpcCondDbData& writeCdo) const {
    SG::ReadCondHandle<CondAttrListCollection> readCdo{m_readKey_folder_mc_deadElements, ctx};
    ATH_CHECK(readCdo.isValid());

    ATH_MSG_DEBUG("Size of CondAttrListCollection " << readCdo.fullKey() << " readCdo->size()= " << readCdo->size());

    CondAttrListCollection::const_iterator itr;
    unsigned int chan_index = 0;
    unsigned int iFracDeadStrip = 0;
    for (itr = readCdo->begin(); itr != readCdo->end(); ++itr) {
        const coral::AttributeList& atr = itr->second;
        CondAttrListCollection::ChanNum channum = itr->first;
        Identifier chamberId = Identifier(channum);

        std::string eff_panel, striplist, eff;

        eff_panel = *(static_cast<const std::string*>((atr["PanelRes"]).addressOfData()));
        striplist = *(static_cast<const std::string*>((atr["StripStatus"]).addressOfData()));

        ATH_MSG_DEBUG("-----------------------------entry #" << chan_index);
        ATH_MSG_DEBUG("channel ID = Panel ID " << channum << " as identif. = " << m_idHelperSvc->toString(chamberId));
        ATH_MSG_DEBUG("eff_panel load is " << eff_panel);
        ATH_MSG_DEBUG("striplist load is " << striplist << " " << striplist.size());

        // Efficiencies and Cluster Sizes
        constexpr std::string_view delimiter{" "};
        const auto info_panel = CxxUtils::tokenize(eff_panel, delimiter);

        int DBversion = CxxUtils::atoi(info_panel[0]);

        int npanelstrip = CxxUtils::atoi(info_panel[2]);

        double ProjectedTracks = CxxUtils::atof(info_panel[1]);
        writeCdo.setProjectedTrack(chamberId, ProjectedTracks);

        double Efficiency = CxxUtils::atof(info_panel[3]);
        writeCdo.setEfficiency(chamberId, Efficiency);


        double GapEfficiency = CxxUtils::atof(info_panel[5]);
        writeCdo.setGapEfficiency(chamberId, GapEfficiency);

        double MeanClusterSize = CxxUtils::atof(info_panel[17]);
        writeCdo.setMeanClusterSize(chamberId, MeanClusterSize);

        if (DBversion > 2) {
            double FracClusterSize1 = CxxUtils::atof(info_panel[19]) + CxxUtils::atof(info_panel[20]) * 10000;
            writeCdo.setFracClusterSize1(chamberId, FracClusterSize1);

            double FracClusterSize2 = CxxUtils::atof(info_panel[21]) + CxxUtils::atof(info_panel[22]) * 10000;
            writeCdo.setFracClusterSize2(chamberId, FracClusterSize2);
        } else {
            if (info_panel.size() > 20) {
                double FracClusterSize1 = CxxUtils::atof(info_panel[19]);
                writeCdo.setFracClusterSize1(chamberId, FracClusterSize1);

                double FracClusterSize2 = CxxUtils::atof(info_panel[20]);
                writeCdo.setFracClusterSize2(chamberId, FracClusterSize2);
            } else {
                writeCdo.setFracClusterSize1(chamberId, 0.6);
                writeCdo.setFracClusterSize2(chamberId, 0.2);
                ATH_MSG_DEBUG("Panel with incomplete info in the DB, size = " << info_panel.size() << " instead of required >20");
                ATH_MSG_DEBUG("PanelId = " << channum << " = " << m_idHelperSvc->toString(chamberId));
                ATH_MSG_DEBUG("Cluster Size 1 and 2 fractions are set to 0.6 and 0.2 for this chamber.");
            }
        }

        // strip status
        // n chars = #strips (status between 0--9)
        int countdeadstrip = 0;
        int countdeadstripinfidarea = 0;
        int countpanelstrip = 0;

        // update for the timing and error on timing
        // new info strip |status time error_on_time|
        constexpr std::string_view delimiter_strip{"|"};
       

        const auto info_strip= CxxUtils::tokenize(striplist, delimiter_strip);
        if (info_strip.size() > 1) {
            for (unsigned int i = 0; i < info_strip.size(); ++i) {
                const std::string &ch_strip2 = info_strip[i];

                constexpr std::string_view delimiter_strip2{" "};

                auto info_strip2 = CxxUtils::tokenize(ch_strip2, delimiter_strip2);

                double Time = CxxUtils::atof(info_strip2[1]);
                double SigmaTime = CxxUtils::atof(info_strip2[2]);
                const auto &strip_status = info_strip2[0];


                Identifier strip_id;
                CondAttrListCollection::ChanNum stripnum;
                stripnum = channum + i * 4;
                strip_id = channum + i * 4;

                ATH_MSG_DEBUG("strip " << strip_id << " has time " << Time << " and " << SigmaTime);

                writeCdo.setStripTime(strip_id, std::vector<double>{Time, SigmaTime});

                ATH_MSG_VERBOSE("strip #" << i + 1 << " strip_id " << stripnum << " expanded "
                                          << m_idHelperSvc->toString(strip_id));

                ++countpanelstrip;

                if (strip_status[0] == '0') {                  
                    ++countdeadstrip;
                    if (i > 1 && i < info_strip.size() - 2) {
                        // strip in the fiducial area for the efficiency measurement
                        ++countdeadstripinfidarea;
                    }
                }
            }
        } else {
            ATH_MSG_DEBUG("no timing info");

            for (unsigned int i = 0; i < striplist.size(); i++) {
                char part_strip = striplist[i];
                
                char ch_panel = part_strip;


                CondAttrListCollection::ChanNum stripnum = channum + i * 4;
                Identifier strip_id{channum + i * 4};

                ATH_MSG_VERBOSE("strip #" << i + 1 << " info_strip " << part_strip << " strip_id " << stripnum << " expanded "
                                          << m_idHelperSvc->toString(strip_id) << " panel = " << ch_panel);

                ++countpanelstrip;

                if (part_strip == '0') {
                    ++countdeadstrip;                  
                    if (i > 1 && i < striplist.size() - 2) {
                        // strip in the fiducial area for the efficiency measurement
                        ++countdeadstripinfidarea;
                    }
                }
            }
        }

        if (countpanelstrip != npanelstrip)
            ATH_MSG_WARNING(
                "WARNING (no side effects for this, just a reminder for a proper fix of the DB content) no matching strip number!!! "
                << countpanelstrip << " != " << npanelstrip << " Identifier: " << channum);

        float FracDeadStripMap = 0;
        if (countpanelstrip - 4 > 0) FracDeadStripMap = float(countdeadstripinfidarea) / float(countpanelstrip - 4);

        // store in the suitabel maps
        writeCdo.setFracDeadStrip(chamberId, FracDeadStripMap);
        ++iFracDeadStrip;

        std::stringstream ss;
        if (msgLvl(MSG::DEBUG)) {
            ss << "Size of RPC_PanelFracDeadStripMap " << iFracDeadStrip << "; in panel ";
            ss << channum << " FracDeadStri(in fid.area) " << FracDeadStripMap << " (incl. borders) ";
            if (countpanelstrip == 0)
                ss << "DIVISION BY ZERO IMPOSSIBLE";
            else
                ss << float(countdeadstrip) / float(countpanelstrip);
            ss << " nDeadStrips,InFidArea/nStrips " << countdeadstrip << ",";
            ss << countdeadstripinfidarea << "/" << countpanelstrip;
            ATH_MSG_DEBUG(ss.str());
        }

        ATH_MSG_DEBUG("Efficiency is " << Efficiency << " and fraction is " << FracDeadStripMap << " and thus "
                                       << Efficiency - (0.99 - FracDeadStripMap));

        if (msgLvl(MSG::DEBUG) && Efficiency - (0.99 - FracDeadStripMap) > 0. && 
                                  (Efficiency < 0.995 || FracDeadStripMap > 0.01) && (Efficiency > 0.005 || FracDeadStripMap < 0.99)) {
            std::stringstream msg;
            msg << "WARNING: Inconsistent panel eff.=" << Efficiency << " and 0.99-dead_frac=" << 0.99 - FracDeadStripMap
                << " nDeadStrips,InFidArea/nStrips " << countdeadstrip << "," << countdeadstripinfidarea << "/" << countpanelstrip
                << " for panelId=" << m_idHelperSvc->toString(chamberId);
            if (Efficiency - (0.99 - FracDeadStripMap) > 0.2)
                msg << " difference >0.2";
            else if (Efficiency - (0.99 - FracDeadStripMap) > 0.1)
                msg << " difference >0.1";
            else if (Efficiency - (0.99 - FracDeadStripMap) > 0.05)
                msg << " difference >0.05";
            else if (Efficiency - (0.99 - FracDeadStripMap) > 0.025)
                msg << " difference >0.025";
            else if (Efficiency - (0.99 - FracDeadStripMap) > 0.01)
                msg << " difference >0.01";
            else
                msg << " difference >0 but <=0.01";
            ATH_MSG_DEBUG(msg.str());
        }
       
    }
    return StatusCode::SUCCESS;
}
