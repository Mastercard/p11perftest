#!/usr/bin/env bash
#
# Copyright (c) 2025 Mastercard

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#   http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
set -e

cleanup() {
  echo "Caught SIGINT. Exiting..."
  exit 1
}

trap cleanup SIGINT  # Handle SIGINT (CTRL-C)

PACKAGE="p11perftest"
GITHUB_REPO="https://github.com/Mastercard/p11perftest"
GITHUB_REPO_COMMIT="HEAD"
SUPPORTED_ARCHS="amd64 arm64"
SUPPORTED_DISTROS="ol8 ol9 deb12 deb13 ubuntu2204 ubuntu2404 amzn2023 alpine321"

function host_arch_to_buildx_arch() {
    case "$1" in
        x86_64) echo "amd64" ;;
        aarch64|arm64) echo "arm64" ;;
        *) echo "$1" ;;
    esac
}

function buildx_arch_to_platform_arch() {
    case "$1" in
        amd64) echo "amd64" ;;
        arm64) echo "arm64" ;;
        x86_64) echo "amd64" ;;
        aarch64) echo "arm64" ;;
        *) echo "$1" ;;
    esac
}

function detect_numprocs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
        return
    fi
    if command -v getconf >/dev/null 2>&1; then
        getconf _NPROCESSORS_ONLN 2>/dev/null && return
    fi
    if command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.logicalcpu 2>/dev/null && return
    fi
    echo 1
}

#
# Rewrite a proxy URL so that it is reachable from inside a build container.
# A proxy listening on the host loopback interface cannot be reached as
# 'localhost' from a container, since that resolves to the container itself.
# Docker exposes the host as 'host.docker.internal' instead.
#
function container_reachable_proxy() {
    echo "$1" | sed -E 's#//(localhost|127\.0\.0\.1|\[::1\])(:|/|$)#//host.docker.internal\2#'
}

#
# Usage information
#
function usage() {
    echo "Build package(s) for multiple distros and architectures using Docker buildx."
    echo ""
    echo "Usage: $0 [-r URL] [-v] [-j N] [-c COMMIT] [-p FILE] [--config-args ARGS] (distro[/arch]|all[/all]) [...]"
    echo "Supported distros: $SUPPORTED_DISTROS"
    echo "Supported archs: $SUPPORTED_ARCHS"
    echo ""
    echo "Options:"
    echo "  --repo URL, -r URL           Repository URL (default: $GITHUB_REPO)"
    echo "  --commit COMMIT, -c COMMIT   Commit hash, tag or branch to build (default: $GITHUB_REPO_COMMIT)"
    echo "  --skip-git-sslverify, -k     Disable SSL verification for git clone in Docker build"
    echo "  --proxy URL, -P URL          HTTP(S) proxy to use inside the build (e.g. http://localhost:8118)."
    echo "                                A loopback address is automatically rewritten to"
    echo "                                host.docker.internal so that the container can reach it."
    echo "                                Defaults to the http_proxy/https_proxy environment variables."
    echo "  --noproxy LIST               Comma-separated hosts that must bypass the proxy"
    echo "                                (default: localhost,127.0.0.1,::1)"
    echo "  --ignore-proxy               Ignore proxy environment variables during Docker build"
    echo "  --local-source               Build from local Docker context instead of cloning repository"
    echo "  --verbose, -v                Increase verbosity (can be specified multiple times)"
    echo "  --no-cache, -n               Do not use docker build cache"
    echo "  --max-procs N, -j N          Maximum number of concurrent build processes (default: all available CPUs)"
    echo "  --proxyrootca FILE, -x FILE  Root CA file to use for the build"
    echo "  --extra-header FILE, -H FILE Additional cryptoki header (.h) to inject under src/cryptoki/"
    echo "                                in the build (read-only mount, repeatable). Overrides the"
    echo "                                forwarding pkcs11.h shipped with p11perftest."
    echo "  --package NAME, -p NAME      Package name to use for build tags (default: $PACKAGE)"
    echo "  --config-args ARGS           Additional arguments to pass to the configure script"
    echo "  --help, -h                   Show this help message"
    echo ""
    exit 1
}

#
# Get the current directory
#
function get_current_dir() {
    current_dir="$(pwd)"
    echo "${current_dir}"
}

#
# Get the directory of the script
#
function get_script_dir() {
    script_dir="$(cd "$(dirname "$0")" && pwd)"
    echo "${script_dir}"
}

#
# Generate a random container name
#
function gen_random_container_name() {
    random_docker_name=$(head -c 16 /dev/urandom | base64 | tr -dc 'a-z0-9' | head -c 12)
    echo -n "container-$PACKAGE-$random_docker_name"
}

#
# Get the current git tag or commit hash if current commit is not tagged
#
function get_git_tag_or_hash() {
    # Get the current tag if it exists, otherwise get the short commit hash
    git describe --tags --abbrev=0 2>/dev/null || git rev-parse --short HEAD
}

#
# Copy the root CA file to the script directory (which is the context)
#
function copy_root_ca() {
    local rootca="$1"
    local script_dir=$(get_script_dir)

    # Check if the file exists
    if [ -f "$rootca" ]; then
        cp "$rootca" "$script_dir/proxyrootca.crt"
        echo "Root CA file copied to $script_dir/proxyrootca.crt"
    else
        echo "Root CA file not found: $rootca"
        exit 1
    fi
}

#
# Build the artifacts for the given distro and arch
#
# $1  - package
# $2  - distro
# $3  - arch
# $4  - verbose: 0, 1 or 2
# $5  - no cache: 0 or 1
# $6  - repo_url
# $7  - repo_commit
# $8  - proxyrootca
# $9  - config_args
# $10 - repo_sslverify
# $11 - ignore_proxy
# $12 - source_mode
# $13 - extra_headers_dir
# $14 - proxy_url (empty when unused)
# $15 - no_proxy_list

function create_build() {
    set -e                      # Exit on error, repeated here to ensure it's set in the subshell

    local package="$1"
    local distro="$2"
    local arch="$3"
    local verbose="$4"
    local no_cache="$5"
    local repo_url="$6"
    local repo_commit="$7"
    local proxyrootca="$8"
    local config_args="$9"
    local repo_sslverify="${10}"
    local ignore_proxy="${11}"
    local source_mode="${12}"
    local extra_headers_dir="${13}"
    local proxy_url="${14}"
    local no_proxy_list="${15}"

    local verbosearg="--quiet"

    if [ "$verbose" -eq 1 ]; then
        verbosearg="--progress=auto"
    elif [ "$verbose" -eq 2 ]; then
        verbosearg="--progress=plain"
    fi

    local no_cachearg=""
    if [ "$no_cache" -eq 1 ]; then
        no_cachearg="--no-cache"
    fi

    # Proxy handling.
    # HTTP_PROXY, HTTPS_PROXY and NO_PROXY (and their lowercase variants) are
    # predefined build arguments: BuildKit injects them into the environment of
    # every RUN instruction, so no ARG declaration is needed in the Dockerfiles.
    local proxy_build_args=""
    if [ "$ignore_proxy" = "true" ]; then
        proxy_build_args="--build-arg HTTP_PROXY= --build-arg HTTPS_PROXY= --build-arg http_proxy= --build-arg https_proxy= --build-arg ALL_PROXY= --build-arg all_proxy= --build-arg NO_PROXY= --build-arg no_proxy="
    elif [ -n "$proxy_url" ]; then
        local container_proxy
        container_proxy="$(container_reachable_proxy "$proxy_url")"
        proxy_build_args="--build-arg HTTP_PROXY=$container_proxy --build-arg HTTPS_PROXY=$container_proxy --build-arg http_proxy=$container_proxy --build-arg https_proxy=$container_proxy --build-arg NO_PROXY=$no_proxy_list --build-arg no_proxy=$no_proxy_list"
    fi

    # Make the host reachable as host.docker.internal, including on engines
    # where it is not provided out of the box (plain Docker on Linux).
    local add_host_arg=""
    if [ "$ignore_proxy" != "true" ] && [ -n "$proxy_url" ]; then
        add_host_arg="--add-host=host.docker.internal:host-gateway"
    fi

    local extra_headers_arg=""
    if [ -n "$extra_headers_dir" ] && [ "$extra_headers_dir" != "NONE" ]; then
        extra_headers_arg="--build-context extra-headers=$extra_headers_dir"
    fi

    local platformarch
    platformarch="$(buildx_arch_to_platform_arch "$arch")"

    echo "Building artifacts for $distro on arch $arch (platform: $platformarch)..."

    local containername=$(gen_random_container_name)
    docker buildx build $verbosearg $no_cachearg \
        --platform linux/$platformarch \
        --build-arg REPO_URL=$repo_url \
        --build-arg REPO_COMMIT_OR_TAG=$repo_commit \
        --build-arg REPO_SSLVERIFY=$repo_sslverify \
        --build-arg IGNORE_PROXY=$ignore_proxy \
        --build-arg SOURCE_MODE=$source_mode \
        --build-arg PROXY_ROOT_CA=$proxyrootca \
        --build-arg CONFIG_ARGS="$config_args" \
        $proxy_build_args \
        $add_host_arg \
        $extra_headers_arg \
        -t $package-build-$distro-$arch \
        -f $(get_script_dir)/buildx/Dockerfile.$distro \
        $(get_script_dir)

    local artifacts=$(docker run --platform linux/$platformarch --name $containername $package-build-$distro-$arch)
    for artifact in $artifacts; do
        docker cp $containername:$artifact $(get_current_dir)/
    done
    docker rm -f $containername > /dev/null 2>&1
    echo "Done with for $distro on $arch, produced artifacts:"
    for artifact in $artifacts; do
        echo "  $(get_current_dir)/$(basename $artifact)"
    done
}

# main function.

#
# Parse the arguments and execute the builds
#
function parse_and_build() {
    local package="$PACKAGE"
    local repo_url="$GITHUB_REPO"
    local repo_commit="$GITHUB_REPO_COMMIT"
    local verbose=0
    local no_cache=0
    local repo_sslverify="true"
    local ignore_proxy="false"
    local source_mode="git"
    local args=()
    local max_numprocs
    max_numprocs=$(detect_numprocs)
    local numprocs="$max_numprocs"
    local proxyrootca=""
    local config_args=""
    local extra_headers=()
    local proxy_url="${https_proxy:-${HTTPS_PROXY:-${http_proxy:-${HTTP_PROXY:-}}}}"
    local no_proxy_list="localhost,127.0.0.1,::1"

    # Parse optional arguments
    while [[ "$1" == --* || "$1" == -* ]]; do
        case "$1" in
            --package|-p)
                shift
                package="$1"
                ;;
            --repo|-r)
                shift
                repo_url="$1"
                ;;
            --commit|-c)
                shift
                repo_commit="$1"
                ;;
            --verbose|-v)
                if [ "$verbose" -lt 2 ]; then
                    verbose=$(($verbose + 1))
                fi
                ;;
            -vv)
                verbose=2
                ;;
            --skip-git-sslverify|-k)
                repo_sslverify="false"
                ;;
            --ignore-proxy)
                ignore_proxy="true"
                ;;
            --proxy|-P)
                shift
                proxy_url="$1"
                ;;
            --noproxy)
                shift
                no_proxy_list="$1"
                ;;
            --local-source)
                source_mode="local"
                ;;
            --no-cache|-n)
                no_cache=1
                ;;
            --max-procs|-j)
                shift
                numprocs="$1"
                # Validate the number of processes:
                # - Must be a positive integer
                # - Must be less than or equal to the number of CPUs
                if ! [[ "$numprocs" =~ ^[0-9]+$ ]] || [ "$numprocs" -le 0 ] || [ "$numprocs" -gt "$max_numprocs" ]; then
                    echo "Invalid number of processes: $numprocs"
                    usage
                fi
                ;;
            --proxyrootca|-x)
                shift
                proxyrootca="$1"
                ;;
            --extra-header|-H)
                shift
                if [ ! -f "$1" ]; then
                    echo "Extra header file not found: $1"
                    exit 1
                fi
                extra_headers+=("$1")
                ;;
            --config-args)
                shift
                config_args="$1"
                ;;
            --help|-h)
                usage
                ;;
            *)
                echo "Unknown option: $1"
                usage
                ;;
        esac
        shift
    done

    # proxy root CA must be treated from here
    # if proxyrootca is unset, create a dummy file
    # if proxyrootca is set, copy the root CA file to the script directory
    if [ -z "$proxyrootca" ]; then
        echo -n >$(get_script_dir)/proxyrootca.crt
    else
        copy_root_ca "$proxyrootca"
    fi
    proxyrootca="proxyrootca.crt"

    # If config_args is unset, set it to a dummy value
    if [ -z "$config_args" ]; then
        config_args="DUMMY=dummy"
    fi

    # Stage extra cryptoki headers (if any) into a single temp directory that
    # will be passed to docker buildx as the named build context "extra-headers".
    # The directory is bind-mounted read-only inside the build (see Dockerfiles).
    local extra_headers_dir="NONE"
    if [ "${#extra_headers[@]}" -gt 0 ]; then
        extra_headers_dir=$(mktemp -d -t p11perftest-extra-headers.XXXXXX)
        trap 'rm -rf "$extra_headers_dir"' EXIT
        for h in "${extra_headers[@]}"; do
            cp "$h" "$extra_headers_dir/"
        done
        echo "Staged ${#extra_headers[@]} extra cryptoki header(s) into $extra_headers_dir"
    fi

    # Collect remaining arguments
    local args=("$@")

    if [ "${#args[@]}" -eq 0 ]; then
        echo "No distro specified."
        usage
    fi

    local build_args=()

    for arg in "${args[@]}"; do
        if [[ "$arg" == "all/all" ]]; then
            for distro in $SUPPORTED_DISTROS; do
                for arch in $SUPPORTED_ARCHS; do
                    build_args+=("$package" "$distro" "$arch" "$verbose" "$no_cache" "$repo_url" "$repo_commit" "$proxyrootca" "$config_args" "$repo_sslverify" "$ignore_proxy" "$source_mode" "$extra_headers_dir" "$proxy_url" "$no_proxy_list")
                done
            done
        elif [[ "$arg" == "all" ]]; then
            local host_arch
            host_arch=$(host_arch_to_buildx_arch "$(uname -m)")
            for distro in $SUPPORTED_DISTROS; do
                build_args+=("$package" "$distro" "$host_arch" "$verbose" "$no_cache" "$repo_url" "$repo_commit" "$proxyrootca" "$config_args" "$repo_sslverify" "$ignore_proxy" "$source_mode" "$extra_headers_dir" "$proxy_url" "$no_proxy_list")
            done
        elif [[ "$arg" == */* ]]; then
            IFS='/' read -r distro arch_list <<< "$arg"
            if [[ "$arch_list" == "all" ]]; then
                for arch in $SUPPORTED_ARCHS; do
                    build_args+=("$package" "$distro" "$arch" "$verbose" "$no_cache" "$repo_url" "$repo_commit" "$proxyrootca" "$config_args" "$repo_sslverify" "$ignore_proxy" "$source_mode" "$extra_headers_dir" "$proxy_url" "$no_proxy_list")
                done
            else
                IFS=',' read -ra archs <<< "$arch_list"
                for arch in "${archs[@]}"; do
                    build_args+=("$package" "$distro" "$arch" "$verbose" "$no_cache" "$repo_url" "$repo_commit" "$proxyrootca" "$config_args" "$repo_sslverify" "$ignore_proxy" "$source_mode" "$extra_headers_dir" "$proxy_url" "$no_proxy_list")
                done
            fi
        else
            IFS=',' read -ra distros <<< "$arg"
            local host_arch
            host_arch=$(host_arch_to_buildx_arch "$(uname -m)")
            for distro in "${distros[@]}"; do
                build_args+=("$package" "$distro" "$host_arch" "$verbose" "$no_cache" "$repo_url" "$repo_commit" "$proxyrootca" "$config_args" "$repo_sslverify" "$ignore_proxy" "$source_mode" "$extra_headers_dir" "$proxy_url" "$no_proxy_list")
            done
        fi
    done

    export -f create_build
    export -f get_current_dir
    export -f get_script_dir
    export -f gen_random_container_name
    export -f buildx_arch_to_platform_arch
    export -f container_reachable_proxy
    export PACKAGE

    # Run builds in parallel, preserving argument boundaries with NUL separators.
    printf "%s\0" "${build_args[@]}" | xargs -0 -P "$numprocs" -n 15 bash -c 'create_build "$@"' _
}

#
# Main logic
#
if [[ "$#" -lt 1 ]]; then
    usage
fi

parse_and_build "$@"

# EOF
