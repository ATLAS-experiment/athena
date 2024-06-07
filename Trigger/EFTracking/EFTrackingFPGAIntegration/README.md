# EFTracking Athena Integration for FPGA-based Accelerator Card
This repository hosts codes for the EFTracking 2nd (and beyond) demenstrator Athena integration targeting FPGA-based accelerator card. Each kernel has its own class and python steering script. Please check `src` and `python` for details.

## Getting Codes
There are two ways to get the codes
1. Sparse checkout
2. Full checkout

Considering that the integration might use some other packages such as `xAOD`, a full checkout is recommended to ease the setup. If you prefer to perform a sparse checkout, you can follow the ATLAS git tutorial [here](https://atlassoftwaredocs.web.cern.ch/gittutorial/git-clone/). Only the full checkout instructions are provided here.

### Full checkout
If you are familiar with athena and git, you don't have to follow this checkout instructions. The most important steps are to 
1. Add this repository `gitlab.cern.ch/zhcui/athena` as one of your athena remotes
2. Checkout the `EFT-Int-draft` branch

If you are not familiar with athena and git, please follow the instructions below

**Case 1: You don't have a local athena copy**

Pick one of the options below
```bash
# Option 1: use krb5 authentication
# Need to do kinit for each new terminal shell for git remote operation
kinit yourUsername@CERN.CH 
git clone https://:@gitlab.cern.ch:8443/zhcui/athena.git
git remote rename origin EFTracking
git switch EFTrk-Int-draft
# or for older git version
git checkout -b EFTrk-Int-draft EFTracking/EFTrk-Int-draft

# Options 2: use your ssh key
git clone ssh://git@gitlab.cern.ch:7999/zhcui/athena.git
git remote rename origin EFTracking
git switch EFTrk-Int-draft
# or for older git version
git checkout -b EFTrk-Int-draft EFTracking/EFTrk-Int-draft
```

**Case 2: You have a local athena copy**
```bash
cd athena
# Option 1: use krb5 authentication
# Need to do kinit for each new terminal shell for git remote operation
kinit yourUsername@CERN.CH
git remote add EFTracking https://:@gitlab.cern.ch:8443/zhcui/athena.git
git fetch EFTracking
git switch EFTrk-Int-draft
# or for older git version
git checkout -b EFTrk-Int-draft EFTracking/EFTrk-Int-draft


# Option 2: use your ssh key
git remote add EFTracking ssh://git@gitlab.cern.ch:7999/zhcui/athena.git
git fetch EFTracking
git switch EFTrk-Int-draft
# or for older git version
git checkout -b EFTrk-Int-draft EFTracking/EFTrk-Int-draft
```

### Updating codes
Once the repository is setup, you can use git pull (fetch and merge) to update the codes
```bash
git pull EFTracking EFTrk-Int-draft
```

## Compiling the codes
### Getting Docker image
You should have either Singularity/Apptainer or Docker on your machine. To get the AlmaLinux9 image with XRT (2022.2) installed:
```bash
# Docker
docker pull maxwellcui/athenaxrt:2022.2
# Singularity
singularity pull docker://maxwellcui/athenaxrt:2022.2
```

### Structure setup
Go to the directory that includes the `athena`
```bash
mkdir build run
# Now, ls should show athena, build, run
echo $'+ Trigger/EFTracking/EFTrackingIntegration\n- .*' > package_filter_EFT.txt
```