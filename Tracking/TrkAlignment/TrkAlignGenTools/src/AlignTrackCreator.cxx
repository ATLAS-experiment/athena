/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "GaudiKernel/MsgStream.h"

#include "AtlasDetDescr/AtlasDetectorID.h"
#include "xAODEventInfo/EventInfo.h"

#include "TrkEventUtils/MeasurementTypeID.h"

#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/Track.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkMaterialOnTrack/MaterialEffectsOnTrack.h"

#include "TrkAlignEvent/AlignTSOS.h"
#include "TrkAlignEvent/AlignModule.h"
#include "TrkAlignEvent/AlignModuleList.h"
#include "TrkAlignEvent/AlignTrack.h"

#include "TrkAlignGenTools/AlignTrackCreator.h"

#include <fstream>

namespace Trk {

  //________________________________________________________________________
  AlignTrackCreator::AlignTrackCreator(const std::string& type, const std::string& name,
                                     const IInterface* parent)
    : IAlignTrackCreator()
    , AthAlgTool(type,name,parent)
  {
    declareInterface<IAlignTrackCreator>(this);
  }

  //________________________________________________________________________
  StatusCode AlignTrackCreator::initialize()
  {
    // Set up ATLAS ID helper
    if (detStore()->retrieve(m_idHelper, "AtlasID").isFailure()) {
      ATH_MSG_ERROR ("Could not get AtlasDetectorID helper" );
      return StatusCode::FAILURE;
    }
    m_measTypeIdHelper=new MeasurementTypeID(m_idHelper);

    if (m_residualCalculator.retrieve().isFailure()) {
      ATH_MSG_FATAL("Could not get " << m_residualCalculator);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved " << m_residualCalculator);

    // get AlignModuleTool
    if (m_alignModuleTool.retrieve().isFailure()) {
      ATH_MSG_FATAL("Could not get " << m_alignModuleTool);
      return StatusCode::FAILURE;
    }
    ATH_MSG_INFO("Retrieved " << m_alignModuleTool);

    ATH_MSG_INFO("initialize() successful in " << name());

    return StatusCode::SUCCESS;
  }

  //________________________________________________________________________
  StatusCode AlignTrackCreator::finalize()
  {
    ATH_MSG_DEBUG("finalize() in AlignTrackCreator");

    // write run/evt to ASCII file
    if (m_writeEventList) {
      std::ofstream output(m_eventListName.value().c_str());
      for (std::vector<std::pair<int,int> >::iterator it=
             m_goodEventList.begin(); it!=m_goodEventList.end(); ++it) {
        int run=(*it).first;
        int evt=(*it).second;
        output << run << "  " << evt << std::endl;
      }
      
      // if no events, output dummy run and event number so file isn't empty
      if (m_goodEventList.empty()) output<<"0  0"<<std::endl;
      output.close();
    }
    
    delete m_measTypeIdHelper;

    return StatusCode::SUCCESS;
  }

  //________________________________________________________________________
  bool AlignTrackCreator::processAlignTrack(AlignTrack * at)
  {
    DataVector<AlignTSOS>* alignTSOSCollection=new DataVector<AlignTSOS>;

    bool goodtrack = false;
    int ntsos(0);
    double chi2(0.);

    AlignModule * lastModule(nullptr);
    bool overlapTrack(false);

    ATH_MSG_DEBUG("nDoF: "<<at->fitQuality()->numberDoF()<<", tsos size: "<<at->trackStateOnSurfaces()->size());

    for (const TrackStateOnSurface* tsos : *at->trackStateOnSurfaces()) {

      AlignTSOS * atsos(nullptr);
      AlignModule * module(nullptr);

      if (tsos->type(TrackStateOnSurface::Outlier) ||
          tsos->type(TrackStateOnSurface::Perigee) ||
          tsos->type(TrackStateOnSurface::Hole))
        continue;
      else if (tsos->type(TrackStateOnSurface::Measurement)) {

	ATH_MSG_DEBUG("checking ntsos: "<<ntsos);

        const MeasurementBase      * mesb  = tsos->measurementOnTrack();
        const TrackParameters      * tparp = tsos->trackParameters();
        const RIO_OnTrack          * rio   = dynamic_cast<const RIO_OnTrack *>(mesb);
        const CompetingRIOsOnTrack * crio  = dynamic_cast<const CompetingRIOsOnTrack *>(mesb);

        if (!rio && crio) rio=&crio->rioOnTrack(0);

        if (rio && tparp) {
          //get detector type
          AlignModule::DetectorType detType = m_alignModuleTool->getDetectorType(rio->detectorElement());
	  ATH_MSG_DEBUG("detType: "<<detType);

          // see if track is good track
	  if (detType!=AlignModule::NDetectorTypes) {
	    ATH_MSG_DEBUG("calling findAlignModule");
	    module=m_alignModuleTool->findAlignModule(rio,detType);
	  }
	  if (module) {
	    ATH_MSG_DEBUG("found module");
	    goodtrack=true;
	    if (lastModule && module!=lastModule) {
	      overlapTrack=true;
	      ATH_MSG_DEBUG("have overlap track");
	    }
	    else
	      lastModule=module;
	  }	
	  ATH_MSG_DEBUG("module not found");

          // see what kind of TSOS
          TrackState::MeasurementType measType = m_measTypeIdHelper->defineType(mesb);
	  
          // create AlignTSOS using CompetingRIOsOnTrack or RIO_OnTrack
          if (crio) 
            atsos = new AlignTSOS(*tsos,module,crio,measType);	  
          else 
            atsos = new AlignTSOS(*tsos,module,rio,measType);

          if (module)
            ATH_MSG_DEBUG("module id "<<module->identify());
          else
            ATH_MSG_VERBOSE("no module");

          atsos->setValid(true);
        }
      }

      // scatterer with no associated measurement
      else if (tsos->type(TrackStateOnSurface::Scatterer) && m_includeScatterers) {
        atsos=new AlignTSOS(*tsos,TrackState::unidentified);
        atsos->setValid(true);
      }

      if (!atsos) {
        continue;
      }

      if (m_removeATSOSNotInAlignModule && !module) {
        delete atsos;
        continue;
      }

      // set detector specific properties (if needed)
      m_alignModuleTool->setMeasurementProperties(atsos);

      alignTSOSCollection->push_back(atsos);
    }

    ATH_MSG_DEBUG("goodtrack="<<goodtrack<<", alignTSOSCollection size: "<<alignTSOSCollection->size());

    if (goodtrack && (!m_requireOverlap||overlapTrack)) {
      
      // store run/evt number in vector
      if (m_writeEventList) {
        const xAOD::EventInfo * eventInfo;
        StatusCode sc=evtStore()->retrieve(eventInfo);
        if (sc.isFailure())
          ATH_MSG_ERROR("Couldn't retrieve event info");

        int run=eventInfo->runNumber();
        int evt=eventInfo->eventNumber();
        m_goodEventList.emplace_back(run,evt);
      }

      // set residuals on ATSOS and get local chi2
      ATH_MSG_DEBUG("calling setResiduals");
      chi2=m_residualCalculator->setResiduals(alignTSOSCollection);
      ATH_MSG_DEBUG("done with setResiduals");

      // check that chi2 from fitter matches local chi2
      if (std::fabs(chi2-at->fitQuality()->chiSquared())>.01) {
        ATH_MSG_DEBUG("local chi2: "<<chi2<<", from fit: "<<at->fitQuality()->chiSquared());
      }

      ATH_MSG_DEBUG("setting alignTrack");
      at->setAlignTSOSCollection(alignTSOSCollection);
      at->setChi2(chi2);

      if (msgLvl(MSG::DEBUG)) at->dump(msg());

      return true;
    }

    ATH_MSG_DEBUG("failed to make AlignTrack");
    delete alignTSOSCollection;
    return false;
  }

} // end namespace
