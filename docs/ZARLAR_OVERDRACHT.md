# Overdracht — Zarlar (overkoepelend)

*Stand 7 november 2026 — Filip Delannoy (FiDel), Zarlardinge (BE)*

Dit document start een **nieuw gesprek met Claude** over het Zarlar-systeem als geheel: Dashboard, RPi-portaal, Apple Home-bridge, afspraken tussen de repo's. Elke controller-repo heeft daarnaast een eigen overdracht in `docs/` (zie §3). Wie het systeem wil leren kennen, begint bij [`README.md`](../README.md) en [`ARCHITECTUUR.md`](ARCHITECTUUR.md).

---

## 1. Werkwijze (geldt voor alle repo's)

- **Nederlands.** ESP32-C6 in de **Arduino IDE** (HVAC_WONING: klassieke ESP32).
- **Nooit een sketch of script maken zonder akkoord vooraf (GO).** Eerst een kort plan, dan code. **Eén stap per keer**, met feedback ertussen.
- **Eén stap per versie**, met **versiekop + changelog** in elke sketch, daarna een **48 u-test** (Sheet: geheugen en herstarts).
- **Alles via OTA**: updates en diagnose zonder seriële kabel.
- **`/json`-keys en bedieningsadressen niet wijzigen** zonder uitdrukkelijk akkoord: Dashboard, Portal, RPi-bridge en Google Sheets hangen ervan af.
- **Bij twijfel over IP's, namen of verschillen tussen documenten: eerst met FiDel bespreken.**
- **Terminal-commando's:** als FiDel iets op de Mac of de RPi moet uitvoeren, geeft Claude het commando kant-en-klaar om te plakken: in een codeblok, met erbij waar het moet (Terminal van de Mac; de RPi via `ssh` vanaf de Mac) en wat je ongeveer te zien krijgt. FiDel plakt de uitvoer terug als controle.
- **Docs bijwerken bij elke stap:** README, overdracht, en waar nodig het Master-document en het Portal Plan. Beknopt, geen achterhaalde info.

### GitHub

- **De repo's zijn de enige bron.** De documenten in het claude.ai-project "Room transitions" verwijzen alleen nog naar hier (7/11). **Werk in één gesprek tegelijk per repo**: twee gesprekken in dezelfde repo kunnen botsen.
- Claude koppelt de repo's van FidelDworp met **schrijfrechten** (`add_repo`, access `push`) en werkt docs, sketches en mappen **rechtstreeks** bij: één commit per stap, vóór het pushen `fetch` + `rebase` (FiDel commit soms zelf tussendoor).
- **Indeling van elke repo:** `README.md` in de root; **alle andere documenten in `docs/`**; sketchmap `<SketchNaam>/` (ino + `.h` + `partitions.csv`); `GoogleScript/`; `bin/` (laatste gecompileerde `.bin`); `old/`.
- **Overdracht per repo:** `docs/<NAAM>_OVERDRACHT.md`, bijgewerkt door Claude.
- **Na GO:** sketch in de repo → in de sessie compileren (§5) → `.bin` in `bin/` (vorige versie weg) → FiDel uploadt via `http://<ip>/update`.
- **Niet mogelijk voor Claude:** repo-beschrijving ("About") en andere instellingen, GitHub-releases, nieuwe repo's aanmaken. Dat doet FiDel.

### Lessen ESP32-C6 (11/11, uit ROOM v3.4.x — gelden voor alle sketches)

1. **Geen Adafruit-DHT-bibliotheek meer.** Die zet de interrupts uit en wacht per flank tot "160 000 lussen", zonder te stoppen bij een fout. Hapert de DHT22, dan staan de interrupts seconden uit; de C6 heeft één kern → **interrupt-watchdog-herstart** (bewezen op room80 10/11: "laatste stap: DHT22 lezen"; zelfde patroon bij ROOM v3.2 en de Sjalay). Gebruik de eigen uitlezing `dhtRead22()` uit ROOM v3.4.4 (max. 100 µs per flank, stoppen bij de eerste fout, interrupts hooguit ±5 ms uit). Dit voorkomt de herstarts; een vastgelopen sensor komt er niet door terug (daarvoor blijft een power cycle nodig).
2. **IO12/IO13 zijn de USB-datalijnen (D−/D+)** van de C6. Wie ze gebruikt (shield: IO13 = I2C SDA): compileren met **USB CDC On Boot = Disabled** en bij het opstarten de USB-functie uitschakelen (`releaseUsbPins()` uit ROOM v3.4.2). Serieel en flashen dan via de devkit-poort **"UART"**.
3. **NVS (20 KB) vol door oude Matter-resten** (`CHIP_KVS`, `esp_matter_kvs`, `chip-*`, `nimble_bond`): instellingen en diagnose worden dan stilletjes niet meer bewaard. NVS houdt altijd één pagina (126 vakjes) leeg, dus "134 vrij" kan al vol zijn. ROOM v3.4.5 toont NVS per naamruimte en heeft een knop om ongebruikte naamruimtes te wissen; neem dat over in sketches die vroeger Matter draaiden.
4. **Types (struct) bovenaan de sketch** zetten, vóór de eerste functie: de automatische prototypes van de Arduino IDE komen daar.

## 2. Het systeem in het kort

**Eén netwerk** achter de Telenet-router: woning en schuur zien elkaar rechtstreeks. Geen DHCP-reservering mogelijk: vaste IP's staan in de controller zelf. Bereik **.50–.84 is voor de schuur**.

| Controller | Adres | Repo | Versie (7/11) |
|---|---|---|---|
| RPi (portaal + Matter-bridge) | 192.168.0.50:3000 | deze repo, `ESP32_Zarlar/zarlar-rpi/` | server.js v2.12, matterbridge-zarlar v0.8 |
| Dashboard (matrix 16×16, logging) | 192.168.0.60 | deze repo, `ESP32_Zarlar/ESP32_C6_Zarlar_Dashboard_Matrix/` | v6.4 (rij 3 = buitenverlichting), in test |
| HVAC | 192.168.0.70 | `ESP32C6_HVAC` | v2.0 (testopstelling) |
| ECO-boiler | 192.168.0.71 | `ESP32C6_ECO-boiler` | v2.0 |
| S-OUTSIDE | .72 (gereserveerd) | — | alleen een idee |
| Smart Energy | 192.168.0.73 · `smart-energy.local` | `ESP32C6_Smart-Energy` | v1.30, 48 u-test loopt |
| ROOM (B&B … Zitplaats) | .75–.81 (room80 = Eetplaats) | `ESP32C6_ROOMS` | v3.3 op room80 (7/11) |
| ACCESS1/2/3 | .82/.83/.84 | `ESP32C6_ACCESS` | nog te ontwerpen; vandaag Photon R8-ACCESS |
| Photon-rooms | Particle Cloud + Cloudflare Worker | `PHOTON_ROOMS` | nog in productie, behalve de B&B |

**Dashboard-matrix:** rij 0 HVAC · 1 ECO · 2 S-ENERGY · 3 ACCESS (W·Z·N·O) · 4 scheiding · 5–11 rooms · 12–15 leeg.

**Kernregels:** controllers posten zelf niets (alleen het Dashboard logt, elke 5 min) · geen Matter op de ESP32's (alleen de RPi is bridge naar Apple Home, woning "Zarlardinge") · van buitenhuis via Tailscale naar de RPi.

**Sheets:** R-ROOMS Log `1UiaexdpgpL3XjPWHNdMEbJ18_H9yp1w_VN8VBdVCpY0` · S-ECO `1nn_GBmLhS9zZhp5On_3r-LRLonetSjnHCuHZiNOc4Q0` · S-HVAC `1LL7swNRIVl2qfREHxAU7nCq9fH9zTUxa37s0IduHTXY`. Lezen via de Google Sheets-koppeling (`get_values`); uptime staat met punten als duizendtal-scheiding.

## 3. Repo's en hun overdracht

| Repo | Overdracht |
|---|---|
| `Zarlardinge-Home-Automation` | dit document |
| `ESP32C6_ROOMS` | `docs/ROOMS_OVERDRACHT.md` |
| `ESP32C6_HVAC` | `docs/HVAC_OVERDRACHT.md` |
| `ESP32C6_ECO-boiler` | `docs/ECO_OVERDRACHT.md` |
| `ESP32C6_Smart-Energy` | `docs/ENERGY_OVERDRACHT.md` |
| `ESP32C6_ACCESS` | `docs/ACCESS_OVERDRACHT.md` · standaard: `docs/ACCESS_STANDAARD.md` |
| `SJALAY-YSERMONDE` | `docs/SJALAY_OVERDRACHT.md` |
| `HVAC_WONING` | `docs/HVAC_WONING_OVERDRACHT.md` |
| `PHOTON_ROOMS` | `docs/PHOTON_OVERDRACHT.md` |
| `ESP32C6_Experiments` | `docs/EXPERIMENTS_OVERDRACHT.md` |

## 4. Open punten (deze repo)

- **Dashboard v6.4:** 48 u-test (heap) en rij 3 (buitenverlichting) in de praktijk bekijken.
- **Dashboard:** HOME/UIT-tegel via de RPi (`/home_json`, `/set_home_global`), nieuwe matrixlegende.
- **Master-document:** §4–§8 bijwerken na de 48 u-tests (o.a. Dashboard v6.1–v6.4, ROOM v3.3).
- **Master §17 Optimalisatieplan:** opruimversies + mDNS op alle controllers, statuspagina op de RPi, ROOM-uitrol.
- **Particle-token** staat leesbaar in `ESP32_Zarlar/worker-status.js` (publieke repo): bewust zo gelaten, de Photons verdwijnen.

## 5. Praktisch

Uitleg voor iedereen over deployen (ESP32 en RPi): [`DEPLOY.md`](DEPLOY.md).


- **Compileren (FiDel):** board ESP32C6 Dev Module, **Flash 16 MB**, Partition Custom (`partitions.csv` in de sketchmap), USB CDC On Boot Enabled. JavaScript altijd in een `.h`-bestand.
- **OTA:** enkel `<sketch>.ino.bin` uploaden via `http://<ip>/update` (ook het Dashboard, sinds v6.1). De partitietabel kan niet via OTA veranderen.
- **Compileren in de Claude-sessie** (werkt sinds 7/11): `arduino-cli` 1.3.1 (GitHub-release); index `package_esp32_index.json` van de GitHub-release `espressif/arduino-esp32` 3.3.12, ingekort tot `esp-rv32`, `esp-x32`, `esptool_py`, `esp32c6-libs`, `esp32-libs` en als `file://`-index gebruikt (espressif.github.io en downloads.arduino.cc zijn niet bereikbaar); bibliotheken via `git clone` (ESP32Async/ESPAsyncWebServer + AsyncTCP, bblanchon/ArduinoJson, adafruit DHT + Adafruit_Sensor + NeoPixel + MCP23017 + BusIO + MAX31865, pstolarz/OneWireNg). arduino-cli herkent universal-ctags niet: `runtime.tools.ctags.path` naar een lege `ctags` en de prototypes zelf genereren met universal-ctags in een tijdelijke kopie van de sketch (nooit in de repo). FQBN ESP32-C6: `esp32:esp32:esp32c6:FlashSize=16M,PartitionScheme=custom,CDCOnBoot=cdc` (ROOM sinds v3.4.2: `CDCOnBoot=default`, zie lessen hierboven); klassieke ESP32: `esp32:esp32:esp32`. **Altijd `--output-dir` buiten de repo** (bv. `/tmp/build_<naam>`): arduino-cli maakt anders een map `build/` (±55 MB) in de sketchmap, die per vergissing mee gecommit werd (Sjalay 9–11/11, ACCESS 11/11). Elke repo krijgt een `.gitignore` met `build/`, `*.elf`, `*.map`.
- **RPi:** `ssh -t fidel@192.168.0.50 'bash ~/update.sh'` zet portaal, webpagina's én Apple Home-plugin gelijk met GitHub (v2, 7/11). Eigen bestanden vanaf de Mac: `bash ~/deploy.sh "omschrijving"` (v3: alleen gekende bestanden uit `~/Downloads`). Details: [`DEPLOY.md`](DEPLOY.md).
- **Google Apps Script:** "Implementaties beheren → potlood → nieuwe versie", nooit "nieuwe implementatie". Na kolomwijzigingen `setupHeaders()` één keer uitvoeren.
