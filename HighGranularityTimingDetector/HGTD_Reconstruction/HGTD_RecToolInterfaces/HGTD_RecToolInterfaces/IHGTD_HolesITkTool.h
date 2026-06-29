/**
* Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
*
* @file HGTD_RecToolInterfaces/IHGTD_HolesITkTool.h
* @author Valentina Raskina <valentina.raskina@cern.ch>
* @author
* @date September, 2022
* @brief the extrapolations of the tracks on the ITk instrumental areas are 
* collected between the last reconstructed hit in ITk and the first layer of HGTD.
* Such extrapolations are called Holes on Track, since the hits associated to these 
* extrapolations are not found. Method returns the TrackParameters vector.
*
*/

#ifndef IHGTD_HOLESITKTOOL_H 
#define IHGTD_HOLESITKTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "xAODTracking/TrackParticle.h"
#include "AtlasHepMC/GenEvent.h"
#include <memory>

class IHGTD_HolesITkTool : virtual public IAlgTool {

    public :
    // Creates the InterfaceID and interfaceID() method
    DeclareInterfaceID(IHGTD_HolesITkTool, 1, 0); 
    /**
    * @brief  Calculates the number of holes on track in ITk between the last measurement in ITk and first layer of HGTD.
    *
    * @param [in] track Track built in the inner tracker to be extended to HGTD.
    *
    * @return Returns an int corresponding to the number of the holes on track.
    */
    virtual std::vector<std::unique_ptr<Trk::TrackParameters> > getHolesITk(const EventContext& ctx, const xAOD::TrackParticle& track_ptkl) const = 0;

};

#endif // IHGTD_HOLESITKTOOL_H

