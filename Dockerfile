FROM debian:latest

RUN dpkg --add-architecture armhf

# Install required build tools
RUN apt-get update && apt-get install -y \
    which \
    sed \
    make \
    binutils \
    build-essential \
    diffutils \
    gcc \
    g++ \
    bash \
    patch \
    gzip \
    bzip2 \
    perl \
    tar \
    cpio \
    unzip \
    rsync \
    file \
    bc \
    findutils \
    wget \
    python3 \
    python3-pip \
    ncurses-dev \
    qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools \
    libglib2.0-dev \
    libgtk2.0-dev \
    openssh-client \
    subversion \
    default-jdk \
    asciidoc \
    w3m \
    graphviz \
    dblatex \
    gcc-arm-linux-gnueabihf \
    tree

RUN apt-get install -y libpulse-dev:armhf libgpiod-dev:armhf

RUN apt-get install -y python3-matplotlib

# Install optional packages (recommended dependencies)
RUN apt-get install -y \
    cvs \
    git \
    mercurial \
    openssh-client \
    subversion \
    graphviz

# Clean up
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy entrypoint script into the container
COPY entrypoint.sh /entrypoint.sh

# Set entrypoint script as executable
RUN chmod +x /entrypoint.sh

# Set entrypoint for the container
ENTRYPOINT ["/entrypoint.sh"]

