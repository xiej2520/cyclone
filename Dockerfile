FROM ubuntu:18.04

WORKDIR /cyclone

RUN apt-get update && \
    apt-get install -y \
    build-essential gcc-multilib

COPY . .

# Build Cyclone for i686
RUN sh do-conf-32 && \
    ./mk-it-32.sh && \
    ./mk-it-32.sh install && \
    cp -r /cyclone/build/cyclone-32/* /usr/local/ && \
    tar zcf cyclone-linux-i686.tgz /cyclone/build/cyclone-32/*


# Run tests for i686 binaries
RUN ./mk-it-32.sh test_bin

# Build Cyclone bootstrap for i686
RUN ./mk-it-32.sh cyclone_src

# Run tests for bootstrap
RUN ./mk-it-32.sh test_boot

# Define the command to run when the container starts
CMD ["bash"]
