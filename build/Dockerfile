FROM ubuntu:latest
LABEL maintainer="<sra-tools@ncbi.nlm.nih.gov>"
RUN apt-get update && \
    apt-get install -y sudo git-core make g++ && \
    adduser --quiet --disabled-password --shell /bin/bash --home /home/devuser --gecos "User" devuser && \
    echo "devuser:sra" | chpasswd && usermod -aG sudo devuser
