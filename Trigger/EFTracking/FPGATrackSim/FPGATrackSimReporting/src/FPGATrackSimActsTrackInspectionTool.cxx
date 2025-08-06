// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "src/FPGATrackSimActsTrackInspectionTool.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "ActsGeometry/ATLASSourceLink.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include <format>

FPGATrackSim::ActsTrackInspectionTool::ActsTrackInspectionTool(const std::string& algname,
    const std::string& name, const IInterface* ifc)
    : AthAlgTool(algname, name, ifc) {
}

StatusCode FPGATrackSim::ActsTrackInspectionTool::initialize() {
    return StatusCode::SUCCESS;
}

FPGATrackSimActsEventTracks FPGATrackSim::ActsTrackInspectionTool::getActsTracks(const ActsTrk::TrackContainer& tracksContainer) const
{
    FPGATrackSimActsEventTracks t_actsTracks;
    for (const ActsTrk::TrackContainer::ConstTrackProxy tp : tracksContainer)
    {
        std::deque<std::unique_ptr<FpgaActsTrack::Measurement>> t_TrackMeasurements;
        const Acts::BoundVector& parameters = tp.parameters();
        tracksContainer.trackStateContainer().visitBackwards(tp.tipIndex(), [&t_TrackMeasurements](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy& state) -> void
            {
                if (state.hasUncalibratedSourceLink()) {
                    try {
                        std::reference_wrapper<const xAOD::UncalibratedMeasurement> measurementRef =
                            std::cref(ActsTrk::getUncalibratedMeasurement(state.getUncalibratedSourceLink().get<ActsTrk::ATLASUncalibSourceLink>()));
                        const xAOD::UncalibratedMeasurement& measurement = measurementRef.get();
                        assert(static_cast<unsigned int>(measurement.type() < xAOD::UncalibMeasType::nTypes));

                        if (measurement.type() == xAOD::UncalibMeasType::PixelClusterType) {
                            const xAOD::PixelCluster* pixelCluster = static_cast<const xAOD::PixelCluster*>(&measurement);
                            t_TrackMeasurements.emplace_front(std::make_unique<FpgaActsTrack::Measurement>(FpgaActsTrack::Measurement{
                                pixelCluster->identifier(),
                                "Pixel",
                                {   pixelCluster->globalPosition().x(),
                                    pixelCluster->globalPosition().y(),
                                    pixelCluster->globalPosition().z() },
                                state.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::OutlierFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::HoleFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::MaterialFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::SharedHitFlag) }));
                        }
                        else if (measurement.type() == xAOD::UncalibMeasType::StripClusterType) {
                            const xAOD::StripCluster* stripCluster = static_cast<const xAOD::StripCluster*>(&measurement);
                            t_TrackMeasurements.emplace_front(std::make_unique<FpgaActsTrack::Measurement>(FpgaActsTrack::Measurement{
                                stripCluster->identifier(),
                                "Strip",
                                {   stripCluster->globalPosition().x(),
                                    stripCluster->globalPosition().y(),
                                    stripCluster->globalPosition().z()},
                                state.typeFlags().test(Acts::TrackStateFlag::MeasurementFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::OutlierFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::HoleFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::MaterialFlag),
                                state.typeFlags().test(Acts::TrackStateFlag::SharedHitFlag) }));
                        }

                    }
                    catch (const std::bad_any_cast&) {
                    }
                }
            });
        t_actsTracks.emplace_back(std::make_unique<FpgaActsTrack>(FpgaActsTrack{ parameters, std::move(t_TrackMeasurements) }));
    }

    return t_actsTracks;
}


std::string FPGATrackSim::ActsTrackInspectionTool::getPrintoutActsEventTracks(
    const fpgaActsEventTracks& tracks) const
{
    std::ostringstream printoutTable;
    unsigned int t_trackCoutner = 0, t_measCounter = 0;
    printoutTable << "\n|----------------------------------------------------------------------------------------------------|\n";
    for (const auto& track : tracks)
    {
        ++t_trackCoutner;
        printoutTable   << "|     # |        QopT      |      Theta      |        Phi      |         d0       |         z0       |\n"
                        << "|----------------------------------------------------------------------------------------------------|\n"
                        << std::format("| {:>5} | {:>16.10f} | {:>15.10f} | {:>15.10f} | {:>16.10f} | {:>16.10f} |\n",
            t_trackCoutner,
            track->parameters[Acts::eBoundQOverP] / 1000.,
            track->parameters[Acts::eBoundTheta],
            track->parameters[Acts::eBoundPhi],
            track->parameters[Acts::eBoundLoc0],
            track->parameters[Acts::eBoundLoc1]);
        t_measCounter = 0;
        printoutTable   << "|       |____________________________________________________________________________________________|\n"
                        << "|       | ## |  type |     x    |     y    |     z    | outlier |  meas |  hole |     Identifier     |\n"
                        << "|       |--------------------------------------------------------------------------------------------|\n";
        for (const auto& measurement : track->trackMeasurements)
        {
            ++t_measCounter;
            printoutTable << std::format("|       | {:>2} | {} | {:>8.3f} | {:>8.3f} | {:>8.3f} | {:>7} | {:>5} | {:6>5} | {:>18} |\n",
                t_measCounter,
                measurement->type.c_str(),
                measurement->coordinates.x,
                measurement->coordinates.y,
                measurement->coordinates.z,
                measurement->outlierFlag,
                measurement->measurementFlag,
                measurement->holeFlag,
                measurement->identifier);
        }
        printoutTable << "|----------------------------------------------------------------------------------------------------|\n";
    }

    return printoutTable.str();
}


std::string FPGATrackSim::ActsTrackInspectionTool::getPrintoutStatistics(
    const std::map<std::string, std::map<uint32_t, std::vector<uint32_t>> >& tracksForAllEvents) const
{
    std::ostringstream printoutTable;
    printoutTable << "Printing out ACTS statistics";
    printoutTable << "\n|---------------------------------------------------------------------------------------|"
        << "\n|            Collection Name        | Outliers (avg) | Measurements (avg) | Holes (avg) |"
        << "\n|---------------------------------------------------------------------------------------|";
    for (const auto& collection : tracksForAllEvents)
    {
        printoutTable << std::format("\n| {:>33} | {:>14.2f} | {:>18.2f} | {:>11.2f} |",
            collection.first,
            collection.second.at(Acts::TrackStateFlag::OutlierFlag).size() ?
                    std::accumulate(collection.second.at(Acts::TrackStateFlag::OutlierFlag).begin(),
                                    collection.second.at(Acts::TrackStateFlag::OutlierFlag).end(), 0.0)
                    / collection.second.at(Acts::TrackStateFlag::OutlierFlag).size() : 0,

            collection.second.at(Acts::TrackStateFlag::MeasurementFlag).size() ? std::accumulate(collection.second.at(Acts::TrackStateFlag::MeasurementFlag).begin(), collection.second.at(Acts::TrackStateFlag::MeasurementFlag).end(), 0.0) / collection.second.at(Acts::TrackStateFlag::MeasurementFlag).size() : 0,

            collection.second.at(Acts::TrackStateFlag::HoleFlag).size() ? std::accumulate(collection.second.at(Acts::TrackStateFlag::HoleFlag).begin(), collection.second.at(Acts::TrackStateFlag::HoleFlag).end(), 0.0) / collection.second.at(Acts::TrackStateFlag::HoleFlag).size() : 0);
    }
    printoutTable << "\n|---------------------------------------------------------------------------------------|";
    return printoutTable.str();
}
