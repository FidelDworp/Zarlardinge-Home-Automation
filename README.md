# Zarlardinge Home Automation

Zelfgebouwde thuisautomatisering van de schuur en de woning in **Zarlardinge** (België): verwarming, ventilatie, zonneboiler, energie, verlichting en sensoren per kamer. Elke controller is een **ESP32-C6** met een eigen webpagina. Een centraal **Dashboard** verzamelt alles en logt naar Google Sheets, een **Raspberry Pi** brengt alles samen in één portaal en in **Apple Home**.

Dit is de **overkoepelende repo**: hier begint wie het geheel wil begrijpen. Elke controller heeft daarnaast een eigen repo.

*Filip Delannoy (FiDel) — bijgewerkt 6 november 2026*

## Waar begin ik?

| Ik wil… | Lees |
|---|---|
| Het geheel begrijpen in 10 minuten | [`docs/ARCHITECTUUR.md`](docs/ARCHITECTUUR.md) |
| Leren hoe het werkt en zelf beginnen (voor Maarten) | [`docs/UITLEG_VOOR_MAARTEN.md`](docs/UITLEG_VOOR_MAARTEN.md) |
| Alle technische details (pinnen, `/json`, regels, versies) | [`ESP32_Zarlar/Zarlar_Master_Overnamedocument.md`](ESP32_Zarlar/Zarlar_Master_Overnamedocument.md) |
| De Raspberry Pi: portaal, Apple Home, energie | [`ESP32_Zarlar/zarlar-rpi/Zarlar_Portal_Plan.md`](ESP32_Zarlar/zarlar-rpi/Zarlar_Portal_Plan.md) |

## Alle controllers

| Controller | Adres | Repo | Versie | Status |
|---|---|---|---|---|
| **Dashboard** (statusmatrix, logging) | 192.168.0.60 | deze repo, [`ESP32_Zarlar/`](ESP32_Zarlar/) | v6.3 | ✅ productie |
| **RPi-portaal** + Apple Home-bridge | 192.168.0.50:3000 | deze repo, [`ESP32_Zarlar/zarlar-rpi/`](ESP32_Zarlar/zarlar-rpi/) | server.js v2.12, plugin v0.8 | ✅ productie |
| **HVAC** (verwarmingskringen, ventilatie) | 192.168.0.70 | [`ESP32C6_HVAC`](https://github.com/FidelDworp/ESP32C6_HVAC) | v2.0 | 🧪 test; de Photon S-HVAC stuurt nog de echte verwarming |
| **ECO-boiler** (zonneboiler 490 L) | 192.168.0.71 | [`ESP32C6_ECO-boiler`](https://github.com/FidelDworp/ESP32C6_ECO-boiler) | v2.0 | ✅ productie |
| **Smart Energy** (zon, net, EPEX, Tesla) | 192.168.0.73 | [`ESP32C6_Smart-Energy`](https://github.com/FidelDworp/ESP32C6_Smart-Energy) | v1.30 (48 u-test loopt) | ✅ productie |
| **ROOM** (één sketch voor alle kamers) | 192.168.0.75–81 | [`ESP32C6_ROOMS`](https://github.com/FidelDworp/ESP32C6_ROOMS) | v3.2 | 🔄 room80 (eetplaats) draait, uitrol per kamer volgt |
| **Photon-rooms** R1–R8 (oud) | via Particle Cloud | [`PHOTON_ROOMS`](https://github.com/FidelDworp/PHOTON_ROOMS) | — | ⏳ nog in productie tot de overstap |
| **ACCESS1–3** (buitenverlichting, horen samen) | 192.168.0.82–84 (gepland) | nog geen repo | — | 📋 gepland |
| **Tesla-proxy** (BLE naar de Tesla, via Smart Energy) | 192.168.0.134 | [`ESP32C6_Smart-Energy`](https://github.com/FidelDworp/ESP32C6_Smart-Energy) (`Tesla/`) | — | ✅ actief |
| *S-OUTSIDE* (idee: buitenmetingen centraliseren) | 192.168.0.72 (gereserveerd) | — | — | 💡 idee, niet dringend |

**Verwante projecten**

| Project | Repo |
|---|---|
| Ventilatie woning Maarten (PWM → 0–10 V) | [`HVAC_WONING`](https://github.com/FidelDworp/HVAC_WONING) |
| Vakantiewoning Sjalay (Recht): verwarming, boiler, licht | [`SJALAY-YSERMONDE`](https://github.com/FidelDworp/SJALAY-YSERMONDE) |
| Testsketches en proeven | [`ESP32C6_Experiments`](https://github.com/FidelDworp/ESP32C6_Experiments) |
| 3D-ontwerpen (kapjes voor shield en RoomSense) | [`3D-design`](https://github.com/FidelDworp/3D-design) |

## Wat staat er in deze repo?

```
README.md                    dit overzicht
docs/
  ARCHITECTUUR.md            hoe het geheel in elkaar zit
  UITLEG_VOOR_MAARTEN.md     uitleg en eerste stappen voor beginners
ESP32_Zarlar/
  Zarlar_Master_Overnamedocument.md   de technische referentie van het hele systeem
  ESP32_C6_Zarlar_Dashboard_Matrix/   sketch van het Dashboard (+ partitions.csv)
  zarlar-rpi/                RPi-portaal: server.js, webpagina's, Apple Home-plugin, deploy-scripts
  worker-status.js           Cloudflare Worker (Photon-gegevens zonder token in de browser)
*.html, BG.jpg               de oude Photon-webapp (GitHub Pages), wordt nog gebruikt
old/                         vorige versies, alleen ter referentie
```

**Over de HTML-pagina's in de root:** dat is de webapp uit de Photon-tijd, gepubliceerd via GitHub Pages. Ze blijven staan zolang er Photons draaien (het RPi-portaal verwijst bijvoorbeeld naar `S-OUTSIDE.html`). Het Particle-token zit niet in deze bestanden maar in een Cloudflare Worker.

## Afspraken voor alle repo's

- **Eén repo per type controller**, met een `README.md`: wat, status, hardware, `/json`, bestanden, compileren, versies.
- **Vaste mappen:** `<SketchNaam>/` met `<SketchNaam>.ino` en `partitions.csv` ernaast (de Arduino IDE opent de map rechtstreeks), `GoogleScript/`, `docs/`, `old/`.
- **Versiekop en changelog** bovenaan elke sketch; één stap per versie, daarna 48 uur testen.
- **`/json` is een contract:** keys en bedieningsadressen wijzigen niet zonder afspraak, want het Dashboard, het portaal, Apple Home en de Sheets hangen ervan af.
- **Taal:** Nederlands.
