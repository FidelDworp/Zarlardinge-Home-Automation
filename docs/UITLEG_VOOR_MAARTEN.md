# Thuisautomatisering in Zarlardinge — uitleg voor Maarten

*Voor Maarten, van papa (FiDel) — 6 november 2026*

Je hebt de ventilatietest gezien: een klein bordje met wifi, een schuifbalk op je gsm, en de ventilatie draait sneller of trager. Dat is precies hoe het hele systeem in de schuur werkt, alleen met meer van die bordjes. Dit document legt uit hoe het in elkaar zit en hoe je zelf kunt beginnen. Je hoeft niets te kennen om te starten.

---

## 1. De bouwstenen

### Het computertje: de ESP32

Een **ESP32** is een computertje zo groot als een luciferdoosje, met wifi, voor een paar euro. Het heeft **pinnen** waarop je sensoren en schakelaars aansluit. Je programmeert het op je laptop en zet het programma erop via een USB-kabel. Daarna draait het programma altijd, ook na een stroomonderbreking.

In de schuur gebruiken we de **ESP32-C6**. Je ventilatiebordje is een iets ouder model (de "klassieke" ESP32), maar het idee is hetzelfde.

### Sensoren: meten

| Sensor | Meet | Waar |
|---|---|---|
| DS18B20 | Temperatuur (heel nauwkeurig, meerdere op één draad) | Kamers, leidingen, boiler |
| DHT22 | Temperatuur en luchtvochtigheid | Kamers |
| SCD41 | CO2 (hoe "muf" de lucht is) | Slaapkamers, zithoek |
| PIR | Beweging | Kamers, buiten |
| LDR | Licht (is het dag of nacht?) | Kamers |

### Actuatoren: doen

| Wat | Hoe |
|---|---|
| Verlichting | Adresseerbare leds, of **Powerpixels** (eigen uitvinding: een led-chip die zware lampen schakelt) |
| Ventilatie | Een **PWM**-signaal, omgezet naar 0–10 V (zoals bij jouw ventilatie) |
| Pompen, kleppen | Relais |

### Het programma: een "sketch"

Een programma voor een ESP32 heet een **sketch**. Het is geschreven in C++ en je maakt het in de gratis **Arduino IDE**. Een sketch doet bijna altijd hetzelfde, telkens opnieuw:

1. sensoren lezen,
2. beslissen (is het te koud? is er beweging?),
3. iets aansturen (verwarming aan, licht aan),
4. de toestand tonen op een webpagina.

## 2. Hoe alles samenwerkt

In de schuur heeft **elke installatie een eigen ESP32**: de verwarming (HVAC), de zonneboiler (ECO), de energiemeter (Smart Energy) en elke kamer (ROOM). Elk heeft een vast adres in het netwerk, bv. `192.168.0.71` voor de zonneboiler.

```
  [ kamer ]  [ boiler ]  [ verwarming ]  [ energie ]     ← elk een ESP32
       \          |            |             /
        \         |            |            /      ze vragen allemaal "/json" op
         ▼        ▼            ▼           ▼
   [ Dashboard ESP32 ]            [ Raspberry Pi ]
    lichtjesbord + logt            mooie webpagina's
    naar Google Sheets             + Apple Home op je gsm
```

- **`/json`** is een webadres op elke controller met al zijn meetwaarden in een kort tekstformaat. Typ in de schuur maar eens `http://192.168.0.71/json` in je browser: je ziet de gegevens van de zonneboiler.
- **Het Dashboard** is een ESP32 met een bord van 16×16 lampjes. Elke rij is een controller, elke kleur een toestand. Het schrijft ook elke 5 minuten alles weg in een Google Sheet, zodat we grafieken kunnen maken en fouten kunnen terugvinden.
- **De Raspberry Pi** is een iets grotere computer. Hij toont alles op mooie pagina's en zorgt dat de toestellen in de **Apple Woning-app** op de gsm verschijnen.

Een paar slimme keuzes die we onderweg leerden:
- **Elke controller werkt ook alleen.** Valt het netwerk weg, dan blijft de verwarming gewoon regelen.
- **Controllers sturen zelf niets naar het internet.** Dat vraagt te veel geheugen. Het Dashboard doet het voor hen.
- **Eén sketch voor alle kamers.** Wat verschilt per kamer (naam, aantal lampen, welke sensoren), stel je in op een webpagina. Zo moet er maar één programma onderhouden worden.

## 3. Jouw ventilatiecontroller als voorbeeld

Je ventilatie (Begetube 350 R) heeft een ingang van **0 tot 10 V**: hoe hoger de spanning, hoe harder ze draait.

```
 [ gsm ] ─ wifi ─> [ ESP32 ] ─ PWM ─> [ AnaBoX ] ─ 0–10 V ─> [ ventilatie ]
```

1. **De ESP32** maakt een webpagina met een schuifbalk (`http://pwmtest.local`).
2. **PWM** betekent dat de ESP32 zijn uitgang duizenden keren per seconde aan en uit zet. Hoe langer "aan", hoe hoger de stand.
3. **De AnaBoX** maakt daar een rustige spanning van 0–10 V van.

Een belangrijke eigenaardigheid: **0 V betekent bij deze unit volle kracht.** Daarom start de sketch meteen in een veilige stand en gaat hij nooit onder 0,3 V.

Alles over deze controller (aansluitingen, pagina's, metingen) staat in de repo [`HVAC_WONING`](https://github.com/FidelDworp/HVAC_WONING).

## 4. Zelf beginnen: een leerpad

Neem gerust je tijd per stap. Elke stap werkt op zich.

**Stap 1 — Spelen met wat er al is**
- Open `http://pwmtest.local` en probeer de knoppen en de schuifbalk. Bekijk `/settings`: daar zie je de firmwareversie, het vrije geheugen en waarom hij laatst herstartte.

**Stap 2 — De Arduino IDE installeren**
- Download de **Arduino IDE 2** (gratis, arduino.cc).
- Voeg de ESP32-borden toe: *Instellingen → Additional boards manager URLs* met het adres van Espressif, daarna in de *Boards Manager* "esp32" installeren.
- Board kiezen: "ESP32 Dev Module" voor jouw bordje, "ESP32C6 Dev Module" voor de bordjes uit de schuur.

**Stap 3 — Een eerste eigen sketch**
- Begin met het voorbeeld *Blink* (een led laten knipperen): *Bestand → Voorbeelden → Basics → Blink*.
- Daarna een sensor lezen. In de repo [`ESP32C6_Experiments`](https://github.com/FidelDworp/ESP32C6_Experiments) staan kleine testsketches, bv. een temperatuursensor (DS18B20) uitlezen.
- Open de **Serial Monitor** (115200 baud) om te zien wat de ESP32 vertelt.

**Stap 4 — Een sketch van GitHub openen**
- Op GitHub: groene knop **Code → Download ZIP**, uitpakken.
- Open de **map** met dezelfde naam als het `.ino`-bestand (bv. `ESP32_PWM_Test/`). Alles wat de sketch nodig heeft, staat in die map.

**Stap 5 — Iets kleins aanpassen en draadloos uploaden (OTA)**
- Verander bv. een tekst op de webpagina van je ventilatiesketch.
- *Sketch → Export Compiled Binary*, dan het bestand `….ino.bin` (niet de andere `.bin`-bestanden) uploaden via de pagina `/update` van de controller. Geen kabel nodig.

**Stap 6 — Verder kijken**
- Hoe ziet de `/json` van een controller in de schuur eruit? Wat betekenen de sleutels? (Zie de README van die controller.)
- **Home Assistant** op een Raspberry Pi kan al je slimme toestellen samenbrengen; je ESP32 kan daar later aan gekoppeld worden.

## 5. Veiligheid

- **Werk alleen met lage spanning** (5 V, 10 V, 12 V). Aan **230 V** komen we niet: dat blijft binnen de toestellen of gaat via gekeurde relais en een elektricien.
- **Een ESP32-pin verdraagt maximaal 3,3 V.** Een sensor op 5 V kan het bordje kapotmaken. Meet eerst met de multimeter.
- **Dubbelcheck GND (min) en plus** voor je de voeding aansluit.
- Twijfel je? Vraag het, of vraag het aan Claude: die kent dit hele systeem.

## 6. Woordenlijst

| Woord | Betekenis |
|---|---|
| **ESP32** | Klein computertje met wifi |
| **Controller** | Een ESP32 met zijn programma, die één installatie of kamer regelt |
| **Sketch** | Een programma voor de ESP32 (`.ino`-bestand) |
| **Arduino IDE** | Het programma op je laptop waarmee je sketches schrijft en uploadt |
| **Pin / GPIO** | Een aansluiting op de ESP32 (in of uit) |
| **PWM** | Snel aan/uit schakelen om "half aan" te maken |
| **OTA** | Over The Air: een nieuwe sketch uploaden via wifi |
| **`/json`** | Webadres van een controller met al zijn meetwaarden |
| **IP-adres** | Het "huisnummer" van een toestel in het netwerk (bv. 192.168.0.71) |
| **mDNS / `.local`** | Een naam in plaats van een nummer (bv. `pwmtest.local`) |
| **NVS** | Geheugen in de ESP32 dat instellingen bewaart na een herstart |
| **Shield** | De print waar de ESP32 op steekt, met connectoren en voeding |
| **Repo (GitHub)** | Een online map met alle bestanden en de geschiedenis van een project |
| **Matter / Apple Home** | De standaard waarmee toestellen in de Woning-app van Apple verschijnen |

## 7. Waar vind je wat?

| Wat | Waar |
|---|---|
| Overzicht van alles | [`Zarlardinge-Home-Automation`](https://github.com/FidelDworp/Zarlardinge-Home-Automation) (deze repo) |
| Hoe het geheel in elkaar zit | [`docs/ARCHITECTUUR.md`](ARCHITECTUUR.md) |
| Jouw ventilatie | [`HVAC_WONING`](https://github.com/FidelDworp/HVAC_WONING) |
| Smart Energy in de woning | [`ESP32C6_Smart-Energy`](https://github.com/FidelDworp/ESP32C6_Smart-Energy), document `docs/Smart_Energy_Zarlardinge_Maarten.md` |
| Testsketches om mee te oefenen | [`ESP32C6_Experiments`](https://github.com/FidelDworp/ESP32C6_Experiments) |
| Alle technische details | [Master-document](../ESP32_Zarlar/Zarlar_Master_Overnamedocument.md) |

Veel plezier ermee!
