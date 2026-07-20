Welcome to the EF Tracking implementation of traccc as-a-Service with NVIDIA Triton Inference Server. This document will demonstrate how to build, test, and run the server, as well as how to use it with our Python client. An Athena client edition will be updated.

This repository runs traccc as-a-Service. This uses a custom backend, with a wrapper for GPU pipeline information, to launch the Triton server. This Triton server is launched with a model algorithm to transfer information between the client and the GPU. Currently, the model is built for the G200 pipeline, with more models to be included at a later date.


The following description will detail the setup for a working version of traccc as-a-Service with G200.
This traccc setup is currently compatible at traccc v1.0.0, athena 25.0.45.


Environment Setup

In order to run, you need all the dependencies to build and run.  A sample Docker image has been preincluded and converted to a .sif for the Apptainer environment (traccc-aas_v1p4_report.sif). Else, a Dockerfile has been included in order to create your image if you so choose. These are located under /EFTritonAlgsPipelines/data/ .

If creating your own image:

Run this in a machine where you have sudo privileges, ideally. Make sure you have the directory in that machine. If running everything on a different machine than where the image is being built, then you only need that directory in the building machine.
Now we’re going to build the docker image and then turn it into a .sif that Apptainer can run! You may choose your own image name for <image_name>.

sudo docker build -t <image_name> .
sudo apptainer build <path_for_image_location>/<image_name>.sif docker-daemon://<image name>:latest

If your machine gives you issues and says it doesn't have enough room to build the image, move the cache directory to a tmp directory. In that case, here's the build instructions:

sudo docker build -t <image name> .

mkdir -p /<path>/apptainer-tmp

sudo APPTAINER_TMPDIR=/<path_for_new_tmp>/apptainer-tmp \      
  APPTAINER_CACHEDIR=/<path_for_new_tmp>/apptainer-tmp \
  apptainer build /<path_for_image_location>/<image_name>.sif \ 
  docker-daemon://<image_name>:latest

Make sure to copy the image back to the proper directory if working from a different machine.

Server Setup

To start up the Apptainer environment (assuming image name is the same as the sample), run the following command.

apptainer run --nv \
  --bind "$(pwd)/EFTriton:/work" \
  --bind /eos/project/a/atlas-eftracking/GPU/ITk_data/ATLAS-P2-RUN4-03-00-01:/geoDir \
  EFTriton/EFTritonAlgsPipelines/data/env/traccc-aas_v1p4_report.sif
Testing Wrappers

The wrappers, or standalones, feed into the backend to supply GPU pipeline information. They exist in EFTriton/EFTritonAlgsPipelines/src (.cpp) and EFTriton/EFTritonAlgsPipelines/EFTritonPipelines (.hpp). If modifying these and wish to test them with our sample event, compile and run using the following code from EFTritonAlgsPipelines. (If the cmake doesn’t work in Apptainer, run unset CC && unset CXX)

mkdir build
cd build
cmake ../
cmake --build .

./TracccG200Standalone ../../EFTritonTester/event000000000-cells.csv 0

Building Server Backend: EFTritonRunner

This is where we compile the full GPU/traccc setup for the backend. 
Under EFTritonRunner, the backend file is /src/traccc-g200.cc, which contains the Triton backend information and includes the GPU wrappers within. 
Compiling this will create our model, which will be placed in a new directory labeled model_g200 upon compilation, alongside a copy of the configuration file, config.pbtxt (which you can find and modify under /cfg). 

To compile the G200 backend from EFTritonRunner/G200:

mkdir build
cd build/
cmake -B . -S ../     -DCMAKE_INSTALL_PREFIX=../install/     -DCMAKE_BUILD_TYPE=Release
cmake --build . --target install -- -j20

Starting Triton Server

After building the backend and creating the model repository with the necessary two files, you may now start up the Triton server!

 tritonserver \
  --model-repository=/athena/Trigger/EFTracking/EFTrackAaS/EFTriton/EFTriton/EFTritonRunner/G200/model_g200/traccc-g200 \
  --load-model=traccc-g200 \
  --log-verbose=1

If seeking dynamic loading across multiple models, include the tag --model-control-mode=explicit \ .

Testbed Connections

If running this across multiple testbeds, consult this section.
Open a new terminal on the node you would like your client to be located.
Run the following command to connect the server node to the client node (in this example, that is connecting to G01, but can change G01 to whatever you want.)

ssh -L 8001:localhost:8001 $USER@ef-tb-g01



Client Running

To run the client, open a new terminal on the client node of your choosing. If not connecting testbeds in the following section, do this in the same node as your active Triton server.

Go to /EFTritonTester and run the following command.

python TracccTritonClient.py


For more details, a version of non-Athena based general traccc-aaS can be located at https://github.com/milescb/traccc-aaS/tree/main/client, and a CodiMD for tracking as a service with Athena is found at https://codimd.web.cern.ch/1FcLmapORpeBtAVL_M6h4A


