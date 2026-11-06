# Thuisautomatisering in Zarlardinge — uitleg voor Maarten

*Voor Maarten, van papa (FiDel) — 6 november 2026 (bijgewerkt: Google Home, Tailscale, waarom nog geen Home Assistant)*

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
- **De Raspberry Pi** is een iets grotere computer. Hij toont alles op mooie pagina's en zorgt via de **Matter-bridge** dat de toestellen in de **Apple Woning-app** op papa's gsm verschijnen. Dezelfde bridge kan ze ook in **Google Home** zetten (zie §3).

Een paar slimme keuzes die we onderweg leerden:
- **Elke controller werkt ook alleen.** Valt het netwerk weg, dan blijft de verwarming gewoon regelen.
- **Controllers sturen zelf niets naar het internet.** Dat vraagt te veel geheugen. Het Dashboard doet het voor hen.
- **Eén sketch voor alle kamers.** Wat verschilt per kamer (naam, aantal lampen, welke sensoren), stel je in op een webpagina. Zo moet er maar één programma onderhouden worden.

## 3. Alles bedienen vanuit Google Home

Papa bedient de schuur vanuit **Apple Home**. Jij wilt hetzelfde, maar dan in **Google Home**. Daarvoor is alles al in huis.

### De Matter-bridge op de Raspberry Pi

**Matter** is een standaard die Apple, Google, Amazon en Samsung samen afspraken. Een toestel dat "Matter spreekt", werkt met al die apps.

Onze ESP32's spreken zelf geen Matter (dat kostte te veel geheugen). Daarom draait op de Raspberry Pi 3B+ in de schuur **Matterbridge**: een programma dat de controllers "vertaalt" naar Matter. Voor Apple Home lijken het dan gewone slimme toestellen.

Het mooie aan Matter: **één bridge mag aan meerdere apps tegelijk gekoppeld zijn** (dat heet *multi-admin*). Dezelfde bridge die nu in papa's Apple Home zit, kan dus ook in **jouw Google Home**. Er hoeft niets dubbel gebouwd te worden.

```
 [ controllers ] ─ /json ─> [ RPi + Matterbridge ] ─ Matter ─┬─> Apple Home (papa)
                                                              └─> Google Home (jij)
```

Wat je nodig hebt:
- De **Google Home-app** op je gsm.
- Een **Google-hub** in huis die Matter ondersteunt, bv. een **Nest Mini (2e generatie)**, een **Nest Hub** of een **Google TV Streamer**. Google laat Matter-toestellen alleen via zo'n hub werken. Heb je er al een, dan kost dit niets extra.
- Een koppelcode: papa maakt die aan vanuit de bridge, jij voegt hem toe in Google Home met *Toestel toevoegen → Matter*.

Goed om te weten: Google Home toont niet elk Matter-type even mooi als Apple Home (een thermostaat of lamp wel, uitzonderlijke dingen soms niet). Dat testen we samen uit.

### Jouw keuze: onze bridge of een eigen RPi

Een bridge leest alleen de `/json` van de controllers uit en vertaalt die naar Matter. Twee bridges mogen dezelfde controller dus gewoon tegelijk uitlezen. Daardoor kun je kiezen, en later wisselen zonder iets aan je controllers te veranderen.

| | **A. De bridge in de schuur gebruiken** | **B. Een eigen RPi 3B+ met Matterbridge** |
|---|---|---|
| Starten | Meteen: papa geeft een koppelcode, klaar | Matterbridge installeren + een eigen plugin voor jouw controllers (een uitgeklede kopie van die van papa) |
| Wat zie je | Wat in papa's bridge zit (eventueel alleen jouw deel, als aparte "child bridge") | Alleen jouw eigen toestellen |
| Afhankelijkheid | Herstart of storing van papa's RPi = ook bij jou even weg | Volledig onafhankelijk |
| Onderhoud | Papa | Jij (met hulp) |
| Kost | Niets | Een RPi 3B+, goedkoop en ruim genoeg: papa's bridge draait er ook op |

**Voorstel:** begin met A, dan werkt het meteen en zie je hoe Matter werkt. Stap over naar B zodra je meer eigen toestellen hebt of zelf wilt leren sleutelen. In beide gevallen heb je een Google-hub nodig, en je eigen RPi kan ook in papa's Tailscale-netwerk.

### Van buitenaf: Tailscale

**Tailscale** is een soort privé-tunnel over het internet. Alle toestellen die op hetzelfde Tailscale-account inloggen, zien elkaar alsof ze thuis op de wifi zitten, waar ze ook zijn. Zo kan papa onderweg de pagina's van de RPi en van elke controller openen.

Jij zit thuis al op hetzelfde wifi-netwerk. Voor toegang **van buiten** installeer je de Tailscale-app op je gsm en log je in op papa's Tailscale (papa geeft je toegang). Dan heb je dezelfde toegang als hij. Céline heeft dit niet nodig.

### Waarom (nog) geen Home Assistant?

Je hoort vaak: "koop een Raspberry Pi 5 en installeer **Home Assistant**". Dat is een sterk programma, maar **voor jou is het nu een omweg**:

- **Het werk is al gedaan.** De Matterbridge op de RPi 3B+ draait en is getest met Apple Home. Google Home is een extra koppeling, geen nieuw systeem.
- **Geen tweede systeem om te onderhouden.** Met Home Assistant krijg je er een volledig apart systeem bij (updates, back-ups, integraties), naast wat er in de schuur al draait. En om in Google Home te komen, heeft Home Assistant zelf ook weer een Matter-bridge nodig.
- **Kostprijs.** Een RPi 5 met voeding, behuizing en SSD kost nu al gauw **200 à 250 euro**. Voor een Matter-bridge volstaat een **RPi 3B+**, en via de bridge in de schuur kan het zelfs gratis.
- **Doorgroeien kan altijd.** Wil je later meer, bv. **camera's en video**, dan stap je over naar een zwaardere RPi. Je controllers en wat je geleerd hebt, neem je gewoon mee.
- **Zelfde manier van werken als de schuur.** Wat je leert over `/json`, de controllers en de bridge, is precies hoe de rest van Zarlar werkt. Zo kunnen we elkaar helpen.

Later, als je veel toestellen van andere merken wilt combineren of ingewikkelde automatisaties wilt maken, kan Home Assistant wél zinvol worden. Dan kijken we samen opnieuw.

## 4. Jouw ventilatiecontroller als voorbeeld

Je ventilatie (Begetube 350 R) heeft een ingang van **0 tot 10 V**: hoe hoger de spanning, hoe harder ze draait.

```
 [ gsm ] ─ wifi ─> [ ESP32 ] ─ PWM ─> [ AnaBoX ] ─ 0–10 V ─> [ ventilatie ]
```

1. **De ESP32** maakt een webpagina met een schuifbalk (`http://pwmtest.local`).
2. **PWM** betekent dat de ESP32 zijn uitgang duizenden keren per seconde aan en uit zet. Hoe langer "aan", hoe hoger de stand.
3. **De AnaBoX** maakt daar een rustige spanning van 0–10 V van.

Een belangrijke eigenaardigheid: **0 V betekent bij deze unit volle kracht.** Daarom start de sketch meteen in een veilige stand en gaat hij nooit onder 0,3 V.

Alles over deze controller (aansluitingen, pagina's, metingen) staat in de repo [`HVAC_WONING`](https://github.com/FidelDworp/HVAC_WONING).

Later kan deze ventilatiecontroller ook in de Matter-bridge, zodat je hem vanuit Google Home bedient.

## 5. Zelf beginnen: een leerpad

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
- Koppel de Matter-bridge aan je **Google Home** en bedien de schuur vanaf je gsm (zie §3).

## 6. Veiligheid

- **Werk alleen met lage spanning** (5 V, 10 V, 12 V). Aan **230 V** komen we niet: dat blijft binnen de toestellen of gaat via gekeurde relais en een elektricien.
- **Een ESP32-pin verdraagt maximaal 3,3 V.** Een sensor op 5 V kan het bordje kapotmaken. Meet eerst met de multimeter.
- **Dubbelcheck GND (min) en plus** voor je de voeding aansluit.
- Twijfel je? Vraag het, of vraag het aan Claude: die kent dit hele systeem.

## 7. Woordenlijst

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
| **Matter** | Gemeenschappelijke standaard waarmee toestellen in Apple Home, Google Home en andere apps verschijnen |
| **Matterbridge** | Programma op de Raspberry Pi dat onze controllers naar Matter vertaalt |
| **Google-hub** | Een Google-toestel (Nest Mini, Nest Hub, Google TV Streamer) dat Matter-toestellen voor Google Home bedient |
| **Tailscale** | Privé-tunnel zodat je van buitenaf aan de toestellen thuis kunt |
| **Home Assistant** | Apart, uitgebreid domoticaprogramma; voor jou nu niet nodig (zie §3) |

## 8. Waar vind je wat?

| Wat | Waar |
|---|---|
| Overzicht van alles | [`Zarlardinge-Home-Automation`](https://github.com/FidelDworp/Zarlardinge-Home-Automation) (deze repo) |
| Hoe het geheel in elkaar zit | [`docs/ARCHITECTUUR.md`](ARCHITECTUUR.md) |
| Jouw ventilatie | [`HVAC_WONING`](https://github.com/FidelDworp/HVAC_WONING) |
| Smart Energy in de woning | [`ESP32C6_Smart-Energy`](https://github.com/FidelDworp/ESP32C6_Smart-Energy), document `docs/Smart_Energy_Zarlardinge_Maarten.md` |
| Testsketches om mee te oefenen | [`ESP32C6_Experiments`](https://github.com/FidelDworp/ESP32C6_Experiments) |
| Alle technische details | [Master-document](../ESP32_Zarlar/Zarlar_Master_Overnamedocument.md) |

Veel plezier ermee!
