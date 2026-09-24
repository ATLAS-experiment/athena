# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Shared configuration helpers for the event reconstruction algorithms.
# Python-only: no library or component is built.

atlas_subdir( ReconstructionUtils )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
