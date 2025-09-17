/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/Identifier.h"
#include <GeoModelKernel/throwExcept.h>
#include <cstddef>
#ifndef SIMULATIONBASE
#include <MuonReadoutGeometryR4/SpectrometerSector.h>
#include <Acts/Surfaces/PlaneSurface.hpp>
#include <Acts/Geometry/TrapezoidVolumeBounds.hpp>
#include <Acts/Geometry/Volume.hpp>
#include <ActsGeoUtils/NoDeletePtr.h>

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

#include <format>

namespace {
    std::vector<unsigned int> range (unsigned min, unsigned max) {
        assert(max > min);
        std::vector<unsigned int> v(max - min);
        std::iota(v.begin(), v.end(), min);
        return v;
    };
}

namespace MuonGMR4 {

using ChamberSet = SpectrometerSector::ChamberSet;
using BoundEnums = Acts::TrapezoidVolumeBounds::BoundValues;

SpectrometerSector::SpectrometerSector(defineArgs&& args): 
    AthMessaging("MuonSpectrometerSector"), m_args{std::move(args)} {
        for (auto & chamber : m_args.chambers) {
            chamber->setParent(this);
        }
    }

bool SpectrometerSector::operator<(const SpectrometerSector& other) const {
    if (side() != other.side()) {
        return side() < other.side();
    }
    if (sector() != other.sector()) {
        return sector() < other.sector();
    }
    if (other.chamberIndex() != chamberIndex()) {
        return chamberIndex() < other.chamberIndex();
    }
    return (*m_args.chambers.front()) < (*other.m_args.chambers.front()); 
}
int8_t SpectrometerSector::side() const {
    return m_args.chambers.front()->stationEta() > 0 ? 1 : -1;
}
const SpectrometerSector::defineArgs& SpectrometerSector::parameters() const { return m_args; }
const Muon::IMuonIdHelperSvc* SpectrometerSector::idHelperSvc() const { return m_args.chambers.front()->idHelperSvc();}
Muon::MuonStationIndex::ChIndex SpectrometerSector::chamberIndex() const { return m_args.chambers.front()->chamberIndex(); }
int SpectrometerSector::stationPhi() const { return m_args.chambers.front()->stationPhi(); }
int SpectrometerSector::sector() const {return m_args.chambers.front()->sector(); }
bool SpectrometerSector::barrel() const { return m_args.chambers.front()->barrel(); }
std::string SpectrometerSector::identString() const {
    return std::format("{:} {:}-side sector: {:2}",  
                       Muon::MuonStationIndex::chName(chamberIndex()), 
                       side() == 1 ? 'A' : 'C' , sector());
}
const ChamberSet& SpectrometerSector::chambers() const{ return m_args.chambers; }
const Acts::PlaneSurface& SpectrometerSector::surface() const {
    return *m_args.surface;
}
const Amg::Transform3D& SpectrometerSector::localToGlobalTrans(const ActsGeometryContext& gctx) const {
    return surface().transform(gctx.context());
}            
Amg::Transform3D SpectrometerSector::globalToLocalTrans(const ActsGeometryContext& gctx) const {
    return localToGlobalTrans(gctx).inverse(); 
}
double SpectrometerSector::halfXLong() const { return m_args.bounds->get(BoundEnums::eHalfLengthXposY); }
double SpectrometerSector::halfXShort() const { return m_args.bounds->get(BoundEnums::eHalfLengthXnegY); }
double SpectrometerSector::halfY() const { return  m_args.bounds->get(BoundEnums::eHalfLengthY); }
double SpectrometerSector::halfZ() const { return m_args.bounds->get(BoundEnums::eHalfLengthZ); }


std::shared_ptr<Acts::Volume> SpectrometerSector::boundingVolume(const ActsGeometryContext& gctx) const {
    return std::make_shared<Acts::Volume>(localToGlobalTrans(gctx), bounds());
}
std::shared_ptr<Acts::TrapezoidVolumeBounds> SpectrometerSector::bounds() const {
    return m_args.bounds;
}
Chamber::ReadoutSet SpectrometerSector::readoutEles() const {
    Chamber::ReadoutSet toReturn{};
    for (const ChamberPtr& ch : chambers()) {
        toReturn.insert(toReturn.end(), ch->readoutEles().begin(), ch->readoutEles().end());
    }
    return toReturn;
}
const std::vector<SpectrometerSector::chamberLocation> & SpectrometerSector::chamberLocations() const{
    return m_args.detectorLocs;

}
std::ostream& operator<<(std::ostream& ostr, 
                         const SpectrometerSector::defineArgs& args) {
    ostr<<std::endl
        <<"halfX (S/L): "<<args.bounds->get(BoundEnums::eHalfLengthXnegY)
        <<"/"<<args.bounds->get(BoundEnums::eHalfLengthXposY)<<" [mm], ";
    ostr<<"halfY: "<<args.bounds->get(BoundEnums::eHalfLengthY)<<" [mm], ";
    ostr<<"halfZ: "<<args.bounds->get(BoundEnums::eHalfLengthZ)<<" [mm], ";
    ostr<<"************************************************************************"<<std::endl;
    for (const SpectrometerSector::ChamberPtr& ch : args.chambers) {
        ostr<<" --- "<<(*ch)<<std::endl;
    }
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const SpectrometerSector& chamber) {
    ostr<<"MS sector "<<chamber.identString()<<" "<<chamber.parameters();
    return ostr;
}

const std::vector<unsigned int>& SpectrometerSector::logicalLayerIdx(const MuonReadoutElement* reEle) const{
    return m_detLayIdCache.at(reEle);
};

std::unordered_map<const MuonReadoutElement*, std::vector<unsigned int>> 
SpectrometerSector::fillDetLayIdCache() const{

    std::unordered_map<const MuonReadoutElement*, std::vector<unsigned int>> cache{};
    const ActsGeometryContext gctx{};
    const Amg::Transform3D sectorTrans = globalToLocalTrans(gctx);

    // sort the Readout elements by z in the sector fram
    Chamber::ReadoutSet reEleSorted {readoutEles()};
    std::ranges::sort(reEleSorted, [&sectorTrans, &gctx](const MuonReadoutElement* reEle1, const MuonReadoutElement* reEle2)-> bool {
        return  (sectorTrans * reEle1->center(gctx, reEle1->identify())).z() < (sectorTrans * reEle2->center(gctx, reEle2->identify())).z();
    });
    
    // this is a function to find the next logical layer, returning the corresponding reEle and Id
    auto nextLayer = [this, &cache, &reEleSorted](const Identifier& lastId) -> 
                    std::pair<const MuonReadoutElement*, const Identifier> {

        const MuonReadoutElement* nextReEle {nullptr};
        Identifier nextId {};

        for (const MuonReadoutElement* reEle : reEleSorted){

            // Check if this reEle has already been filled up
            if (cache.count(reEle)) continue;

            // compute the logical layer Id of the first layer of the reEle (first layer/gasgap, first channel, projecting to 
            // stationEta=1, doubletZ=1, doubletPhi=1, according to the technology)
            const Identifier logicalId {computeDetLayerId(reEle)};

            //if we find another measurement layer with the same detLayerId as the last we added (e.g. the same gasgap of two RPC z doublets)
            if (logicalId == lastId){
                return std::make_pair(reEle, logicalId);
            }

            // Save the next (subsequent) logic layer in z
            if (!nextReEle) {
                nextReEle = reEle;
                nextId = logicalId;
            }

        }
        return std::make_pair(nextReEle, nextId);
    };
 
    unsigned int layCounter {0}, nIter {0};
    Identifier lastId {};
    unsigned int deltaN {0};
    while(++nIter <= reEleSorted.size()){

        auto [nextReEle, nextId] = nextLayer(lastId);
        if (!nextReEle){
            THROW_EXCEPTION("Failed to retrieve next reEle in fillDetLayIdCache()");
        }

        unsigned int nLayers = nLayerPerReadout(nextReEle);
        
        if (nextId != lastId) {
            layCounter += deltaN;
            deltaN = nLayers;
            lastId = nextId;
        }
        else{
            deltaN = std::max(deltaN, nLayers);
        }
        
        cache[nextReEle] = range(layCounter, layCounter + nLayers);

        ATH_MSG_DEBUG( identString() << " ReEle: " << idHelperSvc()->toStringDetEl(nextReEle->identify()) << 
                     " Add logicLay: " << idHelperSvc()->toStringGasGap(nextId) << " nInserted, layCounter " << cache.at(nextReEle).size() << " ," << layCounter);
    }
    return cache;
};

Identifier SpectrometerSector::computeDetLayerId(const MuonReadoutElement* rele) const{

    switch (rele->detectorType()) {
        case ActsTrk::DetectorType::Mdt:{
            auto mdtRele = dynamic_cast<const MdtReadoutElement*>(rele);
            return idHelperSvc()->mdtIdHelper().channelID(mdtRele->stationName(), 1,
                                                           mdtRele->stationPhi(),
                                                           mdtRele->multilayer(),
                                                            1,1);
        }
        case ActsTrk::DetectorType::Rpc:{
            auto rpcRele = dynamic_cast<const RpcReadoutElement*>(rele);
            return idHelperSvc()->rpcIdHelper().channelID(rpcRele->stationName(), 1, 
                                                           rpcRele->stationPhi(),
                                                             rpcRele->doubletR(), 1, 1, 
                                                               1, 0, 1);
        }
        case ActsTrk::DetectorType::Tgc:{
            auto tgcRele = dynamic_cast<const TgcReadoutElement*>(rele);
            return idHelperSvc()->tgcIdHelper().channelID(tgcRele->stationName(), 1,
                                                           tgcRele->stationPhi(), 
                                                               1, 0, 1);
        }
        case ActsTrk::DetectorType::sTgc:{
           auto stgcRele = dynamic_cast<const sTgcReadoutElement*>(rele);
           return idHelperSvc()->stgcIdHelper().channelID(stgcRele->stationName(), 1,
                                                           stgcRele->stationPhi(), 
                                                           stgcRele->multilayer(),
                                                               1, 0, 1);
        }
        case ActsTrk::DetectorType::Mm:{
            auto mmRele = dynamic_cast<const MmReadoutElement*>(rele);
            return idHelperSvc()->mmIdHelper().channelID(mmRele->stationName(), 1,
                                                          mmRele->stationPhi(), 
                                                          mmRele->multilayer(),
                                                              1, 1);
        }
        default:
            THROW_EXCEPTION("Unexpected Readout Element Type in computeDetLayerId()");
    }
}

unsigned int SpectrometerSector::nLayerPerReadout (const MuonReadoutElement* rele) const {

    switch (rele->detectorType()) {
        case ActsTrk::DetectorType::Mdt:{
            auto mdtRele = dynamic_cast<const MdtReadoutElement*>(rele);
            return mdtRele->numLayers();
        }
        case ActsTrk::DetectorType::Rpc:{
            auto rpcRele = dynamic_cast<const RpcReadoutElement*>(rele);
            return rpcRele->nGasGaps();
        }
        case ActsTrk::DetectorType::Tgc:{
            auto tgcRele = dynamic_cast<const TgcReadoutElement*>(rele);
            return tgcRele->nGasGaps();
        }
        case ActsTrk::DetectorType::sTgc:{
           auto stgcRele = dynamic_cast<const sTgcReadoutElement*>(rele);
            return stgcRele->numLayers();
        }
        case ActsTrk::DetectorType::Mm:{
            auto mmRele = dynamic_cast<const MmReadoutElement*>(rele);
            return mmRele->nGasGaps();
        }
        default:
            THROW_EXCEPTION("Unexpected Readout Element Type in nLayerPerReadout()");
    }
}

}

#endif
