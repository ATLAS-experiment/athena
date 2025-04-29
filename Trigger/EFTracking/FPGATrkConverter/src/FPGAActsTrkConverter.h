/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrkConverter_FPGAACTSTRKCONVERTER__H
#define FPGATrkConverter_FPGAACTSTRKCONVERTER__H 1

#include "FPGATrkConverterInterface/IFPGAActsTrkConverter.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"

class PixelID;
class SCT_ID;

class FPGAActsTrkConverter : public extends<AthAlgTool,IFPGAActsTrkConverter> {
  public:
    
    FPGAActsTrkConverter(const std::string& type, 
		const std::string& name,
		const IInterface* parent);
    virtual ~FPGAActsTrkConverter() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks,
                  const std::vector<std::vector<FPGATrackSimHit>>& hitsInRoads,
                  const std::vector<FPGATrackSimRoad>& roads) const override final; 
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks,
                  const std::vector<FPGATrackSimTrack>& tracks) const override final; 
    protected:
    std::unique_ptr<Acts::BoundTrackParameters> makeParams (const FPGATrackSimRoad &road) const;
    std::unique_ptr<Acts::BoundTrackParameters> makeParams (const FPGATrackSimTrack &track) const;
    
    std::vector<Identifier> getRdoIdList(const FPGATrackSimHit& hit) const;
    template <typename XAOD_CLUSTER>
    StatusCode matchTrackMeasurements(const EventContext& ctx,
                                           const DataVector<XAOD_CLUSTER>& clusterContainer,
                                           const std::vector<Identifier>& rdoIDs,
                                           std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements) const;
    
    StatusCode findPrototrackMeasurements( const EventContext& ctx,
                                           const xAOD::PixelClusterContainer &pixelClusterContainer,
                                           const xAOD::StripClusterContainer &stripClusterContainer,
                                           std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements,
                                           const std::vector <FPGATrackSimHit>& hits) const;
    private:
    const PixelID* m_pixelId{nullptr};
    const SCT_ID* m_SCTId{nullptr};

  };

#endif
