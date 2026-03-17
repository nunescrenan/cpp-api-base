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

DB_DRIVER=${DB_DRIVER:-mysql}
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

db_exists() {
    mysql_cmd -e "USE \`$DB_NAME\`;" >/dev/null 2>&1
}

migrations_table_exists() {
    mysql_cmd "$DB_NAME" -e "SELECT 1 FROM migrations LIMIT 1;" >/dev/null 2>&1
}

migration_exists() {
    local name="$1"
    if [ "$name" = "000_create_migrations_table.sql" ]; then
        migrations_table_exists
        return $?
    fi
    mysql_cmd "$DB_NAME" -e "SELECT 1 FROM migrations WHERE name='$name' LIMIT 1;" 2>/dev/null | grep -q "1"
}

record_migration() {
    local name="$1"
    mysql_cmd "$DB_NAME" -e "INSERT INTO migrations (name) VALUES ('$name');" >/dev/null 2>&1
}

run_migration_file() {
    local file="$1"
    mysql_cmd "$DB_NAME" < "$file" >/dev/null 2>&1
}

cmd_create() {
    info "Creating database: $DB_NAME"
    if db_exists; then
        skip "Database already exists"
    else
        if mysql_cmd -e "CREATE DATABASE \`$DB_NAME\`;" >/dev/null 2>&1; then
            success "Database created"
        else
            error "Failed to create database"
            exit 1
        fi
    fi
}

cmd_migrate() {
    info "Running migrations"
    cmd_create

    local files=($(ls database/migrations/*.sql 2>/dev/null | sort))
    local ran=0
    local skipped=0

    for file in "${files[@]}"; do
        if [ -f "$file" ]; then
            local name=$(basename "$file")
            if migration_exists "$name"; then
                skip "$name"
                ((skipped++))
            else
                info "Running: $name"
                if run_migration_file "$file"; then
                    if [ "$name" != "000_create_migrations_table.sql" ]; then
                        record_migration "$name"
                    fi
                    success "$name"
                    ((ran++))
                else
                    error "Failed: $name"
                    exit 1
                fi
            fi
        fi
    done

    success "$ran ran, $skipped skipped"
}

cmd_seed() {
    info "Running seeders"

    local count=0
    for file in database/seeders/*.sql; do
        if [ -f "$file" ]; then
            local name=$(basename "$file")
            info "Running: $name"
            if mysql_cmd "$DB_NAME" < "$file" >/dev/null 2>&1; then
                success "$name"
                ((count++))
            else
                error "Failed: $name"
                exit 1
            fi
        fi
    done

    success "$count seeder(s) completed"
}

cmd_fresh() {
    info "Dropping database: $DB_NAME"
    mysql_cmd -e "DROP DATABASE IF EXISTS \`$DB_NAME\`;" >/dev/null 2>&1
    success "Database dropped"
    cmd_migrate
    cmd_seed
}

cmd_status() {
    echo -e "${BOLD}${BLUE}Database status${RESET}"
    info "Driver:   $DB_DRIVER"
    info "Host:     $DB_HOST:$DB_PORT"
    info "Database: $DB_NAME"

    if mysql_cmd -e "USE \`$DB_NAME\`;" >/dev/null 2>&1; then
        success "Connection: OK"
        echo -e "${BOLD}Tables:${RESET}"
        mysql_cmd "$DB_NAME" -e "SHOW TABLES;" 2>/dev/null | while read table; do
            echo -e "  ${GREEN}•${RESET} $table"
        done

        if migrations_table_exists; then
            echo -e "${BOLD}Migrations:${RESET}"
            mysql_cmd "$DB_NAME" -e "SELECT name, ran_at FROM migrations ORDER BY id;" 2>/dev/null | while read name ran_at; do
                echo -e "  ${GREEN}•${RESET} $name ${CYAN}($ran_at)${RESET}"
            done
        fi
    else
        error "Connection: Failed"
    fi
}

show_help() {
    echo -e "
${BOLD}Usage:${RESET}
  make db <command>

${BOLD}Commands:${RESET}
  ${GREEN}create${RESET}   Create the database
  ${GREEN}migrate${RESET}  Run all migrations
  ${GREEN}seed${RESET}     Run all seeders
  ${GREEN}fresh${RESET}    Drop database, migrate and seed
  ${GREEN}status${RESET}   Show database connection status
"
}

case "$1" in
    create)  cmd_create ;;
    migrate) cmd_migrate ;;
    seed)    cmd_seed ;;
    fresh)   cmd_fresh ;;
    status)  cmd_status ;;
    *)       show_help ;;
esac
