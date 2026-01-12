FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential build tools and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    clang \
    clang-tidy \
    clang-format \
    cppcheck \
    valgrind \
    gcovr \
    gdb \
    ninja-build \
    pkg-config \
    curl \
    zip \
    unzip \
    tar \
    sudo \
    bc \
    lcov \
    && rm -rf /var/lib/apt/lists/*

# Install additional tools for code quality
RUN pip3 install --no-cache-dir \
    cpplint \
    conan \
    platformio \
    gcovr

# Create a non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Set up working directory
WORKDIR /workspace

# Copy the entire project into the container
COPY . /workspace/

# Set permissions for the workspace and analysis script
RUN chown -R $USERNAME:$USERNAME /workspace \
    && chmod +x /workspace/scripts/run-analysis.sh \
    && mkdir -p /workspace/build \
    && chown -R $USERNAME:$USERNAME /workspace/build \
    && mkdir -p /workspace/coverage \
    && chown -R $USERNAME:$USERNAME /workspace/coverage

# Create symlinks for clang tools to ensure they're in PATH
RUN ln -s /usr/bin/clang-format /usr/local/bin/clang-format \
    && ln -s /usr/bin/clang-tidy /usr/local/bin/clang-tidy

# Switch to non-root user
USER $USERNAME

# Set environment variables for tools
ENV PATH="/workspace/scripts:${PATH}"
ENV PYTHONPATH="/workspace/test:${PYTHONPATH:-}"

# Default command
CMD ["/bin/bash"] 