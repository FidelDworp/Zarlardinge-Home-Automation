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
| **1** | **`update.sh`**: de RPi haalt alles van GitHub | Claude heeft iets aangepast (portaal, webpagina's, Apple Home-plugin). **Dit is de gewone weg** |
| **2** | **`deploy.sh`** op de Mac | je hebt **zelf** een bestand aangepast en in Downloads gezet |

Eén commando doet telkens alles: portaal, webpagina's én de Apple Home-plugin.

### 1. `update.sh`: de RPi gelijkzetten met GitHub

In de Terminal van de Mac:

```
ssh -t fidel@192.168.0.50 'bash ~/update.sh'
```

Van buitenhuis (via Tailscale): `ssh -t fidel@100.123.74.113 'bash ~/update.sh'`.

Wat het doet:
1. haalt de laatste versie van GitHub op en toont welke wijzigingen erbij kwamen;
2. kopieert de webpagina's (`public/`) naar het portaal;
3. **`server.js` anders dan wat draait?** → kopiëren en het portaal herstarten;
4. **Apple Home-plugin `matterbridge-zarlar` anders?** → kopiëren en Matterbridge herstarten (40 s wachten, daarna een stukje logboek). Blijven nieuwe tegels weg: Keuken Geel Bolleke herstarten;
5. werkt zichzelf bij als er een nieuwe `update.sh` op GitHub staat.

Je mag het zo vaak uitvoeren als je wilt: als er niets veranderd is, gebeurt er niets. `-t` zorgt ervoor dat de RPi zo nodig om je wachtwoord kan vragen (voor `sudo`).

### 2. `deploy.sh`: eigen bestanden vanaf de Mac

Alleen als je **zelf** iets aanpaste.

1. Zet het bestand in **Downloads**. Het script herkent:
   - `server.js`, `update.sh`, `deploy.sh` → `zarlar-rpi/`
   - `.html`, `.css` en `zarlar.js` → `public/`
   - `module.js`, `package.json`, `matterbridge-zarlar.config.json` of `matterbridge-zarlar.tar.gz` → de Apple Home-plugin
2. In de Terminal: `bash ~/deploy.sh "korte omschrijving"`.
3. Het script haalt eerst de laatste versie van GitHub, zet je bestanden op hun plaats, maakt een commit, pusht en voert daarna `update.sh` uit op de RPi. Downloads wordt pas opgeruimd als alles gelukt is.

Andere bestanden (documenten `.md`, sketches, foto's…) laat het script gewoon in Downloads staan: documenten en sketches houdt Claude op GitHub bij.

### Eenmalig: de nieuwe scripts in gebruik nemen ✅ gedaan op 7/11/2026

Sindsdien werken `update.sh` v2 en `deploy.sh` v3 zichzelf bij. Ter info, zo gebeurde het (alleen opnieuw nodig op een nieuwe Mac of RPi):

- **RPi:** `ssh -t fidel@192.168.0.50 'bash ~/update.sh; cp ~/repo/ESP32_Zarlar/zarlar-rpi/update.sh ~/update.sh && bash ~/update.sh'`
  (de tweede uitvoering herstart Matterbridge één keer, omdat de plugin op GitHub iets anders is dan op de RPi)
- **Mac:** `cd ~/Zarlardinge_Dashboards && git pull --rebase && cp ESP32_Zarlar/zarlar-rpi/deploy.sh ~/deploy.sh`

## 3. Wie doet wat?

| Wat | Wie |
|---|---|
| Sketch aanpassen, compileren, `.bin` in `bin/`, documenten bijwerken | Claude, op GitHub, na het akkoord van FiDel |
| `.bin` op de controller zetten (A) | FiDel |
| `ssh -t fidel@192.168.0.50 'bash ~/update.sh'` na een wijziging van Claude aan het portaal of de plugin | FiDel |
| 48 uur testen en de Sheet bekijken | FiDel en Claude samen |

Je hoeft dus niets meer naar GitHub te kopiëren: alles staat er al.
