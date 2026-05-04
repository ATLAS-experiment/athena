/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexByteStreamTool  -  description
//                              -------------------
//     begin                : 11 05 2022
//     email                : will@cern.ch
//  ***************************************************************************/

#include "eFexByteStreamTool.h"
#include "CxxUtils/span.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/Status.h"

#include "xAODTrigL1Calo/eFexTowerAuxContainer.h"
#include "xAODTrigger/eFexEMRoIAuxContainer.h"
#include "xAODTrigger/eFexTauRoIAuxContainer.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "bytestreamDecoder/L1CaloRdoEfexTob.h"
#include "bytestreamDecoder/L1CaloRdoEfexTower.h"
#include "bytestreamDecoder/L1CaloBsDecoderUtil.h"
#include "bytestreamDecoder/L1CaloRdoRodInfo.h"
#include "bytestreamDecoder/L1CaloBsDecoderRun3.h"

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

namespace {
    // Helper method for creating the containers
    template<typename T, typename W>
    StatusCode addContainer(
            std::map <std::pair<L1CaloRdoFexTob::TobSource, bool>, SG::WriteHandle<T>> &map,
            L1CaloRdoFexTob::TobSource source, bool inTime, const SG::WriteHandleKey <T> &handle,
            const EventContext &ctx) {
        if (!handle.empty()) {
            return map.emplace(std::make_pair(source, inTime),
                               SG::WriteHandle<T>(handle, ctx)).first->second.record(
                    std::make_unique<T>(), std::make_unique<W>());
        }
        return StatusCode::SUCCESS;
    }
}

eFexByteStreamTool::eFexByteStreamTool(const std::string& type,
        const std::string& name,
        const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode eFexByteStreamTool::initialize() {

    // Initialise eEM data handle keys
    ATH_CHECK(m_eEMWriteKey.initialize(!m_eEMWriteKey.empty()));

    // Initialise eTAU data handle keys
    ATH_CHECK(m_eTAUWriteKey.initialize(!m_eTAUWriteKey.empty()));

    // write keys for xTOBs
    ATH_CHECK(m_eEMxWriteKey.initialize(!m_eEMxWriteKey.empty()));
    ATH_CHECK(m_eTAUxWriteKey.initialize(!m_eTAUxWriteKey.empty()));

    // multislice TOBs
    ATH_CHECK(m_eEMSliceWriteKey.initialize(!m_eEMSliceWriteKey.empty()));
    ATH_CHECK(m_eTAUSliceWriteKey.initialize(!m_eTAUSliceWriteKey.empty()));

    // Initialize eTower handle key
    ATH_CHECK(m_eTowerWriteKey.initialize(!m_eTowerWriteKey.empty()));

    m_decoder = std::make_unique<L1CaloBsDecoderRun3>();

    // Initialize monitoring tool if not empty
    if (!m_monTool.empty()) {
        ATH_CHECK(m_monTool.retrieve());
        m_decoder->setLogger( std::make_unique<MonitoredLogging>(m_monTool) );
        ATH_MSG_INFO("Logging errors to " << m_monTool.name() << " monitoring tool");
    }


    return StatusCode::SUCCESS;
}

void eFexByteStreamTool::MonitoredLogging::err(const std::string& location, const std::string& title, const std::string&) const {

    Monitored::Group(m_monTool,
                     Monitored::Scalar("efexDecoderErrorLocation",std::string("lb=") + std::to_string(Gaudi::Hive::currentContext().eventID().lumi_block()) + "," + (location.empty() ? std::string("UNKNOWN") : location)),
                     Monitored::Scalar("efexDecoderErrorTitle",title.empty() ? std::string("UNKNOWN") : title)
                     );

    Monitored::Group(m_monTool, Monitored::Scalar("lbn",Gaudi::Hive::currentContext().eventID().lumi_block()),
                     Monitored::Scalar("decoderError",(title.empty() ? std::string("UNKNOWN") : title) + ":" + (location.empty() ? std::string("UNKNOWN") : location))
    );
}

// BS->xAOD conversion
StatusCode eFexByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf, const EventContext& ctx) const {

    // indices are source (tob or xtob) and bool for in/out of time
    std::map<std::pair<L1CaloRdoFexTob::TobSource,bool>,SG::WriteHandle<xAOD::eFexEMRoIContainer>> eContainers;
    std::map<std::pair<L1CaloRdoFexTob::TobSource,bool>,SG::WriteHandle<xAOD::eFexTauRoIContainer>> tContainers;

    bool multislice=false;

    if (!m_eEMWriteKey.empty()) {
        ATH_CHECK(StatusCode(addContainer<xAOD::eFexEMRoIContainer, xAOD::eFexEMRoIAuxContainer>(
                eContainers, L1CaloRdoFexTob::TobSource::EfexTob, false, m_eEMWriteKey, ctx)));
    }
    if (!m_eTAUWriteKey.empty()) {
        ATH_CHECK(StatusCode(addContainer<xAOD::eFexTauRoIContainer, xAOD::eFexTauRoIAuxContainer>(
                tContainers, L1CaloRdoFexTob::TobSource::EfexTob, false, m_eTAUWriteKey, ctx)));
    }
    if (!m_eEMxWriteKey.empty()) {
        ATH_CHECK( StatusCode(addContainer<xAOD::eFexEMRoIContainer,xAOD::eFexEMRoIAuxContainer>(
                eContainers,L1CaloRdoFexTob::TobSource::EfexXtob,false,m_eEMxWriteKey,ctx)) );
    }
    if (!m_eTAUxWriteKey.empty()) {
        ATH_CHECK( StatusCode(addContainer<xAOD::eFexTauRoIContainer,xAOD::eFexTauRoIAuxContainer>(
                tContainers,L1CaloRdoFexTob::TobSource::EfexXtob,false,m_eTAUxWriteKey,ctx)) );
    }
    if (!m_eEMSliceWriteKey.empty()) {
        ATH_CHECK( StatusCode(addContainer<xAOD::eFexEMRoIContainer,xAOD::eFexEMRoIAuxContainer>(
                eContainers,L1CaloRdoFexTob::TobSource::EfexXtob,true,m_eEMSliceWriteKey,ctx)) );
        multislice=true;
    }
    if (!m_eTAUSliceWriteKey.empty()) {
        ATH_CHECK( StatusCode(addContainer<xAOD::eFexTauRoIContainer,xAOD::eFexTauRoIAuxContainer>(
                tContainers,L1CaloRdoFexTob::TobSource::EfexXtob,true,m_eTAUSliceWriteKey,ctx)) );
        multislice=true;
    }


    SG::WriteHandle<xAOD::eFexTowerContainer> eTowers;
    if(!m_eTowerWriteKey.empty()) {
        eTowers = SG::WriteHandle<xAOD::eFexTowerContainer>(m_eTowerWriteKey,ctx);
        ATH_CHECK( eTowers.record(std::make_unique<xAOD::eFexTowerContainer>(),std::make_unique<xAOD::eFexTowerAuxContainer>()) );
    }

    std::list<L1CaloRdoRodInfo> rodInfos;
    std::list<L1CaloRdoEfexTob> efexTobs;
    std::list<L1CaloRdoEfexTower> efexTowers;
    L1CaloBsDecoderRun3& decoder = *m_decoder;
    std::map<std::tuple<int,int,int,int,int>,size_t> towerMap; // maps {crate,module,fpga,eta,phi} onto index in vector
    for (const ROBF* rob : vrobf) {
        // Iterate over ROD words and decode
        ATH_MSG_DEBUG("Decoding " << rob->rod_ndata() << " ROD words from ROB 0x" << std::hex << rob->rob_source_id() << std::dec);
        L1CaloBsDecoderUtil::decodeRodInfo( rob, rodInfos );

        if(rob->rod_ndata()==0) continue;

        CxxUtils::span data{rob->rod_data(), rob->rod_ndata()};
        auto lastRod = rodInfos.end(); lastRod--;

        if ( (rob->rob_source_id() >> 16) == eformat::TDAQ_CALO_FEAT_EXTRACT_DAQ && !m_eTowerWriteKey.empty() ) {
            // decoding raw input data
            // tower et counts should be in order: PS, L1, L2, L3, Had
            // towers with all et counts 0 will still be zero-suppressed
            efexTowers.clear();
            decoder.decodeEfexData(data.begin(), data.end(), efexTowers, lastRod );
            for(auto& t : efexTowers) {
                if (t.getLayer() != 0) continue; // do hadronic in next loop
                towerMap[std::make_tuple(t.getCrate(),t.getModule(),t.getFpgaNumber(),t.getRegion().getEtaIndex(),t.getRegion().getPhiIndex())] = eTowers->size();
                eTowers->push_back( std::make_unique<xAOD::eFexTower>() );
                // in bytestream cell orders are L2,PS,L1,L3 so reorder to usual PS,L1,L2,L3 order
                std::vector<uint16_t> counts(11,xAOD::eFexTower::c_missingCountCode);
                counts[0] = t.getSupercells().at(4);
                for(size_t idx = 0;idx<4;idx++) {
                    counts[idx+1] = t.getSupercells().at(idx+5); // L1
                    counts[idx+5] = t.getSupercells().at(idx); // L2
                }
                counts[9] = t.getSupercells().at(9);

                eTowers->back()->initialize(t.getRegion().getEtaIndex()*0.1 + 0.05,2.*ROOT::Math::Pi()*(0.5 + t.getRegion().getPhiIndex() - 64*(t.getRegion().getPhiIndex()>=32))/64,
                                        counts,
                                        t.getModule() + t.getCrate()*12,
                                        t.getFpgaNumber(),
                                        t.getFlag(),0 /* hadronic status flag */);
            }

            for(auto& t : efexTowers) {
                if (t.getLayer() == 0) continue; // do hadronic towers now ... add to existing towers
                size_t idx = eTowers->size();
                if(auto itr = towerMap.find({t.getCrate(), t.getModule(), t.getFpgaNumber(),t.getRegion().getEtaIndex(),t.getRegion().getPhiIndex()}); itr != towerMap.end()) {
                    idx = itr->second;
                } else {
                    // possible that ecal tower was zero-suppressed but hcal has energy, so create such a tower
                    towerMap[std::make_tuple(t.getCrate(),t.getModule(),t.getFpgaNumber(),t.getRegion().getEtaIndex(),t.getRegion().getPhiIndex())] = eTowers->size();
                    eTowers->push_back( std::make_unique<xAOD::eFexTower>() );
                    eTowers->back()->initialize(t.getRegion().getEtaIndex()*0.1 + 0.05,2.*ROOT::Math::Pi()*(0.5 + t.getRegion().getPhiIndex() - 64*(t.getRegion().getPhiIndex()>=32))/64,
                                                std::vector<uint16_t>(11,xAOD::eFexTower::c_missingCountCode),
                                                t.getModule() + t.getCrate()*12,t.getFpgaNumber(),0,t.getFlag());
                }
                auto tower = eTowers->at(idx);
                tower->setHad_status(t.getFlag());
                auto et_count = tower->et_count();
                et_count.at(10) = t.getValue();
                tower->setEt_count(et_count);
            }



        } else if ( (rob->rob_source_id() >> 16) == eformat::TDAQ_CALO_FEAT_EXTRACT_ROI ) {
            // decoding tobs
            efexTobs.clear();
            decoder.decodeEfexTobs(data.begin(), data.end(), efexTobs, lastRod);

            for (const L1CaloRdoEfexTob &tob: efexTobs) {
                // loop over slices ... create separate tobs for each, where there's a word defined
                for (size_t slice = 0; slice < tob.numSlices(); slice++) {
                    if (!multislice && int(slice) != tob.getL1aPos())
                        continue; // ignore out-of-time slices if multiSlice option = false
                    if (tob.getWord0(slice) == 0) continue; // this tob isn't in this slice
                    if (tob.getTobType() == L1CaloRdoFexTob::TobType::EM) {
                        auto cont = eContainers.find({tob.getTobSource(), int(slice) != tob.getL1aPos()});
                        if (cont == eContainers.end()) continue; // not writing this tob collection
                        cont->second->push_back(std::make_unique<xAOD::eFexEMRoI>());
                        if (tob.getTobSource() == L1CaloRdoFexTob::TobSource::EfexTob) {
                            cont->second->back()->initialize(tob.getModule(), tob.getCrate(), tob.getWord0(slice));
                        } else {
                            cont->second->back()->initialize(tob.getWord0(slice), tob.getWord1(slice));
                        }
                    } else if (tob.getTobType() == L1CaloRdoFexTob::TobType::Tau) {
                        auto cont = tContainers.find({tob.getTobSource(), int(slice) != tob.getL1aPos()});
                        if (cont == tContainers.end()) continue; // not writing this tob collection
                        cont->second->push_back(std::make_unique<xAOD::eFexTauRoI>());
                        if (tob.getTobSource() == L1CaloRdoFexTob::TobSource::EfexTob) {
                            cont->second->back()->initialize(tob.getModule(), tob.getCrate(), tob.getWord0(slice));
                        } else {
                            cont->second->back()->initialize(tob.getWord0(slice), tob.getWord1(slice));
                        }
                    }
                } // timeslice loop
            } // tob loop
        } // tob data if block
    } // fragment loop

    if(msgLevel(MSG::DEBUG)) {
        std::stringstream msg;
        for (auto&[k, v]: eContainers) {
            msg << v->size() << " " << (k.second ? "out-of-time " : "in-time ") << "eg" <<
                (k.first == L1CaloRdoFexTob::TobSource::EfexTob ? "" : "x") << "TOB, ";
        }
        for (auto&[k, v]: tContainers) {
            msg << v->size() << " " << (k.second ? "out-of-time " : "in-time ") << "tau" <<
                (k.first == L1CaloRdoFexTob::TobSource::EfexTob ? "" : "x") << "TOB, ";
        }
        ATH_MSG_DEBUG("Decoded: " << msg.str());
        if (!m_eTowerWriteKey.empty()) {
            ATH_MSG_DEBUG("Decoded: " << eTowers->size() << " eTowers");
        }
    }
    // this is how to print the in-time em xTOBs:
//    for(const auto& tob : *(eContainers[{L1CaloRdoFexTob::TobSource::EfexXtob,false}])) {
//        std::cout << tob->eFexNumber() << "." << tob->fpga() << ": " << tob->fpgaEta() << " " << tob->fpgaPhi() << " " << tob->et() << " " << tob->etXTOB() << " " << tob->etTOB() << " " << std::endl;
//    }

    return StatusCode::SUCCESS;
}

namespace Decoder {
    struct Tob {
        uint32_t word0;
    };
    struct xTob {
        uint32_t word0;
        uint32_t word1;
    };
    struct Slice {
        struct Id {
            uint32_t sliceNumber = 0;
            uint32_t fpgaNumber = 0;
            bool operator<(const Id& r) const {
                return (fpgaNumber<r.fpgaNumber || (fpgaNumber==r.fpgaNumber && sliceNumber<r.sliceNumber));
            }
        };
        void addTob(const xAOD::eFexEMRoI& tob) {
            if(tob.isTOB()) {
                tobs.push_back({.word0=tob.word0()});
            } else {
                em_xtobs.push_back({.word0=tob.word0(),.word1=tob.word1()});
            }
        }
        void addTob(const xAOD::eFexTauRoI& tob) {
            if(tob.isTOB()) {
                tobs.push_back({.word0=tob.word0()});
            } else {
                tau_xtobs.push_back({.word0=tob.word0(),.word1=tob.word1()});
            }
        }
        std::vector<uint32_t> getWords(const Id& id) {

            const uint32_t tobType = (id.fpgaNumber==1) ? 1 : 0; // only fpga1 produce tau tobs
            const uint32_t numTobs = tobs.size();
            const uint32_t numEmXtobs = em_xtobs.size();
            const uint32_t numTauXtobs = tau_xtobs.size();
            const uint32_t safeMode=0; // tob/xtob data not suppressed

            std::vector<uint32_t> out;
            out.reserve(numTobs + numEmXtobs*2+numTauXtobs*2 + 1 + (numTobs%2 ? 0 : 1)); // last part to ensure multiple of 64bit
            for(auto& tob : tobs) out.push_back(tob.word0);
            for(auto& xtob : em_xtobs) {out.push_back(xtob.word0);out.push_back(xtob.word1);}
            for(auto& xtob : tau_xtobs) {out.push_back(xtob.word0);out.push_back(xtob.word1);}
            if(numTobs%2==0) out.push_back(0); // padding word

            uint32_t sliceTrailer = 0;
            sliceTrailer += (tobType&0x1)<<8;
            sliceTrailer += (numTobs&0x7)<<9;
            sliceTrailer += (numEmXtobs&0x3f)<<12;
            sliceTrailer += (numTauXtobs&0x3f)<<18;
            sliceTrailer += (id.sliceNumber&0x7)<<24;
            sliceTrailer += (safeMode&0x1)<<27;
            sliceTrailer += (id.fpgaNumber&0x3)<<28;

            out.push_back(sliceTrailer);
            return out;
        
        }
        std::vector<xTob> em_xtobs;
        std::vector<xTob> tau_xtobs;
        std::vector<Tob> tobs;
    };
    struct Module {
        struct Id {
            uint32_t shelfNumber = 0;
            uint32_t efexNumber = 0;
            bool operator<(const Id& r) const {
                return (shelfNumber<r.shelfNumber || (shelfNumber==r.shelfNumber && efexNumber<r.efexNumber));
            }
        };
        void addTob(const xAOD::eFexTauRoI& tob, uint32_t sliceNumber) {
            const uint32_t fpgaNumber = (tob.isTOB()) ? 1 : tob.fpga(); // must put emTobs in fpga0, tauTobs in fpga1
            slices[{.sliceNumber=sliceNumber,.fpgaNumber=fpgaNumber}].addTob(tob);
        }
        void addTob(const xAOD::eFexEMRoI& tob, uint32_t sliceNumber) {
            const uint32_t fpgaNumber = (tob.isTOB()) ? 0 : tob.fpga(); // must put emTobs in fpga0, tauTobs in fpga1
            slices[{.sliceNumber=sliceNumber,.fpgaNumber=fpgaNumber}].addTob(tob);
        }
        std::vector<uint32_t> getWords(const Id& id, int numSlices) {
            std::vector<uint32_t> out;

            for(auto& [thisId,slice] : slices) {
                auto sliceWords = slice.getWords(thisId);
                out.insert(out.end(), sliceWords.begin(), sliceWords.end());
            }

            const size_t efexBlockSize = out.size(); // total size

            uint32_t efexTrailer1 = 0;
            efexTrailer1 += (efexBlockSize&0xfff);
            efexTrailer1 += (id.efexNumber&0xf)<<12;
            efexTrailer1 += (id.shelfNumber&0x1)<<16;
            efexTrailer1 += (numSlices&0xf)<<24;

            uint32_t efexTrailer2 = 0; // should really compute CRC and put in 20 MSBs

            out.push_back(efexTrailer1);
            out.push_back(efexTrailer2);
            return out;
        }
        std::map<Slice::Id,Slice> slices;
    };
    struct Fragment {
        int numSlices = 1;
        template<typename T> void addTob(const T& tob, int sliceNumber) {
            const uint32_t shelfNumber = tob.shelfNumber();
            const uint32_t efexNumber = tob.eFexNumber();
            modules[{.shelfNumber=shelfNumber,.efexNumber=efexNumber}].addTob(tob,sliceNumber);
        }
        std::vector<uint32_t> getWords(uint32_t shelfNumber) {
            std::vector<uint32_t> out;
            for(auto& [id,module] : modules) {
                if(id.shelfNumber!=shelfNumber) continue;
                auto words = module.getWords(id,numSlices);
                out.insert(out.end(),words.begin(),words.end());
            }
            // add the rod trailer (the bits that matter, at least)
            uint32_t rodTrailer1 = 0;
            uint32_t rodTrailer2 = 0;

            rodTrailer1 += (out.size()&0xffff);

            out.push_back(rodTrailer1);
            out.push_back(rodTrailer2);
            return out;
        }
        std::map<Module::Id,Module> modules = {};
    };
}

StatusCode eFexByteStreamTool::convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf,
                                           const xAOD::TrigCompositeContainer* tc,
                                           const EventContext& eventContext) {

    // multislice encoding is not yet implemented
    // to do would need to decide how many slices have been read out (not guaranteed determinable from the out-of-time tobs)
    // then need to set the sliceNumber appropriately for central TOBs

    Decoder::Fragment f{.numSlices=1};
    const int centralSlice = f.numSlices / 2; // central slice index; integer division, evaluates to 0 when numSlices=1

    if (tc && !tc->empty()) {
        const xAOD::TrigComposite* l1 = tc->at(0);
        for (const std::string& name : l1->getObjectNames<xAOD::eFexEMRoIContainer>()) {
            auto link = l1->objectLink<xAOD::eFexEMRoIContainer>(name);
            if (link.isValid()) {
                if (const xAOD::eFexEMRoIContainer* cont = link.getStorableObjectPointer()) {
                    for (const xAOD::eFexEMRoI_v1* tob : *cont) {
                        f.addTob(*tob, centralSlice);
                    }
                }
            }
        }
        for (const std::string& name : l1->getObjectNames<xAOD::eFexTauRoIContainer>()) {
            auto link = l1->objectLink<xAOD::eFexTauRoIContainer>(name);
            if (link.isValid()) {
                if (const xAOD::eFexTauRoIContainer* cont = link.getStorableObjectPointer()) {
                    for (const xAOD::eFexTauRoI_v1* tob : *cont) {
                        f.addTob(*tob, centralSlice);
                    }
                }
            }
        }
    }

    // clearCache, newRodData, and newRobFragment defined in IL1TriggerByteStreamTool
    // Allocate memory
    clearCache(eventContext);

    static const std::vector<uint32_t> mids = {0x1000,0x1100}; // 0x1000,0x1100 are the two shelves
    for(uint32_t i=0;i<2;i++) {
        auto words = f.getWords(i);
        uint32_t* data = newRodData(eventContext, words.size());
        std::copy( words.begin(), words.end(), data); // transfer words to cache/reserved array

        // Create ROB fragment
        uint32_t moduleid = mids[i];
        eformat::helper::SourceIdentifier sid(eformat::TDAQ_CALO_FEAT_EXTRACT_ROI, moduleid);
        vrobf.push_back(newRobFragment(eventContext, sid.code(), words.size(), data, 0/*detEvtType??*/));
    }
    return StatusCode::SUCCESS;
}

