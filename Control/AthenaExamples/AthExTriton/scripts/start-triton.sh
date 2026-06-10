#!/usr/bin/env bash
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Triton requires a model repository
MODEL_FILE="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/MLTest/2020-03-02/MNIST_testModel.onnx"
MODEL_REPO=$(mktemp --tmpdir -d model_repo_XXXXX)
mkdir -p "$MODEL_REPO"/MNIST_testModel/1
cp $MODEL_FILE "$MODEL_REPO"/MNIST_testModel/1/model.onnx

# Figure out container runtime
if command -v podman-hpc 2>/dev/null; then
    CONTCMD="podman-hpc run --gpu"
elif command -v podman 2>/dev/null; then
    CONTCMD="podman run"
elif command -v docker 2>/dev/null; then
    CONTCMD="docker run"
    echo -e '\033[0;31m' WARNING: Using docker. May fail without correct permissions. '\033[0m'
else
    echo -e '\033[0;31m' ERROR: No docker-compatible container runtime found. '\033[0m'
    exit 1
fi

# Start container
IMAGE_PATH='nvcr.io/nvidia/tritonserver:26.04-py3'
$CONTCMD --gpus all --rm --net host \
    -v "$MODEL_REPO":/models $IMAGE_PATH \
    tritonserver --model-repository=/models --log-verbose=1
rm -r "$MODEL_REPO"
