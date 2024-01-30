/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////
//                                                      //
//  Implementation of class VP1TrackingGeometryChannel  //
//                                                      //
//  Author: Andreas.Salzburger@cern.ch (primary)        //
//          Thomas.Kittelmann@cern.ch                   //
//                                                      //
//  Initial version: June 2007                          //
//
//  Major updates:
//  - 2024, Jan -- Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
//                 Migrated from the old ATLAS SVN to Git
//                                                      
//////////////////////////////////////////////////////////

#include "VP1TrackingGeometryPlugin/VP1TrackingGeometryChannel.h"
#include "VP1TrackingGeometrySystems/VP1TrackingGeometrySystem.h"
#include "VP1GuideLineSystems/VP1GuideLineSystem.h"

VP1TrackingGeometryChannel::VP1TrackingGeometryChannel()
  : IVP13DStandardChannelWidget(VP1CHANNELNAMEINPLUGIN(VP1TrackingGeometryChannel,"Tracking Geometry"),
                                "This channel displays the tracking geometry system.",
                                "Riccardo.Maria.Bianchi@cern.ch, Andreas Salzburger <Andreas.Salzburger@cern.ch>")
{
}

void VP1TrackingGeometryChannel::init()
{
  addSystem(new VP1GuideLineSystem);
  addSystem(new VP1TrackingGeometrySystem);
}


