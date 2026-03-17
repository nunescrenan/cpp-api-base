#!/bin/bash

set -e

RED='\033[31m'
GREEN='\033[32m'
YELLOW='\033[33m'
BLUE='\033[34m'
CYAN='\033[36m'
BOLD='\033[1m'
RESET='\033[0m'

if [ -f .env ]; then
    export $(grep -v '^#' .env | xargs)
fi

DB_HOST=${DB_HOST:-localhost}
DB_PORT=${DB_PORT:-3306}
DB_USER=${DB_USER:-root}
DB_PASSWORD=${DB_PASSWORD:-}
DB_NAME=${DB_NAME:-cpp-api-base}

mysql_cmd() {
    mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASSWORD" -N "$@" 2>/dev/null
}

info()    { echo -e "${CYAN}[INFO]${RESET} ℹ $1"; }
success() { echo -e "${GREEN}[SUCCESS]${RESET} ✓ $1"; }
error()   { echo -e "${RED}[ERROR]${RESET} ✗ $1"; }
skip()    { echo -e "${YELLOW}[SKIPPED]${RESET} ○ $1"; }

generate_id() {
    openssl rand -hex 16
}

generate_secret() {
    openssl rand -base64 32 | tr -d '/+=' | head -c 48
}

cmd_create() {
    shift
    local name="$*"
    if [ -z "$name" ]; then
        error "Usage: make client create <name>"
        exit 1
    fi

    local client_id=$(generate_id)
    local client_secret=$(generate_secret)

    mysql_cmd "$DB_NAME" -e "INSERT INTO api_clients (client_id, client_secret, name, is_active) VALUES ('$client_id', '$client_secret', '$name', 1);" 2>/dev/null

    echo ""
    info "Client created: $name"
    echo -e "  ${GREEN}client_id:${RESET}     $client_id"
    echo -e "  ${GREEN}client_secret:${RESET} $client_secret"
    echo ""
    echo -e "  ${YELLOW}⚠ Save the secret now, it won't be shown again!${RESET}"
    echo ""
}

cmd_list() {
    echo ""
    echo -e "${BOLD}API Clients${RESET}"
    echo ""

    local count=$(mysql_cmd "$DB_NAME" -e "SELECT COUNT(*) FROM api_clients;" 2>/dev/null)

    if [ "$count" -eq 0 ]; then
        echo -e "  ${YELLOW}No clients found${RESET}"
        echo ""
        return
    fi

    mysql_cmd "$DB_NAME" -e "SELECT client_id, name, is_active, created_at FROM api_clients ORDER BY created_at DESC;" 2>/dev/null | while IFS=$'\t' read -r client_id name is_active created_at; do
        local status="${GREEN}active${RESET}"
        if [ "$is_active" != "1" ]; then
            status="${RED}inactive${RESET}"
        fi
        echo -e "  ${GREEN}•${RESET} $name"
        echo -e "    ${CYAN}id:${RESET} $client_id"
        echo -e "    ${CYAN}status:${RESET} $status"
        echo -e "    ${CYAN}created:${RESET} $created_at"
        echo ""
    done
}

cmd_remove() {
    local client_id="$2"
    if [ -z "$client_id" ]; then
        error "Usage: make client remove <client_id>"
        exit 1
    fi

    local name=$(mysql_cmd "$DB_NAME" -e "SELECT name FROM api_clients WHERE client_id='$client_id';" 2>/dev/null)

    if [ -z "$name" ]; then
        error "Client not found: $client_id"
        exit 1
    fi

    mysql_cmd "$DB_NAME" -e "DELETE FROM api_clients WHERE client_id='$client_id';" 2>/dev/null
    success "Client removed: $name ($client_id)"
}

cmd_disable() {
    local client_id="$2"
    if [ -z "$client_id" ]; then
        error "Usage: make client disable <client_id>"
        exit 1
    fi

    local name=$(mysql_cmd "$DB_NAME" -e "SELECT name FROM api_clients WHERE client_id='$client_id';" 2>/dev/null)

    if [ -z "$name" ]; then
        error "Client not found: $client_id"
        exit 1
    fi

    mysql_cmd "$DB_NAME" -e "UPDATE api_clients SET is_active=0 WHERE client_id='$client_id';" 2>/dev/null
    success "Client disabled: $name ($client_id)"
}

cmd_enable() {
    local client_id="$2"
    if [ -z "$client_id" ]; then
        error "Usage: make client enable <client_id>"
        exit 1
    fi

    local name=$(mysql_cmd "$DB_NAME" -e "SELECT name FROM api_clients WHERE client_id='$client_id';" 2>/dev/null)

    if [ -z "$name" ]; then
        error "Client not found: $client_id"
        exit 1
    fi

    mysql_cmd "$DB_NAME" -e "UPDATE api_clients SET is_active=1 WHERE client_id='$client_id';" 2>/dev/null
    success "Client enabled: $name ($client_id)"
}

show_help() {
    echo -e "
${BOLD}Usage:${RESET}
  make client <command> [value]

${BOLD}Commands:${RESET}
  ${GREEN}create${RESET}  make client create <name>
  ${GREEN}list${RESET}    make client list
  ${GREEN}remove${RESET}  make client remove <client_id>
  ${GREEN}disable${RESET} make client disable <client_id>
  ${GREEN}enable${RESET}  make client enable <client_id>
"
}

case "$1" in
    create)  cmd_create "$@" ;;
    list)    cmd_list ;;
    remove)  cmd_remove "$@" ;;
    disable) cmd_disable "$@" ;;
    enable)  cmd_enable "$@" ;;
    *)       show_help ;;
esac
