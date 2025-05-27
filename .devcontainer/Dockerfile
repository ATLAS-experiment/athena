# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Start from the base CERN Alma 9 image.
FROM cern/alma9-base:latest

# Perform the image setup as root.
USER root
WORKDIR /root

# Install some additional packages.
RUN yum -y install which git wget tar atlas-devel libuuid-devel texinfo \
           libX11-devel libXpm-devel libXft-devel  \
           libXext-devel openssl-devel glibc-devel rpm-build libcurl-devel     \
           mesa-libGL-devel libaio make expat-devel && \
    yum clean all

# Set up the ATLAS user, and give it super user rights.
RUN echo '%wheel	ALL=(ALL)	NOPASSWD: ALL' >> /etc/sudoers && \
    adduser atlas && chmod 755 /home/atlas && \
    usermod -aG wheel atlas

# Copy the interactive environment setup scripts in place.
COPY atlas_prompt.sh setup_atlas.sh /etc/profile.d/
# Add the message of the day instructions for the image.
COPY motd /etc/


# Switch to the ATLAS account.
USER atlas
WORKDIR /home/atlas
RUN echo 'cat /etc/motd' >> ~/.bashrc && \
    echo 'source /etc/profile.d/setup_atlas.sh' >> ~/.bashrc


# Set up the command to run when starting the image.
CMD ["/bin/bash", "-l"]
