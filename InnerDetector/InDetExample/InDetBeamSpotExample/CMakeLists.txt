# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetBeamSpotExample )

# Necessary external(s):
find_package( cx_Oracle )
find_package( pyyaml )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( bin/*.py test/*.sh)

# Aliases:
atlas_add_alias( plotBeamSpotVert "plotBeamSpotVert.py" )
atlas_add_alias( pickleTool "pickleTool.py" )
atlas_add_alias( resolveBeamSpotTag "resolveBeamSpotTag.py" )
atlas_add_alias( beamspotnt "beamspotnt.py" )
atlas_add_alias( plotBeamSpotCompare "plotBeamSpotCompare.py" )
atlas_add_alias( dumpBeamSpot "dumpBeamSpot.py" )
atlas_add_alias( runJobs "runJobs.py" )
atlas_add_alias( plotBeamSpotMon "plotBeamSpotMon.py" )
atlas_add_alias( dqBeamSpot "dqBeamSpot.py" )
atlas_add_alias( extractScanInfo "extractScanInfo.py" )
atlas_add_alias( checkJobs "checkJobs.py" )
atlas_add_alias( taskman "taskman.py" )
atlas_add_alias( bsCompare "bsCompare.py" )
atlas_add_alias( plotBeamSpotVxVal "plotBeamSpotVxVal.py" )
atlas_add_alias( periodInfo "periodInfo.py" )
atlas_add_alias( beamspotman "beamspotman.py" )
atlas_add_alias( dumpLBFileMap "dumpLBFileMap.py" )

