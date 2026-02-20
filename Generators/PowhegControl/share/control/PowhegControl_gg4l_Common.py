# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import PowhegControl
transform_runArgs = runArgs if "runArgs" in dir() else None
transform_opts = opts if "opts" in dir() else None
PowhegConfig = PowhegControl.PowhegControl(process_name="gg4l", run_args=transform_runArgs, run_opts=transform_opts)
PowhegConfig.mass_b = 0
