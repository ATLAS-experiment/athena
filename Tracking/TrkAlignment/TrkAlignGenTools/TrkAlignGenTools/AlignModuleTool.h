/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKALIGNGENTOOLS_ALIGNMODULETOOL_H
#define TRKALIGNGENTOOLS_ALIGNMODULETOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/DataVector.h"

#include "TrkAlignInterfaces/IAlignModuleTool.h"
#include "TrkAlignEvent/AlignModule.h"

/**
   @file AlignModuleTool.h
   @class AlignModuleTool

   @brief Tool for managing list of AlignModules, detector elements in 
   AlignModules, and AlignPar for both.  Can be used as is or inherited by
   subdetector. (dual_use library exists for this class.)

   @author Robert Harrington <roberth@bu.edu>
   @author Daniel Kollar <daniel.kollar@cern.ch>
   @date 3/17/2009
*/

class AtlasDetectorID;
class TRT_ID;

namespace Trk {

  class TrkDetElementBase;
  class RIO_OnTrack;

  class AlignModuleTool :  public AthAlgTool, virtual public IAlignModuleTool {
 
  public:
    typedef DataVector<AlignPar>    AlignParVec;
    typedef DataVector<AlignParVec> AlignPar2DVec;

    AlignModuleTool(const std::string& type, const std::string& name,
		    const IInterface* parent);
    
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    AlignModuleListType alignModuleListType() const override{
      return static_cast<AlignModuleListType>(m_alignModuleListType.value());
    }

    //
    // GeometryManagerTool methods
    //

    /** sets alignModuleLists and idHash maps for a set of AlignModules.  AlignModuleList should be of type Combined, containing detector elements of different types. */
    void setAlignModules(AlignModuleList* alignmods,
			 std::vector<AlignModuleList*>* idHashMaps) override;
    
    /** Sets the AlignParLists using boost::multi_array.  Lists are owned by GeometryManagerTool. */
    void setAlignParLists(AlignPar2DVec* fullAlignParList, 
			  AlignPar2DVec* alignParList=0) override;

    /** set and retrieve flag whether or not the sub-TrkDetElementBase structures are aligned for a given detector type */
    void setSubDetElement(AlignModule::DetectorType detType, bool status) override { m_subDetElement[detType]=status; }
    bool subDetElement(AlignModule::DetectorType detType) const override { return m_subDetElement[detType]; }

    //
    // AlignTrackCreator methods
    //

    /** Returns pointer to AlignModule containing RIO.  Returns 0 if not found. */
    virtual AlignModule * findAlignModule(const RIO_OnTrack * rio,
                                          AlignModule::DetectorType detType) const override;

    /** Returns pointer to AlignModule containing RIO.  Returns 0 if not found. */
    virtual AlignModule * findAlignModule(const TrkDetElementBase* det,
                                          AlignModule::DetectorType detType) const override;

    //
    // MatrixTool methods
    //

    /** Returns total number of alignment parameters. */
    int nAlignParameters() const override { return m_alignParList1D.size(); }
    
    /** Returns 1-D vector of all AlignModules containing all detector types. */
    const AlignModuleList* alignModules1D() override { return &m_alignModules1D; }
    
    /** Returns 1-D vector of all AlignPars for all detector types. */
    AlignParVec* alignParList1D() override { return &m_alignParList1D; }
    
    
    //
    // DerivCalcTool methods
    //

    /** returns vector of AlignPars for an AlignModule */
    AlignParVec* getAlignPars(const AlignModule* alignModule) const override;
    
    /** methods used for numerical derivative to shift module before 
	refitting track and restore to normal position, to be inherited by 
	detector-specific align modules */
    virtual void shiftModule(const AlignModule*, const Track*, 
			     AlignModule::TransformParameters, double) const override {}
    virtual void restoreModule(const AlignModule*) const override {}
    
    
    //
    // AlignDBTool methods
    // 
    
    /** fills values for alignment parameters for a detector element, applying AlignModule 
        transforms if necessary */ 
    virtual void fillDetAlignmentParameters(const TrkDetElementBase* det,
					    DataVector<AlignPar>* apVec,
					    AlignModule::DetectorType detType) const override;

    /** Returns collection of AlignModules for a particular detector type.*/    
    virtual const AlignModuleList* getAlignModuleCollection() const override
    { return m_alignModules; }

    //
    // FillNtuple methods
    //

    /** returns vector of full AlignPars for an AlignModule */
    AlignParVec* getFullAlignPars(const AlignModule* alignModule) const override;


    //
    // miscellaneous methods
    //

    /** Returns detector type (AlignModule enum) for a detector element. */
    AlignModule::DetectorType getDetectorType(const TrkDetElementBase* det) const override;

    /** Returns identifier hash for sub-TrkDetElementBase structure based on detector type.
        It needs detector specific implementation in a derived class */
    virtual int subDetElementIDHash(Identifier ) const { return 0; }

    /** returns complete AlignParList (active parameters only) */
    AlignPar2DVec* alignParList() const { return m_alignParList; }

    /** returns complete AlignParList */
    AlignPar2DVec* fullAlignParList() const { return m_fullAlignParList; }

  protected:
    const AtlasDetectorID* m_idHelper = nullptr;          //!< Used to identify type of detector

  private:

    IntegerProperty m_alignModuleListType{this, "AlignModuleListType", Trk::L3}; //!< uses Trk enum AlignModuleListType (L1,L2,L3,L1_5,L2_5)

    AlignModuleList*                    m_alignModules = nullptr;    //!< list of AlignModules
    std::vector<const AlignModuleList*> m_alignModuleMaps; //!< maps detector element idHash to AlignModule

    AlignPar2DVec*            m_alignParList = nullptr;     //!< contains active parameters
    AlignPar2DVec*            m_fullAlignParList = nullptr; //!< contains all parameters

    std::vector<AlignModule*> m_alignModules1D;    //!< 1-D vector of AlignModules
    AlignParVec               m_alignParList1D;    //!< 1-D vector of AlignPars

    std::vector<bool>         m_subDetElement;     //!< identify detectors aligning structures smaller than TrkDetElementBase
  };

 MsgStream& operator << ( MsgStream& sl, AlignModuleTool& alignModTool);
  
}

#endif // TRKALIGNGENTOOLS_ALIGNMODULETOOL_H
