FROM ubuntu:22.04

# Avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install basic dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    vim \
    wget \
    lsb-release \
    software-properties-common \
    gnupg \
    ninja-build \
    pkg-config \
    curl \
    gdb \
    valgrind \
    linux-tools-generic \
    ccache \
    && rm -rf /var/lib/apt/lists/* \
    && git config --system core.editor "vim"

# Install LLVM 18
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 18 && \
    apt-get install -y clang-18 lldb-18 lld-18 clangd-18

# Set Clang 18 as default
RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-18 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-18 100

# Install Google Benchmark
RUN git clone https://github.com/google/benchmark.git && \
    cd benchmark && \
    cmake -E make_directory "build" && \
    cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../ && \
    cmake --build "build" --config Release && \
    cmake --build "build" --config Release --target install && \
    cd / && rm -rf /benchmark

# Install Google Test
RUN git clone https://github.com/google/googletest.git && \
    cd googletest && \
    cmake -E make_directory "build" && \
    cmake -E chdir "build" cmake ../ && \
    cmake --build "build" --config Release && \
    cmake --build "build" --config Release --target install && \
    cd / && rm -rf /googletest

    