# Managing Powheg for ATLAS

The experts' Gitlab repository can be found [here](https://gitlab.cern.ch/atlas-physics/pmg/mcexperts/powheg-experts/athena). It is a fork of the central Athena repository. 
The `powheg-experts` repository gets synchronized regularly with the official ATLAS/athena repo.

We use it to develop the software and then open merge requests in central Athena to propagate them into releases. We use [Gitlab issues](https://gitlab.cern.ch/atlas-physics/pmg/mcexperts/powheg-experts/athena/issues) to track tasks and development.

To make **new developments**, please make a new branch with `<changeName>_<AthenaVersion`, e.g. `newProcess_main`.
Then make a merge request into `atlas/athena` repository.

## Adding new processes

See the [issues](https://gitlab.cern.ch/atlas-physics/pmg/mcexperts/powheg-experts/athena/issues) on Gitlab. Some information about previous work and status can be found
[here](https://docs.google.com/spreadsheets/d/10hMg4QcfEs7UgBXklrCEcVMHZOWxCXtI5drEnQ5HcdU/edit?usp=sharing).
