/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
 
 /// @author Baptiste Ravina
 
 #ifndef EVENT_SELECTOR_RUNNUMBERSELECTORALG_H
 #define EVENT_SELECTOR_RUNNUMBERSELECTORALG_H
 
 // Algorithm includes
 #include <AnaAlgorithm/AnaAlgorithm.h>
 #include <AsgTools/PropertyWrapper.h>
 #include <SystematicsHandles/SysReadHandle.h>
 #include <SelectionHelpers/SysReadSelectionHandle.h>
 #include <SelectionHelpers/SysWriteSelectionHandle.h>
 
 // Framework includes
 #include <xAODBase/IParticleContainer.h>
 #include <xAODEventInfo/EventInfo.h>
 
 #include <EventSelectionAlgorithms/SignEnums.h>
 
 namespace CP {
 
   /// \brief an algorithm to select an event with 2-lepton mass compared to a
   /// specified value
 
   class RunNumberSelectorAlg final : public EL::AnaAlgorithm {
 
     /// \brief the standard constructor
   public:
     using EL::AnaAlgorithm::AnaAlgorithm;
     virtual StatusCode initialize() override;
     virtual StatusCode execute() override;

   private:
 
     /// \brief the run number against which to compare
     Gaudi::Property<unsigned int> m_runNumberRef {this, "runNumber", 0, "runNumber to compare"};

     /// \brief whether to use a random run number
     Gaudi::Property<bool> m_useRandomRunNumber {this, "useRandomRunNumber", false, "whether to use a random run number (for MC) instead of the actual run number (for data)"};

     /// \brief the comparison (GT, LT, etc)
     Gaudi::Property<std::string> m_sign {this, "sign", "SetMe", "comparison sign to use"};

     /// \brief the operator version of the comparison (>, <, etc)
     SignEnum::ComparisonOperator m_signEnum{};

     /// \brief the systematics
     CP::SysListHandle m_systematicsList {this};

     /// \brief the event info handle
     CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
       this, "eventInfo", "EventInfo", "the EventInfo container to read selection decisions from"
         };

     /// \brief the preselection
     CP::SysReadSelectionHandle m_preselection {
       this, "eventPreselection", "SetMe", "name of the preselection to check before applying this one"
         };

     /// \brief the output selection decoration
     CP::SysWriteSelectionHandle m_decoration {
       this, "decorationName", "SetMe", "decoration name for the RunNumber selector"
         };

   }; // class
 } // namespace CP

#endif // EVENT_SELECTOR_RUNNUMBERSELECTORALG_H
