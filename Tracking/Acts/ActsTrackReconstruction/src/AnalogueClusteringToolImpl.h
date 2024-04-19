/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTS_ANALOGUECLUSTERING_H
#define ACTS_ANALOGUECLUSTERING_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "PixelConditionsData/ITkPixelOfflineCalibData.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IOnTrackCalibratorTool.h"
#include "OnTrackCalibrator.h"

namespace ActsTrk {

template <typename calib_data_t, typename traj_t>
class AnalogueClusteringToolImpl : public extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>> {
public:
    using base_class = typename extends<AthAlgTool, IOnTrackCalibratorTool<traj_t>>::base_class;
    using Pos = typename OnTrackCalibrator<traj_t>::PixelPos;
    using Cov = typename OnTrackCalibrator<traj_t>::PixelCov;
    using TrackStateProxy = typename OnTrackCalibrator<traj_t>::TrackStateProxy;

    AnalogueClusteringToolImpl(const std::string& type,
			       const std::string& name,
			       const IInterface* parent);

    virtual StatusCode initialize() override;

    std::pair<Pos, Cov> calibrate(const Acts::GeometryContext&,
				  const Acts::CalibrationContext&,
				  const xAOD::PixelCluster&,
				  const TrackStateProxy&) const;
    
    virtual void connect(OnTrackCalibrator<traj_t>& calibrator) const override;

private:

    using error_data_t = typename std::remove_pointer_t<decltype(std::declval<calib_data_t>().getClusterErrorData())>;

    const InDetDD::SiDetectorElement* getDetectorElement(xAOD::DetectorIDHashType id) const;

    std::pair<float, float> anglesOfIncidence(const InDetDD::SiDetectorElement& element,
					      const TrackStateProxy& state) const;

    const error_data_t* getErrorData() const;

    std::pair<float, float>
    getPositionCorrection(const error_data_t& errorData,
			  const InDetDD::SiDetectorElement& element,
			  const std::pair<float, float>& angles,
			  const xAOD::PixelCluster& cluster) const;

    std::pair<float, float>
    getCorrectedError(const error_data_t& errorData,
		      const InDetDD::SiDetectorElement& element,
		      const std::pair<float, float>& angles,
		      const xAOD::PixelCluster& cluster) const;

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey {
	this,
        "DetEleCollKey",
	"",
	"Key of SiDetectorElementCollection for Pixel"
    };

    SG::ReadCondHandleKey<calib_data_t> m_clusterErrorKey {
	this,
	"PixelOfflineCalibData",
	"ITkPixelOfflineCalibData",
	"Calibration data for pixel clusters"
    };


    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {
	this,
	"PixelLorentzAngleTool",
	"",
	"Tool to retreive Lorentz angle"
    };

    // in micrometers
    Gaudi::Property<int> m_thickness {this, "PixelThickness", 250};


};

} // namespace ActsTrk

#include "AnalogueClusteringToolImpl.icc"

#endif
