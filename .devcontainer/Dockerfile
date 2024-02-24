FROM mcr.microsoft.com/devcontainers/cpp:1-debian-11

# Copy SSH keys to the container
COPY .secret/.ssh /home/vscode/.ssh

# Copy gitconfig to the container
COPY .secret/.gitconfig /home/vscode/.gitconfig

# Copy wakatime config to the container
COPY .secret/.wakatime.cfg /home/vscode/.wakatime.cfg

RUN <<EOF
chown -R vscode:vscode /home/vscode/.ssh
chown -R vscode:vscode /home/vscode/.gitconfig
chown vscode:vscode /home/vscode/.wakatime.cfg
EOF

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends clangd
