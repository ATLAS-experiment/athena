# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PMGSystematicsTools )

# External dependencies:
find_package( Python COMPONENTS Interpreter )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )
find_package( Rivet )
find_package( YODA )

# Install files from the package:
atlas_install_data( data/*.yaml )
atlas_install_scripts( local/bin/*.py )
atlas_install_scripts( share/*.sh )

# Aliases:
atlas_add_alias( PMG_systematicsTool "systematicsTool.py" )
atlas_add_alias( PMG_submissionTool "submissionTool.py" )
