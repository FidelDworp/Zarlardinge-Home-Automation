#!/bin/bash
# ─────────────────────────────────────────────────────
# deploy.sh — eigen bestanden uit Downloads naar GitHub én de RPi
# Gebruik: bash ~/deploy.sh "omschrijving van wijziging"
#
# Alleen nodig als FiDel ZELF een bestand aanpaste. Wat Claude op GitHub
# bijwerkt, komt op de RPi met:  ssh -t fidel@192.168.0.50 'bash ~/update.sh'
#
# v3 (7 nov 2026):
#   - Alleen gekende bestanden: server.js · update.sh · deploy.sh (root),
#     .html/.css en zarlar.js (public/), module.js · package.json ·
#     matterbridge-zarlar.config.json · matterbridge-zarlar.tar.gz (plugin).
#     Andere bestanden (ook .md-documenten en sketches) blijven in Downloads
#     staan en worden gemeld: documenten en sketches onderhoudt Claude op GitHub.
#   - Eerst de laatste versie van GitHub ophalen, dan pas kopiëren.
#   - deploy.sh werkt zichzelf bij (geldt vanaf de volgende keer).
#   - Oude repo-naam in de git-remote wordt vervangen door Zarlardinge-Home-Automation.
# v2 (16 okt 2026): stopt bij elke fout (Rosetta, git, SSH); Downloads pas
#   opgeruimd als alles gelukt is.
# ─────────────────────────────────────────────────────

REPO_ROOT="$HOME/Zarlardinge_Dashboards"
REPO="$REPO_ROOT/ESP32_Zarlar/zarlar-rpi"
DOWNLOADS="$HOME/Downloads"
BERICHT="${1:-update}"
RPI="fidel@100.123.74.113"     # Tailscale: werkt thuis en buitenshuis
PLUGIN="matterbridge-zarlar"

ROOT_FILES=("server.js" "update.sh" "deploy.sh")
PLUGIN_FILES=("module.js" "package.json" "matterbridge-zarlar.config.json")
NOOIT_PUBLIC=("worker-status.js" "module.js" "server.js")

stop() {
  echo "✗ $1"
  echo "  Downloads NIET opgeruimd — je bestanden staan er nog."
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  exit 1
}
in_lijst() { local x="$1"; shift; for y in "$@"; do [ "$x" = "$y" ] && return 0; done; return 1; }

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Zarlar deploy — $(date '+%d/%m/%Y %H:%M')"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# 0. Controles
[ "$(arch)" = "i386" ] && stop "Terminal draait via Rosetta (arch = i386). Finder → Hulpprogramma's → Terminal → Cmd-I → 'Open met Rosetta' uitvinken, Terminal herstarten."
cd "$REPO_ROOT" || stop "Repo niet gevonden: $REPO_ROOT"
git status >/dev/null 2>&1 || stop "git werkt niet op deze Mac (zie foutmelding hierboven)."
if git remote get-url origin | grep -qE 'Zarlardinge_Dashboards|ZarlarApp'; then
  git remote set-url origin https://github.com/FidelDworp/Zarlardinge-Home-Automation.git
  echo "→ git-remote bijgewerkt naar Zarlardinge-Home-Automation"
fi

# 1. Eerst de laatste versie van GitHub
echo "→ GitHub ophalen..."
git pull -q --rebase || stop "git pull mislukt (lokale wijzigingen of conflict?). Los op met: cd $REPO_ROOT && git status"

# 2. Gekende bestanden uit Downloads naar hun plaats
GEKOPIEERD=()
OVERGESLAGEN=()
for bestand in "$DOWNLOADS"/*; do
  [ -f "$bestand" ] || continue
  naam=$(basename "$bestand")
  ext="${naam##*.}"
  if [ "$naam" = "$PLUGIN.tar.gz" ]; then
    tar -xzf "$bestand" -C "$REPO" || stop "Uitpakken van $naam mislukt."
    echo "→ $naam → $PLUGIN/ (uitgepakt)"
  elif in_lijst "$naam" "${ROOT_FILES[@]}"; then
    cp "$bestand" "$REPO/$naam"; echo "→ $naam → zarlar-rpi/"
  elif in_lijst "$naam" "${PLUGIN_FILES[@]}"; then
    cp "$bestand" "$REPO/$PLUGIN/$naam"; echo "→ $naam → $PLUGIN/"
  elif { [ "$ext" = "html" ] || [ "$ext" = "css" ] || [ "$naam" = "zarlar.js" ]; } && ! in_lijst "$naam" "${NOOIT_PUBLIC[@]}"; then
    cp "$bestand" "$REPO/public/$naam"; echo "→ $naam → public/"
  else
    case "$ext" in md|ino|h|js|gs|bin) OVERGESLAGEN+=("$naam") ;; esac
    continue
  fi
  GEKOPIEERD+=("$bestand")
done

for naam in "${OVERGESLAGEN[@]}"; do
  echo "  (overgeslagen: $naam — geen RPi-bestand; documenten en sketches onderhoudt Claude op GitHub)"
done
if [ ${#GEKOPIEERD[@]} -eq 0 ]; then
  echo "⚠ Geen Zarlar-bestanden gevonden in Downloads."
  echo "  Alleen de RPi bijwerken met wat op GitHub staat:  ssh -t fidel@192.168.0.50 'bash ~/update.sh'"
  echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
  exit 0
fi

# 3. Commit en push
echo "→ git commit + push..."
git add -A ESP32_Zarlar/zarlar-rpi || stop "git add mislukt."
if git diff --cached --quiet; then
  echo "  (geen wijzigingen t.o.v. GitHub — commit overgeslagen)"
else
  git commit -q -m "$BERICHT" || stop "git commit mislukt."
  git push -q || stop "git push mislukt."
fi

# 4. RPi bijwerken
echo "→ RPi bijwerken..."
ssh -t -o ConnectTimeout=15 "$RPI" 'bash ~/update.sh' \
  || stop "RPi niet bereikbaar of update.sh faalde. GitHub is wel bijgewerkt: later 'ssh -t fidel@192.168.0.50 \"bash ~/update.sh\"' volstaat."

# 5. deploy.sh zelf bijwerken
if ! cmp -s "$REPO/deploy.sh" "$HOME/deploy.sh"; then
  # via een kopie + mv: het script dat nu draait, mag niet overschreven worden
  cp "$REPO/deploy.sh" "$HOME/deploy.sh.nieuw" && chmod +x "$HOME/deploy.sh.nieuw" && mv -f "$HOME/deploy.sh.nieuw" "$HOME/deploy.sh"
  echo "→ ~/deploy.sh bijgewerkt (geldt vanaf de volgende keer)"
fi

# 6. Downloads opruimen (alleen als alles gelukt is)
echo "→ Downloads opruimen..."
for bestand in "${GEKOPIEERD[@]}"; do
  rm "$bestand" && echo "  ✓ $(basename "$bestand")"
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo " Klaar!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
