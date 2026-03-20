/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS // Full Athena only

// -*- c++ -*-
#ifndef TRIGGER_DECISION_TOOL_FeatureCollectAthena_H
#define TRIGGER_DECISION_TOOL_FeatureCollectAthena_H


/**********************************************************************************
 * @Project:
 * @Package: TrigDecision
 *
 *
 * @author Nicolas Berger  <Nicolas.Berger@cern.ch>  - LAPP Annecy
 * @author Tomasz Bold     <tomasz.bold@cern.ch>     - UC Irvine, AGH-UST Krakow
 * @author Lukas Heinrich  <lukas.heinrich@cern.ch>  - NYU
 *
 ***********************************************************************************/

#include <concepts>
#include <string>
#include <type_traits>

#include "AthContainers/ConstDataVector.h"

#include "TrigNavigation/NavigationCore.h"
#include "TrigNavigation/NavigationCore.icc"
#include "TrigDecisionTool/Conditions.h"
#include "TrigDecisionTool/TDTUtilities.h"
#include "TrigDecisionTool/ClassTraits.h"
#include "TrigDecisionTool/Feature.h"

#include "TrigSteeringEvent/TrigPassBits.h"
#include "TrigSteeringEvent/TrigPassFlags.h"

// for template specializations

#include "AnalysisTriggerEvent/Muon_ROI.h"
#include "AnalysisTriggerEvent/EmTau_ROI.h"
#include "AnalysisTriggerEvent/Jet_ROI.h"
#include "AnalysisTriggerEvent/JetET_ROI.h"
#include "AnalysisTriggerEvent/EnergySum_ROI.h"
#include "AnalysisTriggerEvent/LVL1_ROI.h"

#include "xAODTrigger/EmTauRoI.h"
#include "xAODTrigger/EmTauRoIContainer.h"

#include "xAODTrigger/MuonRoI.h"
#include "xAODTrigger/MuonRoIContainer.h"

#include "xAODTrigger/JetRoI.h"
#include "xAODTrigger/JetRoIContainer.h"

// for smart type information

#include "TrigStorageDefinitions/EDM_TypeInfo.h"

namespace Trig {

  /**
   * This is the implementation of the Feature collection for Athena
   **/
  
  namespace FeatureAccessImpl {
    // function declaration (see cxx for the deifinition) wanted to have this freedom in case of patches needed
    const TrigPassBits* getBits(size_t sz, const HLT::TriggerElement* te,
                                const std::string& label, const HLT::NavigationCore* navigation );

    const TrigPassFlags* getFlags(size_t sz, const HLT::TriggerElement* te,
                                  const std::string& label, const HLT::NavigationCore* navigation );

    // Detect DataVector type by existence of ElementProxy
    template<typename T>
    concept isDataVector = requires {
      typename T::ElementProxy;
    };

    template<class T>
    const T* use_or_construct(const T* source, const HLT::TriggerElement* te, const std::string& label,
                              unsigned int condition, const HLT::NavigationCore* navigation ) {

      if constexpr(!isDataVector<T>) {
        return source;
      }
      else {
        const TrigPassBits* bits{nullptr};
        if ( condition == TrigDefs::Physics ) {// only passing objects
          bits = getBits(source->size(), te, label , navigation);
        }
        if ( bits ) { // the actual filtering
          auto destination = new ConstDataVector<T>(SG::VIEW_ELEMENTS);

          for(const typename T::base_value_type *obj : *source) {
            if ( HLT::isPassing(bits, obj, source)  ) // if bits are missing or obj is realy marked as passing
              destination->push_back(obj);
          }
          return destination->asDataVector();
        }
        return source;
      }
    }
  

    // The "flatten" option is for the case that requested and stored are the same types
    // (e.g. requested is egamma_container and stored is egamma_container or
    // requested is TrigRoiDescriptor and stored is TrigRoiDescriptor).
    // The actual data storage may be yet different. In the second case it is TrigRoiDescriptorCollection.
    // Here we have two cases, if the requested object is DataVector and the Physics flag is requested 
    // we need to do the additional filtering. We guess that this is the case by sniffing the object T
    // with the has_traits template. This filtering is done as follows. New container is created with
    // the VIEW_ELEMENTS mode and only selected objects are inserted into it.

    template<class T, class STORED, bool flatten, class LINK>
    void insert_and_flatten(std::vector<Trig::Feature<T> >& destination, const STORED* source,
                            const HLT::TriggerElement* te, const std::string& label,
                            unsigned int condition, const HLT::NavigationCore* navigation, const LINK& lnk) {

      if constexpr(flatten) {
        const TrigPassBits* bits{nullptr};
        if ( condition == TrigDefs::Physics ) {// only passing objects
          bits = getBits(source->size(), te, label , navigation);
        }

        for(const T* obj : *source) {
          if ( bits==nullptr || HLT::isPassing(bits, obj, source)  ) {// if no bits or obj is marked as passing
            destination.push_back(Trig::Feature<T>(obj, te, label,
                                                   false,  // do not delete
                                                   ElementLink<typename LINK::value_type>(obj,*source)));
          }
        }
      }
      else {
        const T* possibly_reduced_container = use_or_construct<T>(source, te, label, condition, navigation);

        destination.push_back(Trig::Feature<T>(possibly_reduced_container, te, label,
                                               // true: Feature<T> deletes container at deletion
                                               possibly_reduced_container != source,
                                               lnk));
      }
    }

    template<class REQUESTED,class EDMLIST>
    struct get_links {

      template<class FEATURE>
      void operator()() {

        using container_type = Features2Container_t<FEATURE,EDMLIST>;
        using object_type = Features2Object_t<FEATURE,EDMLIST>;
        using link_type = Features2LinkHelper_t<FEATURE,container_type>;

        constexpr bool do_flatten  = (! std::is_same_v<REQUESTED,container_type>) && std::is_same_v<FEATURE,container_type>;
        constexpr bool do_retrieve = ! (std::is_same_v<REQUESTED,container_type> && std::is_same_v<FEATURE,object_type>);

        if constexpr( !do_retrieve ) {
          // no retrieve -> do nothing
        }
        else {
          // do retrieve
          std::string sourceLabel;
          link_type link;
          const bool new_result = m_navigation->getRecentFeatureDataOrElementLink( m_te, link, m_label, *m_sourceTE, sourceLabel );

          if (new_result) {
            if (m_teName.empty() || m_teName == Trig::getTEName(**m_sourceTE)) {
              if (link.cptr()) {
                // Helper to deref link
                auto get_ptr = [](const auto& link) {
                  if constexpr(std::is_same_v<FEATURE,container_type>)
                    return link.cptr();
                  else
                    return *link;
                };

                insert_and_flatten<REQUESTED,FEATURE,do_flatten,link_type>
                  (*m_data, get_ptr(link),
                   *m_sourceTE, sourceLabel, m_condition, m_navigation,link);
              }
            }
          }
          *m_result = *m_result && new_result;
        }
      }

      const HLT::TriggerElement* m_te{nullptr};
      std::vector<Trig::Feature<REQUESTED> >* m_data{nullptr};
      const std::string m_label;
      unsigned int m_condition{0};
      std::string m_teName;
      const HLT::NavigationCore* m_navigation{nullptr};
      bool* m_result{nullptr};
      const HLT::TriggerElement** m_sourceTE{0};
    };

    /**
     * @brief actual feature acceess implementation
     * It has (thanks to the ClassTraits) functionality to flatten containers of containers.
     **/
    template<class T>
    void collect(const HLT::TriggerElement* te, std::vector<Trig::Feature<T> >& data,
                 const std::string& label, unsigned int condition, const std::string& teName,
                 const HLT::TrigNavStructure* navstructure) {

      auto navigation = dynamic_cast<const HLT::NavigationCore*>(navstructure);

      if (condition == TrigDefs::Physics && !te->getActiveState() ) return;

      const HLT::TriggerElement* sourceTE{};
      bool result = true;
#ifndef __GCCXML__
      using object_type = Features2Object_t<T>;
      using feature_list = Object2Features_t<object_type>;
      get_links<T,TypeInfo_EDM> link_getter{te, &data, label, condition, teName, navigation, &result, &sourceTE};
      feature_list::for_each(link_getter);
#endif

      if (result){
        /*do nothing anymore*/
      } else {
        // getRecentFeature returned false -> bifurcation?
        const std::vector<HLT::TriggerElement*> bif_tes = navigation->getDirectPredecessors(sourceTE);
        if ( bif_tes.size() <= 1 ) {
          return; // that means it is plain error (it will be printed by the Navigation)
        } else {
          // bifurcation point
          for( const HLT::TriggerElement* predecesor_te : bif_tes )
            collect(predecesor_te, data, label, condition, teName, navigation);
        }
      }
    }
  

    template<>
    void collect<Muon_ROI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<Muon_ROI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    template<>
    void collect<EmTau_ROI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<EmTau_ROI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    template<>
    void collect<Jet_ROI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<Jet_ROI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    template<>
    void collect<xAOD::EmTauRoI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<xAOD::EmTauRoI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    template<>
    void collect<xAOD::MuonRoI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<xAOD::MuonRoI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    template<>
    void collect<xAOD::JetRoI>(const HLT::TriggerElement* te, std::vector<Trig::Feature<xAOD::JetRoI> >& data, const std::string&, unsigned int, const std::string&, const HLT::TrigNavStructure* navigation);

    // ==============
    //
    // This section deals with the TrigPassFlags creation
    //
    // ==============

    // access by container, stored as container
    template<isDataVector CONT> TrigPassFlags
    build_flags (const CONT *orig_cont, const CONT* cont, const TrigPassFlags * orig_tpf) {

      TrigPassFlags tpf(cont->size(), orig_tpf->flagSize());

      if(orig_cont->size() != orig_tpf->size()) {
        //std::cout << "WARNING: original constainer size (" << orig_cont->size() << ") different for size of TrigPassFlags (" << orig_tpf->size() << ")." << std::endl;
        return tpf;
      }

      unsigned int currentPos=0;
      for(const typename CONT::base_value_type* obj : *cont) {
        const auto orig_obj = std::find(orig_cont->begin(),orig_cont->end(),obj);

        if(orig_obj == orig_cont->end()) {
          //std::cout << "WARNING: object in reduced container can' be found in original." << std::endl;
        } else {
          size_t idx = orig_obj-orig_cont->begin();
          tpf.setFlag(currentPos, orig_tpf->getFlag(idx));
        }
        currentPos++;
      }

      return tpf;
    }

    template<class T> TrigPassFlags
    build_flags (const T *orig, const T* feature, const TrigPassFlags * orig_tpf) {
      if(orig != feature) return TrigPassFlags(); // a problem TODO: print a ERROR

      TrigPassFlags tpf(1, orig_tpf->flagSize());
      tpf.setFlag(0,orig_tpf->getFlag(0));
      return tpf;
    }

    // access by single object, stored as container
    template<class T, class STORED> TrigPassFlags
    build_flags2(const STORED* orig_cont, const T* obj, const TrigPassFlags * orig_tpf)
    {
      if(orig_cont->size() != orig_tpf->size()) {
        //std::cout << "WARNING: original constainer size (" << orig_cont->size() << ") different for size of TrigPassFlags (" << orig_tpf->size() << ")." << std::endl;
        return TrigPassFlags();
      }

      TrigPassFlags tpf(1, orig_tpf->flagSize());

      const auto orig_obj = std::find(orig_cont->begin(),orig_cont->end(), obj);

      if(orig_obj == orig_cont->end()) {
        //std::cout << "WARNING: object in reduced container can' be found in original." << std::endl;
      } else {
        size_t idx = orig_obj-orig_cont->begin();
        tpf.setFlag(0, orig_tpf->getFlag(idx));
      }
      return tpf;
    }

    template<class T> TrigPassFlags
    getFlags(const Trig::Feature<T>& f, const TrigPassFlags *orig_tpf, HLT::NavigationCore* navigation ) {

      using STORED = TrigDec::ClassTraits<T>::type;

      const STORED* orig{};
      const HLT::TriggerElement* sourceTE{};
      std::string sourceLabel;
      if (navigation->getRecentFeature(f.te(), orig, f.label(), sourceTE, sourceLabel)) {
        if constexpr (std::is_same_v<T,STORED>)
          return build_flags(orig, f.cptr(), orig_tpf);
        else
          return build_flags2(orig, f.cptr(), orig_tpf);
      }
      return TrigPassFlags();

    }

    // specialization for types without getRecentFeature implementation
    template<> TrigPassFlags getFlags(const Trig::Feature<EmTau_ROI>& f, const TrigPassFlags *orig_tpf, HLT::NavigationCore* navigation );
    template<> TrigPassFlags getFlags(const Trig::Feature<Muon_ROI>& f, const TrigPassFlags *orig_tpf, HLT::NavigationCore* navigation );
    template<> TrigPassFlags getFlags(const Trig::Feature<Jet_ROI>& f, const TrigPassFlags *orig_tpf, HLT::NavigationCore* navigation );

  } // EOF namespace FeatureAccessImpl

} // EOF namespace Trig

#endif


#endif //XAOD_ANALYSIS
