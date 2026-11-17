# Plan voor de overnamedocumentatie

*Voorstel v0.1 — 17 november 2026 — Filip Delannoy (FiDel), opgesteld met Claude. Status: ter bespreking.*

## 1. Doel

Het Zarlar-systeem moet ook zonder FiDel blijven werken, en iemand anders moet het kunnen **beheren**, **herstellen** en, als hij dat wil, **veranderen of afbouwen**. Eerst Maarten, maar ook een helper of een elektricien die even moet ingrijpen.

De technische kennis staat al grotendeels in de repo's. Wat ontbreekt, is vooral de **praktische laag**: wat doe je als er iets misloopt, waar ligt wat, wie heeft welke toegang.

## 2. Voor wie

| Lezer | Wat hij moet kunnen | Niveau |
|---|---|---|
| **Iedereen in huis, gasten-verantwoordelijke** | Licht en verwarming werkend krijgen als er iets uitvalt | 1 |
| **Maarten als beheerder** | Herstarten, een controller vervangen, updates zetten, backups bewaren, accounts beheren | 2 |
| **Maarten (of een helper) als ontwikkelaar** | Begrijpen hoe het werkt en het veranderen, met of zonder AI | 3 |
| **Een AI (Claude, Gemini)** | Meteen verder kunnen in een nieuw gesprek | 3 (overdracht per repo) |

## 3. Wat er al is en wat ontbreekt

| Document | Niveau | Stand |
|---|---|---|
| [`README.md`](../README.md) | 2–3 | ✅ overzicht van alle controllers en repo's |
| [`ARCHITECTUUR.md`](ARCHITECTUUR.md) | 3 | ✅ hoe het geheel werkt, waarom zo gebouwd |
| [`UITLEG_VOOR_MAARTEN.md`](UITLEG_VOOR_MAARTEN.md) | 3 | ✅ leerpad, bouwstenen, woordenlijst |
| [`DEPLOY.md`](DEPLOY.md) | 2 | ✅ software op een controller of de RPi zetten |
| [Master-document](Zarlar_Master_Overnamedocument.md), Portal Plan, README + standaard per repo | 3 | ✅ technische referentie |
| `<NAAM>_OVERDRACHT.md` per repo | AI | ✅ |
| **Noodkaarten per technische kast** | 1 | ❌ ontbreekt |
| **Inventaris:** waar staat welke kast, voeding, controller, zekering | 2 | ❌ ontbreekt (deels in bedradingsschema's) |
| **Toegang en accounts:** router, wifi, GitHub, Google, Apple Home, Tailscale, Particle, RPi | 2 | ❌ ontbreekt (mag **niet** in de repo) |
| **Herstelprocedures:** controller vervangen, RPi terugzetten, voeding vervangen | 2 | ❌ ontbreekt (deels in DEPLOY) |
| **Reserveonderdelen** en waar ze liggen | 2 | ❌ ontbreekt |
| **Eigen hardware:** Powerpixel, PowerQuad, Zarlar shield, RoomSense, PIReX — ontwerpbestanden, onderdelenlijst, gekende fixes | 3 | ⚠️ verspreid (EasyEDA, Eagle, gesprekken) |
| **Bedradingsschema's per kast** (zoals "ACCESS3-BB bedrading") | 2 | ⚠️ bestaan als tekening, nog niet in de repo's |
| **Afbouwen of vereenvoudigen:** wat kan door een gewoon product vervangen worden | 2 | ❌ ontbreekt |

## 4. Voorgestelde documenten

Alles in deze repo onder `docs/overname/`, behalve wat geheim is.

1. **`NOODKAART_<kast>.md`** — één A4 per technische kast, afgedrukt en geplastificeerd in de kast zelf:
   - wat deze kast bedient (kamers, lichtkringen, verwarming);
   - welke zekering voor wat, en waar de voedingen staan;
   - wat zonder domotica blijft werken (handschakelaars, verwarming in noodstand?);
   - herstarten: welke voeding uit en weer aan, in welke volgorde;
   - wie te bellen.
2. **`INVENTARIS.md`** — elke kast met plaats, foto, voedingen (spanning, stroom), controllers (IP, repo), zekeringen en het bijhorende bedradingsschema (PDF in `docs/overname/schema/`).
3. **`BEHEER.md`** — de beheerdershandleiding:
   - dagelijks gebruik (Apple Home, Google Home, portaal);
   - periodieke taken (backups, Sheets nakijken, batterijen, stof);
   - updates zetten (verwijzing naar DEPLOY);
   - wat te doen bij een melding of een rode tegel op het Dashboard.
4. **`HERSTEL.md`** — stap voor stap:
   - een ESP32 vervangen (nieuw devkit, eerste keer via USB, daarna instellingen via `/settings`);
   - de RPi terugzetten vanaf een image van de SD-kaart;
   - een voeding of een Powerpixel vervangen.
5. **`TOEGANG.md`** — **alleen** een lijst van *welke* accounts er zijn en *waar* de gegevens liggen (wachtwoordbeheerder of verzegelde envelop). **Nooit wachtwoorden of tokens in de repo.**
6. **`RESERVE.md`** — welke reserveonderdelen er zijn, hoeveel, en waar ze liggen: voorgeprogrammeerde ESP32's, Powerpixels, shields, voedingen, zekeringen.
7. **`EIGEN_HARDWARE.md`** — per eigen print: doel, schema en print (PDF + ontwerpbestanden), onderdelenlijst, gekende fixes (bv. het fix-draadje naar +12V) en waar je ze laat maken.
8. **`AFBOUW.md`** — per deel van het systeem: kan het weg, en wat komt er dan in de plaats? Bijvoorbeeld licht via gewone schakelaars, een commerciële thermostaat. Ook wat zeker moet blijven werken (verwarming, ventilatie, buitenlicht B&B).

## 5. Volgorde (één stap per keer, met feedback ertussen)

| Stap | Wat | Wie |
|---|---|---|
| 1 | Lijst van alle technische kasten en hun plaats, met foto's | FiDel levert, Claude zet om in `INVENTARIS.md` |
| 2 | **Eerste noodkaart: de B&B-kast** (nu toch in opbouw), als proef | Claude stelt op, FiDel verbetert |
| 3 | **Proef met Maarten:** hij voert de noodkaart uit zonder hulp. Wat onduidelijk is, passen we aan | Maarten + FiDel |
| 4 | Noodkaarten voor de andere kasten, met dezelfde opbouw | Claude + FiDel |
| 5 | `TOEGANG.md` en de wachtwoorden op een veilige plek | FiDel (Claude alleen de lijst) |
| 6 | `RESERVE.md` en de reserveonderdelen effectief klaarleggen | FiDel |
| 7 | `HERSTEL.md`, met een echte proef: één controller vervangen | Claude + FiDel |
| 8 | `EIGEN_HARDWARE.md`: ontwerpbestanden in een repo bijeenbrengen | FiDel levert, Claude ordent |
| 9 | `BEHEER.md` en `AFBOUW.md` | Claude + FiDel, samen met Maarten |
| 10 | **Jaarlijkse oefening:** Maarten doorloopt de noodkaart van één kast en vervangt één controller | Maarten |

## 6. Vragen aan FiDel

1. Welke **technische kasten** zijn er, en waar staan ze?
2. Wat gebeurt er met de **verwarming** als de HVAC-controller of een ROOM uitvalt? Is er een noodstand?
3. Waar staan de **ontwerpbestanden** van je eigen printen (EasyEDA-account, Eagle-bestanden op de Mac)?
4. Waar wil je de **wachtwoorden** bewaren: wachtwoordbeheerder met noodtoegang, of verzegelde envelop?
5. Wil Maarten later **zelf ontwikkelen**, of vooral **beheren**? Dat bepaalt hoe ver we niveau 3 uitwerken.
