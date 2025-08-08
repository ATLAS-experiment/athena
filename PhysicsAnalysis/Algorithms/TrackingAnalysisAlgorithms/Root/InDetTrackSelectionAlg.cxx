/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner



//
// includes
//

#include <TrackingAnalysisAlgorithms/InDetTrackSelectionAlg.h>

#include <InDetTrackSystematicsTools/InDetTrackSystematics.h>
#include <PATInterfaces/ISystematicsTool.h>

//
// method implementations
//

namespace CP
{

  StatusCode InDetTrackSelectionAlg ::
  initialize ()
  {
    if (!m_selectionTool.empty())
      ANA_CHECK (m_selectionTool.retrieve());
    if (!m_filterTool.empty())
    {
      if (m_selectionTool.empty())
      {
        ATH_MSG_ERROR( "The TruthFilterTool was requested without initializing the SelectionTool. "
                       "This is not supported. Please check that the SelectionTool is used." );
        return StatusCode::FAILURE;
      }

      CP::SystematicSet recommendedSystematics;
      if ( m_filterWP=="TIGHT" )
      {
        ATH_MSG_INFO( "The TruthFilterTool is set up with the TIGHT selection "
                      "working point. Will run the appropriate systematics" );
        recommendedSystematics =
        {
          InDet::TrackSystematicMap.at(InDet::TRK_FAKE_RATE_TIGHT),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_TIGHT_GLOBAL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_TIGHT_IBL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_TIGHT_PP0),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_TIGHT_PHYSMODEL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_TIGHT_COMBINED)
        };
      }
      else if ( m_filterWP=="LOOSE" )
      {
        ATH_MSG_INFO( "The TruthFilterTool is set up with the LOOSE selection "
                      "working point. Will run the appropriate systematics" );
        recommendedSystematics =
        {
          InDet::TrackSystematicMap.at(InDet::TRK_FAKE_RATE_LOOSE),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_LOOSE_GLOBAL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_LOOSE_IBL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_LOOSE_PP0),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_LOOSE_PHYSMODEL),
          InDet::TrackSystematicMap.at(InDet::TRK_EFF_LOOSE_COMBINED)
        };
      }
      else
      {
        ATH_MSG_ERROR( "The TruthFilterTool was requested with the unsupported selection working "
                       "point "+ m_filterWP + ". Please use the cut levels 'Loose' or 'TightPrimary'." );
        return StatusCode::FAILURE;
      }

      ANA_CHECK (m_filterTool.retrieve());

      const SystematicSet affectingSystematics = m_filterTool->affectingSystematics();
      for (auto& sys : recommendedSystematics)
      {
        if (affectingSystematics.find(sys.name()) == affectingSystematics.end())
        {
          ATH_MSG_ERROR( "Systematic " + sys.name() + " was expected for the FilterTool "
                         "based on the working point " + m_filterWP + " but not found "
                         "in affectingSystematics.");
          return StatusCode::FAILURE;
        }
      }
      //TODO: The second argument is meant to be 'affecting'. However, this leads
      //      to the registration of both 'Loose' and 'Tight' systematics in each
      //      WP. This may require changes to the underlying methods of the filterTool.
      ANA_CHECK (m_systematicsList.addSystematics (recommendedSystematics, recommendedSystematics));
    }

    ANA_CHECK (m_tracksHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_tracksHandle, SG::AllowEmpty));
    ANA_CHECK (m_selectionHandle.initialize (m_systematicsList, m_tracksHandle));
    ANA_CHECK (m_systematicsList.initialize());

    if (!m_selectionTool.empty())
    {
      m_acceptInfo = m_selectionTool->getAcceptInfo();
      if (!m_filterTool.empty())
      {
        if (m_acceptInfo.addCut( "truthFilter", "Selection of tracks according to the InDetTrackTruthFilterTool" ) < 0)
        {
          ATH_MSG_ERROR( "Failed to add cut 'truthFilter' because the TAccept object is full." );
          return StatusCode::FAILURE;
        }
      }
      asg::AcceptData blankAccept (&m_acceptInfo);
      // Just in case this isn't initially set up as a failure clear it this one
      // time. This only calls reset on the bitset
      blankAccept.clear();
      m_setOnFail = selectionFromAccept(blankAccept);

      ANA_CHECK (m_nameSvc.retrieve());
      ANA_CHECK (m_nameSvc->addAcceptInfo (m_tracksHandle.getNamePattern(),
                 m_selectionHandle.getLabel(), m_acceptInfo) );
    }

    return StatusCode::SUCCESS;
  }



  StatusCode InDetTrackSelectionAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      if (!m_filterTool.empty())
        ANA_CHECK (m_filterTool->applySystematicVariation (sys));

      const xAOD::TrackParticleContainer *tracks = nullptr;
      ANA_CHECK (m_tracksHandle.retrieve (tracks, sys));
      for (const xAOD::TrackParticle *track : *tracks)
      {
        if (m_preselection.getBool (*track, sys))
        {
          if (!m_selectionTool.empty())
          {
            asg::AcceptData fullAccept (&m_acceptInfo);
            asg::AcceptData selectAccept = m_selectionTool->accept(track);
            for (unsigned int i = 0; i < selectAccept.getNCuts(); i++)
            {
              fullAccept.setCutResult (selectAccept.getCutName(i), selectAccept.getCutResult(i));
            }

            if (!m_filterTool.empty())
            {
              fullAccept.setCutResult ("truthFilter", m_filterTool->accept (track));
            }

            m_selectionHandle.setBits
              (*track, selectionFromAccept (fullAccept), sys);
          }
          else
          {
            m_selectionHandle.setBool (*track, true, sys);
          }
        }
        else
        {
          if (!m_selectionTool.empty())
          {
            m_selectionHandle.setBits (*track, m_setOnFail, sys);
          }
          else
          {
            m_selectionHandle.setBool (*track, false, sys);
          }
        }
      }
    }

    return StatusCode::SUCCESS;
  }
}
