# Deployen: nieuwe software op een controller of op de RPi zetten

*Bijgewerkt 7 november 2026 — Filip Delannoy (FiDel)*

"Deployen" betekent: een nieuwe versie van een programma op het toestel zetten waar het moet draaien. In Zarlar zijn er twee soorten toestellen:

- de **ESP32-controllers** (ROOM, HVAC, ECO, Smart Energy, Dashboard, Sjalay, ventilatie Maarten): daar draait een **sketch**;
- de **Raspberry Pi** (RPi, 192.168.0.50): daar draaien het **portaal** (`server.js` + webpagina's) en de **Apple Home-bridge** (Matterbridge met de plugin `matterbridge-zarlar`).

**Waar staat de juiste versie?** Altijd op GitHub. Claude houdt de sketches, de `.bin`-bestanden en alle documenten daar rechtstreeks bij. Wat op GitHub staat, is de waarheid.

---

## 1. ESP32-controllers

### Overzicht

| | Manier | Wanneer | Nodig |
|---|---|---|---|
| **A** | **`.bin` uit de repo uploaden** | gewone update, de snelste manier | alleen een browser |
| **B** | Zelf compileren en uploaden via `/update` | als je zelf iets aan de sketch veranderde | Arduino IDE |
| **C** | Via de USB-kabel | de allereerste keer, of als de controller niet meer opstart | Arduino IDE + kabel |

**OTA** (Over The Air) wil zeggen: via WiFi, zonder kabel. A en B zijn OTA.

### A. Een `.bin` uit de repo uploaden (aangeraden)

Elke controller-repo heeft een map **`bin/`** met de laatste versie, al gecompileerd door Claude, bv. `bin/ESP32_C6_ROOM_v3.3.ino.bin`.

1. Open de repo op GitHub, ga naar `bin/` en klik op het bestand.
2. Klik op **Download raw file** (pijltje naar beneden, rechtsboven). Op een gsm of tablet kan het ook.
3. Open in de browser de updatepagina van de controller: `http://<adres>/update`, bv. `http://192.168.0.80/update`.
4. Kies het gedownloade bestand en klik op uploaden. De controller zet de nieuwe versie in zijn tweede geheugenhelft en herstart vanzelf.
5. Controleer de versie (onderaan de pagina of in `/settings`).

Mislukt de update, dan blijft de oude versie gewoon draaien.

### B. Zelf compileren en via `/update` uploaden

1. Haal de sketchmap van GitHub (zie §3) en open de **map** in de Arduino IDE (bv. `ESP32_C6_ROOM/`).
2. Instellingen (*Tools*):
   - **ESP32-C6:** board *ESP32C6 Dev Module*, Flash Size **16 MB**, Partition Scheme *Custom* (`partitions.csv` naast de sketch wordt automatisch gebruikt), USB CDC On Boot *Enabled*.
   - **Klassieke ESP32** (ventilatie Maarten): board *ESP32 Dev Module*, partities *Default 4MB with spiffs*.
3. *Sketch → Export Compiled Binary*.
4. In de map `build/…` staan meerdere `.bin`-bestanden. Kies **enkel `<sketch>.ino.bin`**, niet `.bootloader.bin`, `.partitions.bin` of `.merged.bin`.
5. Upload dat bestand zoals bij A.

### C. Via de USB-kabel

1. Controller met een USB-kabel aan de laptop, sketchmap open in de Arduino IDE, instellingen zoals bij B.
2. Kies de juiste poort (*Tools → Port*) en klik op **Upload** (pijl naar rechts).
3. Nodig voor een gloednieuwe controller, of als de partitietabel verandert: die kan niet via OTA.

### Adressen

| Controller | Updatepagina |
|---|---|
| Dashboard | `http://192.168.0.60/update` |
| HVAC | `http://192.168.0.70/update` |
| ECO-boiler | `http://192.168.0.71/update` |
| Smart Energy | `http://192.168.0.73/update` |
| ROOM Eetplaats (room80) | `http://192.168.0.80/update` |
| Ventilatie Maarten | `http://pwmtest.local/update` |
| Sjalay | via Tailscale (zie de Sjalay-README) |

---

## 2. Raspberry Pi

### Overzicht

| | Manier | Wanneer |
|---|---|---|
| **1** | **`update.sh` op de RPi** | Claude heeft iets aangepast op GitHub (portaal, webpagina's). Dit is nu de gewone weg |
| **2** | `deploy.sh` op de Mac | je hebt zelf een bestand aangepast en in Downloads gezet |
| **3** | Plugin-procedure | er is een nieuwe versie van de Apple Home-plugin `matterbridge-zarlar` |

### 1. `update.sh`: de RPi haalt de laatste versie van GitHub

In de Terminal van de Mac (thuis of via Tailscale):

```
ssh fidel@192.168.0.50 'bash ~/update.sh'
```

Wat het doet: de RPi haalt de repo op, kopieert de webpagina's (`public/`) naar het portaal en herstart de server als `server.js` veranderde. Van buitenhuis: vervang `192.168.0.50` door het Tailscale-adres van de RPi.

### 2. `deploy.sh`: eigen bestanden vanaf de Mac

Alleen als je **zelf** een bestand aanpaste (bv. een webpagina).

1. Zet het bestand in **Downloads** (`server.js`, `.html`, `.css`, `.js`).
2. In de Terminal: `bash ~/deploy.sh "korte omschrijving"`.
3. Het script zet het bestand op de juiste plaats in de repo, maakt een commit, haalt eerst de laatste versie van GitHub (`git pull --rebase`), pusht, en laat de RPi `update.sh` uitvoeren. Downloads wordt pas opgeruimd als alles gelukt is.

⚠️ **Geen documenten (`.md`) en geen sketches via `deploy.sh`.** De documenten staan in `docs/` en Claude houdt ze op GitHub bij. Een `.md`-bestand in Downloads komt anders als dubbele kopie in `zarlar-rpi/` terecht.

### 3. De Apple Home-plugin

De plugin staat in `ESP32_Zarlar/zarlar-rpi/matterbridge-zarlar/` en draait in de Docker-container van Matterbridge op de RPi. De procedure (tar.gz kopiëren, uitpakken, container herstarten, logboek nakijken) staat in het [Portal Plan](Zarlar_Portal_Plan.md) §16.10.

---

## 3. Wie doet wat?

| Wat | Wie |
|---|---|
| Sketch aanpassen, compileren, `.bin` in `bin/`, documenten bijwerken | Claude, op GitHub, na het akkoord van FiDel |
| `.bin` op de controller zetten (A) | FiDel |
| `update.sh` op de RPi uitvoeren | FiDel |
| 48 uur testen en de Sheet bekijken | FiDel en Claude samen |

Je hoeft dus niets meer naar GitHub te kopiëren: alles staat er al.
