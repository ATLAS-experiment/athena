# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
import PowhegControl
transform_runArgs = runArgs if "runArgs" in dir() else None
transform_opts = opts if "opts" in dir() else None
PowhegConfig = PowhegControl.PowhegControl(process_name="bblvlv_Beta", run_args=transform_runArgs, run_opts=transform_opts)
