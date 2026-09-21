#!/bin/sh
# Kindle Keyboard (Kindle 3 / K3) Telegram Client Launcher
# Stops the Kindle framework before running the client and restores it on exit.

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
REPOSITORY_DIR="$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)"

if [ -x "$SCRIPT_DIR/kindle-telegram" ]; then
    APPLICATION_DIR="$SCRIPT_DIR"
elif [ -x "$REPOSITORY_DIR/bin/kindle-telegram" ]; then
    APPLICATION_DIR="$REPOSITORY_DIR/bin"
else
    echo "Binary not found beside the launcher or in $REPOSITORY_DIR/bin." >&2
    exit 1
fi

LOG="$APPLICATION_DIR/kindle-telegram.log"

restore_kindle() {
    echo "[Kindle Telegram] Restoring Kindle framework..."
    /etc/init.d/framework start 2>/dev/null || true
    lipc-set-prop com.lab126.powerd -i deferSuspend 1 2>/dev/null || true
}

trap restore_kindle EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

echo "[Kindle Telegram] Stopping Kindle Java framework to free RAM..."
/etc/init.d/framework stop 2>/dev/null || true

# Ensure session storage directory exists.
mkdir -p /mnt/us/telegram 2>/dev/null || mkdir -p "$HOME/.telegram" 2>/dev/null || true

echo "[Kindle Telegram] Launching Telegram Client..."
cd "$APPLICATION_DIR" || exit 1
./kindle-telegram > "$LOG" 2>&1
client_status=$?

echo "[Kindle Telegram] Exited with status $client_status."
exit "$client_status"
