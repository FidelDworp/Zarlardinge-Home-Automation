#!/bin/bash
# ─────────────────────────────────────────────────────
# Zarlar update.sh — zet de RPi gelijk met GitHub
# Gebruik (vanaf de Mac):  ssh fidel@192.168.0.50 'bash ~/update.sh'
#   van buitenhuis:        ssh fidel@100.123.74.113 'bash ~/update.sh'
#
# v2 (7 nov 2026):
#   - Vergelijkt de bestanden met wat op de RPi draait (niet meer met HEAD@{1},
#     dat na een rebase niet altijd de vorige versie is). Zo wordt een nieuwe
#     server.js altijd herkend, ook als een vorige update halverwege stopte.
#   - Apple Home-plugin (matterbridge-zarlar): gewijzigd op GitHub → kopiëren
#     naar Matterbridge en de container herstarten. Geen tar.gz meer nodig.
#   - update.sh werkt zichzelf bij (geldt vanaf de volgende keer).
#   - Stopt bij een fout, met een duidelijke melding.
# v1: git pull, public/ synchroniseren, server.js kopiëren als gewijzigd.
# ─────────────────────────────────────────────────────

REPO="/home/fidel/repo"                       # kloon van Zarlardinge-Home-Automation
SUBMAP="ESP32_Zarlar/zarlar-rpi"
ZARLAR="/home/fidel/zarlar-dashboard"         # het draaiende portaal
MB_DATA="/home/fidel/Matterbridge"            # plugins van Matterbridge (volume van de container)
MB_COMPOSE="/home/fidel/matterbridge"         # docker-compose van Matterbridge
PLUGIN="matterbridge-zarlar"

stop() {
  echo "✗ $1"
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  exit 1
}

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Zarlar update — $(date '+%d/%m/%Y %H:%M')"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 1. Laatste versie van GitHub
cd "$REPO" || stop "Repo niet gevonden: $REPO"
OUD=$(git rev-parse --short HEAD)
echo "→ GitHub ophalen..."
git fetch -q origin || stop "git fetch mislukt (internet of GitHub niet bereikbaar)."
git rebase -q origin/main || stop "git rebase mislukt. Los op met: cd $REPO && git status"
NIEUW=$(git rev-parse --short HEAD)
if [ "$OUD" = "$NIEUW" ]; then
  echo "  al up-to-date ($NIEUW)"
else
  echo "  $OUD → $NIEUW:"
  git log --format='    %h %s' "$OUD..$NIEUW" | head -10
fi
BRON="$REPO/$SUBMAP"

# 2. Webpagina's
echo "→ public/ synchroniseren..."
rsync -a "$BRON/public/" "$ZARLAR/public/" || stop "rsync van public/ mislukt."

# 3. server.js (alleen herstarten als hij verschilt van de draaiende versie)
if cmp -s "$BRON/server.js" "$ZARLAR/server.js"; then
  echo "→ server.js ongewijzigd"
else
  echo "→ server.js gewijzigd — kopiëren en herstarten..."
  cp "$BRON/server.js" "$ZARLAR/server.js" || stop "server.js kopiëren mislukt."
  sudo systemctl restart zarlar || stop "Herstart van de zarlar-dienst mislukt."
  echo "  ✓ portaal herstart"
fi

# 4. Apple Home-plugin
if [ -d "$BRON/$PLUGIN" ]; then
  if diff -rq --exclude=node_modules "$BRON/$PLUGIN" "$MB_DATA/$PLUGIN" >/dev/null 2>&1; then
    echo "→ plugin $PLUGIN ongewijzigd"
  else
    echo "→ plugin $PLUGIN gewijzigd — kopiëren en Matterbridge herstarten..."
    sudo cp -r "$BRON/$PLUGIN/." "$MB_DATA/$PLUGIN/" || stop "Plugin kopiëren mislukt."
    ( cd "$MB_COMPOSE" && sudo docker compose restart ) || stop "Herstart van Matterbridge mislukt."
    echo "  wachten tot Matterbridge opgestart is (40 s)..."
    sleep 40
    sudo docker logs --since 1m matterbridge 2>&1 | grep -iE "Zarlar v|knoppen geregistreerd|error" | tail -5
    echo "  ✓ Matterbridge herstart (blijven tegels weg: Keuken Geel Bolleke herstarten)"
  fi
fi

# 5. update.sh zelf
if ! cmp -s "$BRON/update.sh" "$HOME/update.sh"; then
  # via een kopie + mv: het script dat nu draait, mag niet overschreven worden
  cp "$BRON/update.sh" "$HOME/update.sh.nieuw" && chmod +x "$HOME/update.sh.nieuw" && mv -f "$HOME/update.sh.nieuw" "$HOME/update.sh"
  echo "→ update.sh bijgewerkt (geldt vanaf de volgende keer)"
fi

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Klaar! RPi = GitHub ($NIEUW)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
