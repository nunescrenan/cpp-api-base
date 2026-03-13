#!/usr/bin/env bash

set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir_name="build"

if [[ $# -gt 0 && "$1" != -* ]]; then
    build_dir_name="$1"
    shift
fi

if [[ "$build_dir_name" = /* ]]; then
    build_dir="$build_dir_name"
else
    build_dir="$repo_root/$build_dir_name"
fi

resolve_vcpkg_root() {
    local candidate_bin

    if [[ -n "${VCPKG_ROOT:-}" ]]; then
        if [[ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]]; then
            printf '%s\n' "$VCPKG_ROOT"
            return 0
        fi

        printf 'VCPKG_ROOT esta definido, mas o toolchain nao foi encontrado em %s\n' "$VCPKG_ROOT" >&2
        return 1
    fi

    for candidate_bin in \
        "${HOMEBREW_PREFIX:-}/bin/vcpkg" \
        /opt/homebrew/bin/vcpkg \
        /usr/local/bin/vcpkg
    do
        if [[ -n "$candidate_bin" && -x "$candidate_bin" ]]; then
            local vcpkg_root

            vcpkg_root="$(cd "$(dirname "$candidate_bin")/.." && pwd)"

            if [[ -f "$vcpkg_root/scripts/buildsystems/vcpkg.cmake" ]]; then
                printf '%s\n' "$vcpkg_root"
                return 0
            fi
        fi
    done

    if command -v vcpkg >/dev/null 2>&1; then
        local vcpkg_bin
        local vcpkg_root

        vcpkg_bin="$(command -v vcpkg)"
        vcpkg_root="$(cd "$(dirname "$vcpkg_bin")/.." && pwd)"

        if [[ -f "$vcpkg_root/scripts/buildsystems/vcpkg.cmake" ]]; then
            printf '%s\n' "$vcpkg_root"
            return 0
        fi
    fi

    printf 'Nao foi possivel localizar o vcpkg. Instale-o ou defina VCPKG_ROOT.\n' >&2
    return 1
}

resolve_drogon_prefix() {
    if [[ -n "${Drogon_DIR:-}" && -f "${Drogon_DIR}/DrogonConfig.cmake" ]]; then
        printf '%s\n' "$(cd "${Drogon_DIR}/../../.." && pwd)"
        return 0
    fi

    if [[ -x /opt/homebrew/bin/brew ]]; then
        local brew_prefix

        brew_prefix="$(/opt/homebrew/bin/brew --prefix drogon 2>/dev/null || true)"

        if [[ -n "$brew_prefix" && -d "$brew_prefix" ]]; then
            printf '%s\n' "$brew_prefix"
            return 0
        fi
    fi

    return 1
}

read_cached_source_dir() {
    local cache_file="$1/CMakeCache.txt"

    if [[ ! -f "$cache_file" ]]; then
        return 1
    fi

    awk -F= '/^CMAKE_HOME_DIRECTORY:INTERNAL=/{print $2}' "$cache_file"
}

reset_build_dir_if_needed() {
    local cached_source_dir

    cached_source_dir="$(read_cached_source_dir "$build_dir" || true)"

    if [[ -z "$cached_source_dir" ]]; then
        return 0
    fi

    if [[ "$cached_source_dir" != "$repo_root" ]]; then
        printf 'Limpando cache antigo de CMake em %s\n' "$build_dir"
        rm -rf "$build_dir"
    fi
}

reset_build_dir_if_needed
mkdir -p "$build_dir"

cmake_args=(
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
)

if drogon_prefix="$(resolve_drogon_prefix)"; then
    printf 'Usando Drogon em %s\n' "$drogon_prefix"
    cmake_args+=("-DCMAKE_PREFIX_PATH=$drogon_prefix")
else
    vcpkg_root="$(resolve_vcpkg_root)"
    toolchain_file="$vcpkg_root/scripts/buildsystems/vcpkg.cmake"
    printf 'Usando toolchain do vcpkg em %s\n' "$toolchain_file"
    cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$toolchain_file")
fi

printf 'Configurando projeto em %s\n' "$build_dir"
cmake -S "$repo_root" -B "$build_dir" \
    "${cmake_args[@]}" \
    "$@"

printf 'Setup concluido. Proximo passo: cmake --build %s\n' "$build_dir_name"
