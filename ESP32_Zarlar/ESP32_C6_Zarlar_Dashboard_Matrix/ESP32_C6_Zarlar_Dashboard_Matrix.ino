/* ============================================================
   Zarlar Dashboard v6.4 (zonder Matter)
   ESP32-C6 (32-pin 16MB) @ 192.168.0.60
   Filip Delannoy

   BOARD INSTELLINGEN (Arduino IDE):
     Board        : ESP32-C6 Dev Module (espressif:arduino-esp32-master)
     Upload Speed : 921600
     Flash Size   : 16MB
     Partition    : Custom → partitions.csv (naast .ino)

   EERSTE GEBRUIK / FACTORY RESET:
     1. Upload sketch → verbindt automatisch met WiFi
     2. Bij geen WiFi → AP "Zarlar-Setup" verschijnt
     3. Verbind met "Zarlar-Setup" → browser opent /settings automatisch
     4. Vul WiFi SSID + wachtwoord in, sla op → herstart
     5. Daarna: open http://192.168.0.60/settings voor controllers

   NETWERK:
     Fixed IP  : 192.168.0.60
     Toegang   : http://192.168.0.60/ (geen mDNS)
     AP SSID   : Zarlar-Setup (bij geen WiFi)

   APPLE HOME:
     - Geen Matter meer op deze controller (v6.0). HOME/UIT in Apple Home loopt via
       de RPi-bridge (server.js + matterbridge-zarlar): leest /home_json, zet via
       /set_home_global?v=0|1.
     - Serial: reset-all / status / matrix-test

   STATUSMATRIX — 16x16 WS2812B op IO4 (Pixel-line, shield R3=33Ω):
     Rij 0  : S-HVAC
     Rij 1  : S-ECO
     Rij 2  : S-ENERGY (idx 4, IP 192.168.0.73)
     Rij 3  : S-ACCESS  — de 4 buitenmuren W·Z·N·O (v6.4), bron ACCESS1 (ESP32) of P-Access (Photon R8-ACCESS)
     Rij 4  : Separator (amber)
     Rij 5  : R-BandB
     Rij 6  : R-BADK
     Rij 7  : R-INKOM
     Rij 8  : R-KEUKEN
     Rij 9  : R-WASPL
     Rij 10 : R-EETPL / TESTROOM (controller idx 13)
     Rij 11 : R-ZITPL
     Rij 12-15: leeg

   ⚠️  SERPENTINE RICHTING: matrix wordt bij eerste gebruik getest via
       /matrix_test of Serial commando 'matrix-test'. Pas MATRIX_FLIP_H
       aan als kolommen gespiegeld zijn.

   06nov26        v6.4  RIJ 3 = BUITENVERLICHTING, de 4 muren rondom het huis:
                          kol 0-3 W  : status · PIR (W1 of W2) · licht W1 Voordeur · licht W2 Veldkant
                          kol 4-7 Z  : status · PIR S · licht S Terras · vrij
                          kol 8-11 N : status dim (ACCESS2 nog niet aanwezig)
                          kol 12-15 O: status dim (ACCESS3 nog niet aanwezig)
                        - Status: groen = online + automatisch (PIR), oranje = online + manueel,
                          rood = offline, geel = wacht op eerste poll. PIR paars, licht warm geel.
                        - Bron: ESP32 S-ACCESS (.82) als die actief is, anders de nieuwe Photon
                          P-Access (R8-ACCESS, variabele JSON_status via de Worker). P-Access staat
                          achteraan in de controllerlijst (idx 22): bestaande indexen ongewijzigd.
                          Niet naar Google Sheets. NUM_CONTROLLERS 22 → 23.
                        - Testpatroon: rij 2 heette nog OUTSIDE → ENERGY (alleen het label).
   01nov26        v6.3  Kleuren volgens de gedrukte legende + rust zachter:
                        - rustPx(): eigen, lager niveau MAT_RUST_NIVEAU (3/255, 0 = uit),
                          los van de ondergrens voor actieve leds (MAT_MIN_ZICHTBAAR 7).
                        - HOME: blauw = thuis, rust = weg (was omgekeerd). Beweging paars
                          (was wit). Dauwpunt-alert geel, geen alert = rust.
                        - ECO-rij continu: Tsun blauw→rood (0-90 °C), dT/opbrengst/EQtot/dEQ
                          als intensiteit, heap% groen→rood.
                        - ENERGY-rij continu: vermogens/dagtotalen als intensiteit (0 = rust),
                          EPEX groen→rood (5-35 ct, negatief = cyaan), maandpiek groen→rood.
   01nov26        v6.2  MATRIX DUIDELIJKER + LIVE:
                        - matPx(): gamma-correctie + eigen helderheidsschaal + ondergrens
                          (MAT_MIN_ZICHTBAAR): elke led die "aan" is blijft zichtbaar, ook
                          de rusttoestanden (waren 10-25/255 × helderheid = onzichtbaar).
                          matrix.setBrightness() staat vast op 255.
                        - Grootheden CONTINU i.p.v. trapjes (HSV-verloop): temperatuur
                          diep blauw → fel rood; vocht, CO2, heap, RSSI groen → rood.
                          Room-temperatuur t.o.v. het setpoint (key c): −3 °C blauw,
                          setpoint groen, +3 °C rood (zonder setpoint: absoluut 15-28 °C).
                        - Actieve toestanden fel (255), rust = zacht grijs (MAT_RUST).
                        - LIVE-UPDATE: ESP32-controllers elke live_sec (standaard 15 s,
                          instelbaar in /settings, 0 = uit) gepolld + matrix vernieuwd.
                          Enkel HTTP; Sheets-posts en Photon-polls (TLS) blijven in de
                          gewone cyclus (poll_min), dus even vaak als voorheen.
   01nov26        v6.1  OTA-pagina: /update (GET = uploadformulier, POST = .bin flashen via
                        Update.h, daarna herstart). Knop "⬆ OTA update" in /settings → Systeem
                        info. Bestand: Sketch → Export Compiled Binary → enkel
                        ESP32_C6_Zarlar_Dashboard_Matrix.ino.bin (niet bootloader/merged).
   01nov26        v6.0  ZONDER MATTER (Master §16): includes, matter_home + callback,
                        Matter.begin() + pairing-meldingen in setup(), 5 s-update in loop(),
                        matterNuclearReset() + vlag, /matter_json, /matter_reset,
                        Matter-sectie in /settings, serial reset-matter weg.
                        Bestandsnaam ESP32_C6_Zarlar_Dashboard-Matrix_MATTER.ino →
                        ESP32_C6_Zarlar_Dashboard_Matrix.ino.
                        NIEUW: /home_json → {"home":0|1} (leesbaar voor de RPi-bridge).
                        #define DASH_VERSION: opstarttekst en serial 'status' tonen
                        voortaan dezelfde versie (stonden op v5.7 / v5.8).
                        Header: partitions.csv i.p.v. partitions_16mb.csv.
   15apr26        v5.9  FIX erratic room-rijen op statusmatrix (Photon-rooms):
                        P-TESTROOM (duplicate photon_id van P-Zitpl, ongebruikt
                        in MROW) uit round-robin gehaald: active=false.
                        pollPhotonController(): 1 automatische retry binnen
                        dezelfde poll-beurt bij mislukking (vangt eenmalige
                        TLS/timeout-hikjes op).
                        Hysterese via nieuwe photon_fail_count[]: status/last_json
                        blijven behouden tot PHOTON_FAIL_THRESHOLD (3) opeenvolgende
                        mislukkingen — voorkomt dat 1 mislukte HTTPS-call naar de
                        Cloudflare Worker een rij meteen zwart/rood maakt terwijl
                        de Photon in werkelijkheid online is.
   15apr26        v5.8  S-ENERGY controller toegevoegd (idx 4, IP 192.168.0.73).
                        FUT1 → S-ENERGY, active=true.
                        MROW rij 2: S-OUTSIDE (gereserveerd) → S-ENERGY.
                        renderEnergyRow(): 16 kolommen conform /json v1.26.
                        sim_s0/sim_p1 vlaggen zichtbaar op matrix (oranje=SIM, groen=LIVE).
   15apr26        v5.7  Sheets POST throttle: elke SHEETS_POST_EVERY (5) cycli.
                        Elimineert 3 HTTPS POST/min → grootste TLS-fragmentatiebron.
                        Timeouts verlaagd: ESP32 GET 4s→1.5s, Sheets 10s→3s, Photon 6s→3s.
                        UI en matrix blijven elke minuut vers, Sheets om de 5 minuten.
   15apr26        v5.6  Photon round-robin: per cyclus slechts 1 Photon gepolld.
                        Nooit meer dan 1 TLS-verbinding per minuut.
                        photon_round_robin teller, modulo actieve Photons.
   14apr26        v5.5  TLS-fragmentatie fix: Photon controllers slechts elke
                        PHOTON_POLL_EVERY (5) cycli gepolld ipv elke minuut.
                        Reduceert TLS heap-druk van 5 naar 1 verbinding/5min.
                        photon_skip_counter globale variabele in handlePolling().
   14apr26        v5.4  Heap fix: getStatusJson() → handleStatusJson() met static char buf[2400]
                        in BSS. Elke 15s browser-poll lokte String alloc/free uit → snelste
                        fragmentatiebron. Nul heap-allocatie meer voor status JSON.
   14apr26        v5.3  Poll fix: stream->readBytes() vervangen door getString()+strlcpy().
                        readBytes(699) blokkeerde loop() tot timeout per controller → UI bevroor.
                        Tijdelijke String per poll, onmiddellijk vrijgegeven na strlcpy → geen
                        permanente heap-allocatie, geen fragmentatie, geen blocking.
   14apr26        v5.2  ROOT CAUSE FIX heap-fragmentatie:
                        struct Controller: String last_json → char last_json[700] (BSS).
                        Poll functies: HTTPClient stream → direct lezen in char buffer.
                        logControllerToSheets(): snprintf POST-body, geen String-allocatie.
                        getStatusJson(): strstr/atoi in plaats van String.indexOf/substring.
                        jF()/jI(): const char* i.p.v. const String& → geen String-kopie.
                        Render-functies: const char* j i.p.v. const String& j.
                        Reboot-knop toegevoegd aan /settings.
                        Auto-herstart bij largest_block < 25KB voor > 60s.
   02apr26        v5.1  5 Controllers vervangen door "static damage!": ZITPL EETPL, OUTSIDE, ACCESS en INKOM (Nieuwe IDs)
   01apr26        v5.0  Automatische ESP32/Photon fallback per matrix-rij.
                        MatrixRowDef struct: esp_idx + photon_idx + sys_idx.
                        MROW volgorde gecorrigeerd: stemt nu exact overeen
                        met SVG labelsheet (rij 5=BandB...11=ZITPL).
                        updateMatrix(): ESP32 actief? → ESP32. Anders Photon.
                        Anders zwart. Geen reflash nodig bij transitie.
                        Serial 'status' toont per rij welke controller actief is.
   01apr26        v4.9  MROW fix: Photon indices 16-19 → 15-18 (off-by-one).
                        Rijen 2,3 naar -1 (gereserveerd, geen PROBLEEM melding meer).
   01apr26        v4.8  fetchPhoton() vereenvoudigd: worker /sensor ipv Particle API.
                        getToken() + PT variabele verwijderd — token niet meer in browser.
                        Serial hint 'status' na poll cyclus.
   01apr26        v4.7  Photon poll via Cloudflare Worker. renderPhotonRow().
                        renderPhotonRow() met Photon JSON keys (a,d,e,g,i,j,k,l,q,s,t,u,x).
                        MROW: rijen 5-8 → P-Badkamer/Inkom/Keuken/Waspl tijdens transitie.
                        Heap-guard 30KB voor TLS poll.
   01apr26        v4.6  Separator + gereserveerde rijen → zwart.
                        MROW rij 10: R-EETPL idx 13→11 (geverifieerd via status).
                        Serial 'status': matrix ctrl-index debug toegevoegd.
                        Verouderde HVAC key-waarschuwing verwijderd.
   26mar26        v4.5  ECO renderer herwerkt: collector→pomp→6 boilerlagen→energie→heap.
                        Pomp relay weg (PWM volstaat), HOME global weg, FreeHeap% toegevoegd.
   26mar26        v4.4  ROOM col13: pixel_on_str wit evenredig met actieve pixels.
   26mar26        v4.3  ROOM col10: werkelijke kamerkleur RGB. Col11: dag=geel/nacht=purper.
                        Col12: LDR1 geel omgekeerd evenredig. Col13: TSL2561 lux amber log.
   26mar26        v4.2  Boot-animatie: rode pixel door HVAC+ECO (rij 0-1),
                        blauwe pixel door ROOM-rijen (5-11). SVG labels v3.
   26mar26        v4.1  HVAC_KEY_* defines gecorrigeerd op basis van
                        geverifieerde v1.19 sketch (keys p,q,r,s,t,u,v,
                        y,aa,x,h,ae,ac). §3.6 overnamedocument aangevuld.
   26mar26        v4.0  16x16 NeoPixel statusmatrix op IO4.
                        Helderheid instelbaar via /settings UI.
                        /matrix_test endpoint + Serial 'matrix-test'.
                        Boot-animatie: sweep per rij groen.
                        Renderer per controller type (room/eco/hvac/reserved).
   17mar26        v3.0  Matter: MatterOnOffPlugin HOME/UIT. #define Serial Serial0.
                        ESPmDNS verwijderd. matterNuclearReset() — settings bewaard.
                        Matter sectie in /settings. Serial commando's.
   17mar26        v2.9.1 Bugfix: ECO RSSI-key "p" ipv "ac".
   17mar26        v2.9  RSSI op dashboard-knoppen.
   17mar26        v2.8  Header hersteld. Ping port 80→53.
   17mar26        v2.7  Matter-voorbereiding: mDNS verwijderd, 16MB board.
   17mar26        v2.6  WiFi Strength Tester: /wifi pagina.
   16mar26        v2.5  /info → /settings. max_rows naar GAS.
   16mar26        v2.4  HOME/UIT broadcast naar alle rooms.
   11mar26        v2.3  WiFi sleep uit. AP + captive portal.
   11mar26        v2.0  Volledig herschreven.
   ============================================================ */

// ⚠️ Verplicht voor ESP32-C6 (RISC-V) — vóór alle #include statements
#define Serial Serial0

#define DASH_VERSION "6.4"

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include <esp_sleep.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Update.h>   // v6.1: OTA via /update
#include <nvs.h>
#include <nvs_flash.h>
#include <Adafruit_NeoPixel.h>

// ============================================================
// CONTROLLER TYPES & STATUS
// ============================================================
#define TYPE_PHOTON  0
#define TYPE_SYSTEM  1
#define TYPE_ROOM    2

#define STATUS_INACTIVE  0   // grijs
#define STATUS_PENDING   1   // geel
#define STATUS_ONLINE    2   // groen
#define STATUS_OFFLINE   3   // rood

// ============================================================
// CONTROLLER STRUCT
// v5.2: last_json is char[700] in BSS — geen heap-allocatie meer.
// Eliminineert de graduele heap-fragmentatie die elke poll-cyclus
// optrad door String realloc van ~10 actieve controllers.
// BSS-kost: 22 × 700 = 15.4 KB (vast, geen fragmentatie).
// ============================================================
struct Controller {
  const char* name;
  const char* ip;
  const char* photon_id;
  int         type;
  bool        active;
  int         status;
  String      sheets_url;
  char        last_json[700];   // ← v5.2: was String last_json
  unsigned long last_poll;
};

// ============================================================
// CONTROLLER TABEL
// ============================================================
#define NUM_CONTROLLERS 23   // v6.4: + P-Access (idx 22)

Controller controllers[NUM_CONTROLLERS] = {
  {"S-HVAC",    "192.168.0.70", "", TYPE_SYSTEM, true,  STATUS_PENDING,  "", {}, 0},
  {"S-ECO",     "192.168.0.71", "", TYPE_SYSTEM, true,  STATUS_PENDING,  "", {}, 0},
  {"S-OUTSIDE", "192.168.0.72", "", TYPE_SYSTEM, false, STATUS_INACTIVE, "", {}, 0},
  {"S-ACCESS",  "192.168.0.82", "", TYPE_SYSTEM, false, STATUS_INACTIVE, "", {}, 0},
  {"S-ENERGY",  "192.168.0.73", "", TYPE_SYSTEM, true,  STATUS_PENDING,  "", {}, 0},
  {"FUT2",      "192.168.0.74", "", TYPE_SYSTEM, false, STATUS_INACTIVE, "", {}, 0},
  {"R-BandB",   "192.168.0.75", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-BADK",    "192.168.0.76", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-INKOM",   "192.168.0.77", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-KEUKEN",  "192.168.0.78", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-WASPL",   "192.168.0.79", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-EETPL",   "192.168.0.80", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-ZITPL",   "192.168.0.81", "", TYPE_ROOM,   false, STATUS_INACTIVE, "", {}, 0},
  {"R-TESTROOM","192.168.0.80", "", TYPE_ROOM,   true,  STATUS_PENDING,  "", {}, 0},
  {"P-BandB",   "", "30002c000547343233323032", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Badkamer","", "5600420005504b464d323520", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Inkom",   "", "2c0026000747343232363230", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Keuken",  "", "310017001647373335333438", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Waspl",   "", "33004f000e504b464d323520", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Eetpl",   "", "3c0030000a47353138383138", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-Zitpl",   "", "200033000547373336323230", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},
  {"P-TESTROOM","", "200033000547373336323230", TYPE_PHOTON, false, STATUS_INACTIVE, "", {}, 0},
  {"P-Access",  "", "30002d000747353138383138", TYPE_PHOTON, true,  STATUS_PENDING,  "", {}, 0},  // v6.4: R8-ACCESS
};

// v6.4: buitenverlichting op matrixrij 3
#define ACCESS_ROW    3
#define IDX_S_ACCESS  3    // ESP32 ACCESS1 (.82), later
#define IDX_P_ACCESS  22   // Photon R8-ACCESS (tot ACCESS1 er is)

// v5.9: hysterese voor Photon-polling — telt opeenvolgende mislukkingen
// per controller. Pas na PHOTON_FAIL_THRESHOLD mislukkingen op rij wordt
// de rij echt als offline getoond; transiënte TLS/timeout-hikjes worden
// zo niet meer meteen zichtbaar op de matrix.
uint8_t photon_fail_count[NUM_CONTROLLERS] = {0};
#define PHOTON_FAIL_THRESHOLD 3

// ============================================================
// NVS
// ============================================================
Preferences preferences;
const char* NVS_NS            = "zarlar";
const char* NVS_WIFI_SSID     = "wifi_ssid";
const char* NVS_WIFI_PASS     = "wifi_pass";
const char* NVS_POLL_MIN      = "poll_min";
const char* NVS_ROOM_SCRIPT   = "room_script";
const char* NVS_HOME_GLOBAL   = "home_global";
const char* NVS_MATRIX_BRIGHT = "matrix_br";
const char* NVS_LIVE_SEC      = "live_sec";   // v6.2

// ============================================================
// GLOBALS
// ============================================================
WebServer  server(80);
DNSServer  dnsServer;

String wifi_ssid       = "";
String wifi_pass       = "";
int    poll_minutes    = 10;
int    live_sec        = 15;      // v6.2: live-update ESP32-controllers (s), 0 = uit
unsigned long last_live_cycle = 0;
bool   poll_live       = false;   // v6.2: lopende cyclus is een live-cyclus (geen Sheets/Photon)
String room_script_url = "";

bool          ap_mode          = false;
unsigned long last_poll_cycle  = 0;
int           poll_index       = 0;
bool          polling_active   = false;
unsigned long poll_step_timer  = 0;
bool          home_mode_global = false;

// Photon poll round-robin — v5.6
// Per cyclus wordt slechts ÉÉN Photon gepolld (round-robin op index).
// Nooit meer dan 1 TLS-verbinding per poll-cyclus → minimale heap-fragmentatie.
// Elke Photon krijgt data om de N_ACTIVE_PHOTONS cycli (= minuten).
static int photon_round_robin = 0;  // index in de reeks actieve Photons

// Sheets POST throttle — v5.7
// POST naar Sheets slechts elke SHEETS_POST_EVERY cycli.
// Elimineert 3 HTTPS-verbindingen/minuut → grootste TLS-fragmentatiebron.
// UI en matrix blijven elke minuut vers. Sheets krijgt data om de 5 minuten.
#define SHEETS_POST_EVERY 5
static int sheets_skip_counter = 0;

// WiFi tester — 3 primitieven
static bool wt_scan_pending = false;
static int  wt_snap_rssi    = 0;
static int  wt_snap_ping    = -1;


// ============================================================
// STATUSMATRIX — v4.0
// 16×16 WS2812B op IO4 via Pixel-line (shield R3=33Ω aanwezig)
// Voeding: 5V rechtstreeks op de matrix (NIET via shield PTC 500mA)
//
// Serpentine adressering (standaard AliExpress 16×16 panel):
//   Datadraden = onderaan → pixel 0 = links-onder
//   Rij 0 (onder): pixels 0→15 links→rechts
//   Rij 1: pixels 31→16 rechts→links
//   etc.
// Logisch rij 0 = bovenaan scherm = fysieke rij 15
//
// ⚠️ Als pixels gespiegeld zijn: stel MATRIX_FLIP_H = true in
// ============================================================
#define MATRIX_PIN    4
#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 16
#define MATRIX_LEDS   256
#define MATRIX_FLIP_H false

Adafruit_NeoPixel matrix(MATRIX_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);
uint8_t           matrix_brightness  = 60;
unsigned long     last_matrix_update = 0;

// ============================================================
// MATRIX RIJ-MAPPING — automatische ESP32/Photon fallback
// ============================================================
struct MatrixRowDef {
  int  esp_idx;
  int  photon_idx;
  int  sys_idx;
};

const MatrixRowDef MROW[MATRIX_HEIGHT] = {
  {  -1,  -1,   0 }, // rij  0: S-HVAC
  {  -1,  -1,   1 }, // rij  1: S-ECO
  {  -1,  -1,   4 }, // rij  2: S-ENERGY (idx 4, IP 192.168.0.73)
  {  -1,  -1,  -1 }, // rij  3: S-ACCESS  — v6.4: renderAccessRow() (4 muren)
  {  -1,  -1,  -2 }, // rij  4: separator
  {   6,  14,  -1 }, // rij  5: R-BandB  / P-BandB
  {   7,  15,  -1 }, // rij  6: R-BADK   / P-Badkamer
  {   8,  16,  -1 }, // rij  7: R-INKOM  / P-Inkom
  {   9,  17,  -1 }, // rij  8: R-KEUKEN / P-Keuken
  {  10,  18,  -1 }, // rij  9: R-WASPL  / P-Waspl
  {  11,  19,  -1 }, // rij 10: R-EETPL  / P-Eetpl
  {  12,  20,  -1 }, // rij 11: R-ZITPL  / P-Zitpl
  {  -1,  -1,  -1 }, // rij 12–15: leeg
  {  -1,  -1,  -1 },
  {  -1,  -1,  -1 },
  {  -1,  -1,  -1 },
};

// ============================================================
// HVAC JSON KEYS — geverifieerd 26 maart 2026 via v1.19 sketch
// ============================================================
#define HVAC_KEY_RSSI     "ac"
#define HVAC_KEY_CIRCUIT1 "p"
#define HVAC_KEY_CIRCUIT2 "q"
#define HVAC_KEY_CIRCUIT3 "r"
#define HVAC_KEY_CIRCUIT4 "s"
#define HVAC_KEY_CIRCUIT5 "t"
#define HVAC_KEY_CIRCUIT6 "u"
#define HVAC_KEY_CIRCUIT7 "v"
#define HVAC_KEY_PUMP_SCH "y"
#define HVAC_KEY_PUMP_WON "aa"
#define HVAC_KEY_VENT     "x"
#define HVAC_KEY_TEMP1    "h"
#define HVAC_KEY_HEAP     "ae"

// ============================================================
// NVS LADEN / OPSLAAN
// ============================================================
void loadNVS() {
  preferences.begin(NVS_NS, true);
  wifi_ssid         = preferences.getString(NVS_WIFI_SSID, "");
  wifi_pass         = preferences.getString(NVS_WIFI_PASS, "");
  poll_minutes      = preferences.getInt(NVS_POLL_MIN, 10);
  room_script_url   = preferences.getString(NVS_ROOM_SCRIPT, "");
  home_mode_global  = preferences.getBool(NVS_HOME_GLOBAL, false);
  matrix_brightness = preferences.getUChar(NVS_MATRIX_BRIGHT, 60);
  live_sec          = preferences.getInt(NVS_LIVE_SEC, 15);
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    char ka[12], ku[12];
    snprintf(ka, sizeof(ka), "c%d_act", i);
    snprintf(ku, sizeof(ku), "c%d_url", i);
    controllers[i].active     = preferences.getBool(ka, controllers[i].active);
    controllers[i].sheets_url = preferences.getString(ku, "");
    controllers[i].status     = controllers[i].active ? STATUS_PENDING : STATUS_INACTIVE;
  }
  preferences.end();
}

void saveNVS() {
  preferences.begin(NVS_NS, false);
  preferences.putString(NVS_WIFI_SSID,      wifi_ssid);
  preferences.putString(NVS_WIFI_PASS,      wifi_pass);
  preferences.putInt   (NVS_POLL_MIN,       poll_minutes);
  preferences.putString(NVS_ROOM_SCRIPT,    room_script_url);
  preferences.putUChar (NVS_MATRIX_BRIGHT,  matrix_brightness);
  preferences.putInt   (NVS_LIVE_SEC,       live_sec);
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    char ka[12], ku[12];
    snprintf(ka, sizeof(ka), "c%d_act", i);
    snprintf(ku, sizeof(ku), "c%d_url", i);
    preferences.putBool  (ka, controllers[i].active);
    preferences.putString(ku, controllers[i].sheets_url);
  }
  preferences.end();
}

// ============================================================
// WIFI VERBINDEN
// ============================================================
void connectWiFi() {
  if (wifi_ssid.length() == 0) {
    Serial.println("Geen WiFi geconfigureerd → AP mode");
    startAP();
    return;
  }
  Serial.print("WiFi verbinden met " + wifi_ssid);
  WiFi.mode(WIFI_STA);
  IPAddress ip(192,168,0,60), gw(192,168,0,1), sn(255,255,255,0), dns(8,8,8,8);
  WiFi.config(ip, gw, sn, gw, dns);
  WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 40) {
    delay(500); Serial.print("."); retries++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    esp_wifi_set_ps(WIFI_PS_NONE);
    wifi_config_t wifi_cfg;
    esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);
    wifi_cfg.sta.listen_interval = 1;
    esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg);
    esp_pm_config_t pm_cfg = { .max_freq_mhz=160, .min_freq_mhz=160, .light_sleep_enable=false };
    esp_pm_configure(&pm_cfg);
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    Serial.println("✓ WiFi verbonden: " + WiFi.localIP().toString());
    Serial.printf("  RSSI: %d dBm\n", WiFi.RSSI());
    ap_mode = false;
  } else {
    Serial.println("✗ WiFi mislukt → AP mode");
    startAP();
  }
}

// ============================================================
// AP MODE + CAPTIVE PORTAL
// ============================================================
void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("Zarlar-Setup");
  IPAddress ap_ip = WiFi.softAPIP();
  dnsServer.start(53, "*", ap_ip);
  ap_mode = true;
  Serial.println("AP mode: SSID=Zarlar-Setup  IP=" + ap_ip.toString());
}

// ============================================================
// POLL ÉÉN ESP32 CONTROLLER
// v5.2: HTTPClient stream → direct lezen in char last_json[700].
//       Geen String-allocatie meer — elimineert heap-fragmentatie.
// ============================================================
void pollESP32Controller(int i) {
  if (!controllers[i].active) return;
  if (strlen(controllers[i].ip) == 0) return;
  String url = "http://" + String(controllers[i].ip) + "/json";
  if (!poll_live) Serial.printf("[Poll] %s → %s\n", controllers[i].name, url.c_str());
  HTTPClient http;
  http.begin(url);
  http.setTimeout(1500);
  http.setConnectTimeout(1000);
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    http.end();
    strlcpy(controllers[i].last_json, body.c_str(), sizeof(controllers[i].last_json));
    controllers[i].status    = STATUS_ONLINE;
    controllers[i].last_poll = millis();
    if (!poll_live) Serial.printf("  ✓ %s online (%d bytes)\n", controllers[i].name, (int)body.length());
    if (poll_live) {
      // v6.2: live-cyclus — geen Sheets
    } else if (sheets_skip_counter == 0) {
      logControllerToSheets(i);
    } else {
      Serial.printf("  [Sheets] %s overgeslagen (cyclus %d/%d)\n",
                    controllers[i].name, sheets_skip_counter, SHEETS_POST_EVERY);
    }
  } else {
    controllers[i].status = STATUS_OFFLINE;
    controllers[i].last_json[0] = '\0';
    Serial.printf("  ✗ %s offline (HTTP %d)\n", controllers[i].name, code);
    http.end();
  }
}

// ============================================================
// POLL PHOTON CONTROLLER via Cloudflare Worker
// v5.2: HTTPClient stream → direct lezen in char last_json[700].
// ============================================================
#define PHOTON_WORKER_HOST "controllers-diagnose.filip-delannoy.workers.dev"
#define PHOTON_WORKER_PATH "/sensor?id="

void pollPhotonController(int i) {
  if (!controllers[i].active) return;
  if (strlen(controllers[i].photon_id) == 0) return;

  if (heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) < 30000) {
    Serial.printf("  [Photon] %s overgeslagen — heap te laag\n", controllers[i].name);
    return;
  }

  String url = "https://" + String(PHOTON_WORKER_HOST) + String(PHOTON_WORKER_PATH)
               + String(controllers[i].photon_id);

  bool ok = false;
  String body;

  // v5.9: max 2 pogingen binnen dezelfde poll-beurt — vangt eenmalige
  // TLS/timeout-hikjes op zonder te wachten op de volgende round-robin
  // cyclus (die tot ~70-80 minuten kan duren).
  for (int attempt = 0; attempt < 2 && !ok; attempt++) {
    if (attempt > 0) {
      Serial.printf("  [Photon] %s retry...\n", controllers[i].name);
      delay(300);
    }
    Serial.printf("[Poll] %s → worker/%s (poging %d)\n",
                  controllers[i].name, controllers[i].photon_id, attempt + 1);
    HTTPClient http;
    http.begin(url);
    http.setTimeout(3000);
    http.setConnectTimeout(2000);
    int code = http.GET();
    if (code == 200) {
      body = http.getString();
      http.end();
      if (body.indexOf("\"online\":1") >= 0) ok = true;
    } else {
      http.end();
      Serial.printf("  ✗ %s HTTP %d (poging %d)\n", controllers[i].name, code, attempt + 1);
    }
  }

  if (ok) {
    strlcpy(controllers[i].last_json, body.c_str(), sizeof(controllers[i].last_json));
    controllers[i].status    = STATUS_ONLINE;
    controllers[i].last_poll = millis();
    photon_fail_count[i]     = 0;
    Serial.printf("  ✓ %s online (%d bytes)\n", controllers[i].name, (int)body.length());
  } else {
    photon_fail_count[i]++;
    Serial.printf("  ⚠ %s mislukt (%d/%d opeenvolgend)\n",
                  controllers[i].name, photon_fail_count[i], PHOTON_FAIL_THRESHOLD);
    if (photon_fail_count[i] >= PHOTON_FAIL_THRESHOLD) {
      controllers[i].status = STATUS_OFFLINE;
      controllers[i].last_json[0] = '\0';
      Serial.printf("  ✗ %s nu echt OFFLINE na %d mislukkingen\n",
                    controllers[i].name, photon_fail_count[i]);
    } else {
      Serial.printf("  … %s status behouden (nog niet als offline gemarkeerd)\n",
                    controllers[i].name);
    }
  }
}

void setAllRoomsHomeMode(int mode) {
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    if (!controllers[i].active) continue;
    if (controllers[i].type != TYPE_ROOM) continue;
    if (strlen(controllers[i].ip) == 0) continue;
    char url[48];
    snprintf(url, sizeof(url), "http://%s/set_home?v=%d", controllers[i].ip, mode);
    HTTPClient http;
    http.begin(url);
    http.setTimeout(2000);
    http.setConnectTimeout(1000);
    int code = http.GET();
    Serial.printf("[HOME] %s → %s (HTTP %d)\n", controllers[i].name, mode ? "HOME" : "UIT", code);
    http.end();
  }
}

// ============================================================
// LOG NAAR GOOGLE SHEETS
// v5.2: snprintf POST-body naar char buf op stack — geen String-allocatie.
// ============================================================
void logControllerToSheets(int i) {
  const char* url = (controllers[i].type == TYPE_ROOM)
                    ? room_script_url.c_str()
                    : controllers[i].sheets_url.c_str();
  if (url[0] == '\0') return;
  if (controllers[i].last_json[0] == '\0') return;

  // Bouw POST-body: {"room":"naam", rest van json zonder openende {
  // last_json begint met '{' — skip die en vervang door onze header
  char postbuf[750];
  snprintf(postbuf, sizeof(postbuf), "{\"room\":\"%s\",%s",
           controllers[i].name,
           controllers[i].last_json + 1);  // +1: sla '{' over

  Serial.printf("  [Sheets] POST %s...\n", controllers[i].name);
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(3000);
  int code = http.POST((uint8_t*)postbuf, strlen(postbuf));
  Serial.printf("  [Sheets] %s %s\n", controllers[i].name,
                (code == 200 || code == 302) ? "OK ✓" : "fout");
  http.end();
}

// ============================================================
// POLL CYCLUS — gespreid 2s per controller
// ============================================================
void handlePolling() {
  if (ap_mode) return;
  unsigned long now = millis();
  if (!polling_active) {
    if (now - last_poll_cycle >= (unsigned long)poll_minutes * 60000UL) {
      last_poll_cycle = now;
      last_live_cycle = now;
      polling_active  = true;
      poll_live       = false;
      poll_index      = 0;
      poll_step_timer = now;
      Serial.println("\n=== START POLL CYCLUS ===");
    } else if (live_sec > 0 && now - last_live_cycle >= (unsigned long)live_sec * 1000UL) {
      // v6.2: live-cyclus — enkel ESP32's (HTTP), snel na elkaar, daarna matrix
      last_live_cycle = now;
      polling_active  = true;
      poll_live       = true;
      poll_index      = 0;
      poll_step_timer = now;
    }
    return;
  }
  if (now - poll_step_timer < (poll_live ? 200UL : 2000UL)) return;
  poll_step_timer = now;
  while (poll_index < NUM_CONTROLLERS) {
    if (controllers[poll_index].active) {
      if (controllers[poll_index].type != TYPE_PHOTON &&
          strlen(controllers[poll_index].ip) > 0) {
        pollESP32Controller(poll_index);
        poll_index++;
        return;
      }
      if (poll_live && controllers[poll_index].type == TYPE_PHOTON) {
        poll_index++;                       // v6.2: Photons niet in de live-cyclus
        continue;
      }
      if (controllers[poll_index].type == TYPE_PHOTON &&
          strlen(controllers[poll_index].photon_id) > 0) {
        // Round-robin: tel actieve Photons tot aan deze index
        int photon_seq = 0;
        for (int k = 0; k < poll_index; k++) {
          if (controllers[k].active && controllers[k].type == TYPE_PHOTON
              && strlen(controllers[k].photon_id) > 0) photon_seq++;
        }
        if (photon_seq == photon_round_robin) {
          // Dit is de beurt van deze Photon
          pollPhotonController(poll_index);
        } else {
          Serial.printf("  [Photon] %s overgeslagen (beurt %d, nu %d)\n",
                        controllers[poll_index].name,
                        photon_seq, photon_round_robin);
        }
        poll_index++;
        return;
      }
    }
    poll_index++;
  }
  if (poll_live) {                          // v6.2: live-cyclus klaar
    updateMatrix();                         // stil (geen serial-regel)
    polling_active = false;
    poll_live      = false;
    return;
  }
  // Sheets skip counter ophogen
  sheets_skip_counter++;
  if (sheets_skip_counter >= SHEETS_POST_EVERY) sheets_skip_counter = 0;

  // Round-robin ophogen aan einde van elke cyclus
  int total_photons = 0;
  for (int k = 0; k < NUM_CONTROLLERS; k++) {
    if (controllers[k].active && controllers[k].type == TYPE_PHOTON
        && strlen(controllers[k].photon_id) > 0) total_photons++;
  }
  if (total_photons > 0) {
    photon_round_robin = (photon_round_robin + 1) % total_photons;
  }
  polling_active = false;
  Serial.printf("=== POLL CYCLUS KLAAR | Sheets %d/%d | Photon beurt %d/%d ===\n",
                sheets_skip_counter, SHEETS_POST_EVERY,
                photon_round_robin, total_photons);
  updateMatrix();
}


// ============================================================
// WIFI TESTER
// ============================================================
int wtPing() {
  WiFiClient c;
  unsigned long t = millis();
  bool ok = c.connect(IPAddress(192,168,0,1), 53, 500);
  int ms = ok ? (int)(millis() - t) : -1;
  if (ok) c.stop();
  return ms;
}

void handleWifiJson() {
  char buf[22];
  int r = ap_mode ? 0 : WiFi.RSSI();
  int q = ap_mode ? 0 : constrain(2 * (r + 100), 0, 100);
  snprintf(buf, sizeof(buf), "{\"r\":%d,\"q\":%d}", r, q);
  server.send(200, "application/json", buf);
}

void handleWifiSnap() {
  if (ap_mode) { server.send(200, "application/json", "{\"status\":\"ap\"}"); return; }
  if (server.hasArg("start")) {
    wt_snap_rssi = WiFi.RSSI();
    wt_snap_ping = wtPing();
    if (!wt_scan_pending) { WiFi.scanNetworks(true); wt_scan_pending = true; }
    server.send(200, "application/json", "{\"status\":\"scanning\"}");
    return;
  }
  if (!wt_scan_pending) { server.send(200, "application/json", "{\"status\":\"idle\"}"); return; }
  int n = WiFi.scanComplete();
  if (n == WIFI_SCAN_RUNNING) { server.send(200, "application/json", "{\"status\":\"scanning\"}"); return; }
  String j = "{\"status\":\"ok\",\"rssi\":";
  j += wt_snap_rssi; j += ",\"ping\":"; j += wt_snap_ping; j += ",\"nets\":[";
  String mySsid = WiFi.SSID();
  int found = 0;
  if (n > 0) {
    for (int k = 0; k < n && found < 4; k++) {
      if (WiFi.SSID(k) == mySsid) continue;
      if (found++) j += ",";
      j += "{\"s\":\""; j += WiFi.SSID(k); j += "\",\"r\":"; j += WiFi.RSSI(k); j += "}";
    }
  }
  j += "]}";
  WiFi.scanDelete();
  wt_scan_pending = false;
  server.send(200, "application/json", j);
}

// ============================================================
// STATUSMATRIX — helper functies (v4.0)
// ============================================================
int matPxIdx(int row, int col) {
  if (MATRIX_FLIP_H) col = (MATRIX_WIDTH - 1) - col;
  int phys_row = (MATRIX_HEIGHT - 1) - row;
  if (phys_row % 2 == 0) return phys_row * MATRIX_WIDTH + col;
  else                   return phys_row * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - col);
}

// v6.2: gamma + eigen helderheid + ondergrens. Door de gamma moeten mengkleuren
// "perceptueel" gekozen worden: oranje = (255,140,0), niet (255,70,0). Elke led die "aan" is (r|g|b > 0)
// krijgt minstens MAT_MIN_ZICHTBAAR op zijn sterkste kanaal (kleur blijft behouden).
#define MAT_MIN_ZICHTBAAR 7     // 0-255, na helderheid
#define MAT_RUST_NIVEAU   3     // v6.3: rust-led (grijs), 0-255 na helderheid; 0 = rust uit

void matPx(int row, int col, uint8_t r, uint8_t g, uint8_t b) {
  if (row < 0 || row >= MATRIX_HEIGHT || col < 0 || col >= MATRIX_WIDTH) return;
  if ((r | g | b) == 0) { matrix.setPixelColor(matPxIdx(row, col), 0, 0, 0); return; }
  uint16_t R = (uint16_t)Adafruit_NeoPixel::gamma8(r) * matrix_brightness / 255;
  uint16_t G = (uint16_t)Adafruit_NeoPixel::gamma8(g) * matrix_brightness / 255;
  uint16_t B = (uint16_t)Adafruit_NeoPixel::gamma8(b) * matrix_brightness / 255;
  uint16_t m = max(R, max(G, B));
  if (m < MAT_MIN_ZICHTBAAR) {
    // verhouding van de originele kleur behouden, sterkste kanaal = ondergrens
    uint16_t mo = max(r, max(g, b));
    R = (uint16_t)r * MAT_MIN_ZICHTBAAR / mo;
    G = (uint16_t)g * MAT_MIN_ZICHTBAAR / mo;
    B = (uint16_t)b * MAT_MIN_ZICHTBAAR / mo;
  }
  matrix.setPixelColor(matPxIdx(row, col), (uint8_t)R, (uint8_t)G, (uint8_t)B);
}

// v6.3: rust rechtstreeks op een vast laag niveau (niet via ondergrens/helderheid)
void rustPx(int row, int col) {
  if (row < 0 || row >= MATRIX_HEIGHT || col < 0 || col >= MATRIX_WIDTH) return;
  matrix.setPixelColor(matPxIdx(row, col), MAT_RUST_NIVEAU, MAT_RUST_NIVEAU, MAT_RUST_NIVEAU);
}

// v6.3: intensiteit van één kleur, f = 0..1 (≤ 0 = rust)
void levelPx(int row, int col, float f, uint8_t r, uint8_t g, uint8_t b) {
  if (f <= 0) { rustPx(row, col); return; }
  if (f > 1) f = 1;
  float k = 0.25f + 0.75f * f;            // 25 % … 100 %
  matPx(row, col, (uint8_t)(r * k), (uint8_t)(g * k), (uint8_t)(b * k));
}

// v6.2: continu kleurverloop via HSV. hue_lo → hue_hi (0..65535), x van lo → hi.
void hsvPx(int row, int col, float x, float lo, float hi, uint16_t hue_lo, uint16_t hue_hi) {
  float f = (x - lo) / (hi - lo);
  if (f < 0) f = 0; if (f > 1) f = 1;
  uint16_t hue = (uint16_t)((float)hue_lo + ((float)hue_hi - (float)hue_lo) * f);
  uint32_t c = Adafruit_NeoPixel::ColorHSV(hue, 255, 255);
  matPx(row, col, (c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
}
#define HUE_BLAUW 43690
#define HUE_GROEN 21845
#define HUE_ROOD  0
// diep blauw (lo) → cyaan → groen → geel → fel rood (hi)
void blauwRoodPx(int row, int col, float x, float lo, float hi) { hsvPx(row, col, x, lo, hi, HUE_BLAUW, HUE_ROOD); }
// groen (goed, lo) → geel → rood (slecht, hi)
void groenRoodPx(int row, int col, float x, float lo, float hi) { hsvPx(row, col, x, lo, hi, HUE_GROEN, HUE_ROOD); }

void matRow(int row, uint8_t r, uint8_t g, uint8_t b) {
  for (int c = 0; c < MATRIX_WIDTH; c++) matPx(row, c, r, g, b);
}

void statusPx(int row, int col, int status) {
  switch (status) {
    case STATUS_ONLINE:   matPx(row, col, 0,   200, 0);   break;
    case STATUS_OFFLINE:  matPx(row, col, 200, 0,   0);   break;
    case STATUS_PENDING:  matPx(row, col, 200, 140, 0);   break;
    default:              matPx(row, col, 12,  12,  12);  break;
  }
}

void tempPx(int row, int col, float t, float cold=15.0, float warm=22.0, float hot=28.0) {
  // v6.2: continu — cold = diep blauw, hot = fel rood ("warm" niet meer gebruikt)
  if (t < 1.0 || t > 95.0) { matPx(row, col, 40, 0, 40); return; }
  blauwRoodPx(row, col, t, cold, hot);
}

void boilerTempPx(int row, int col, float t) {
  tempPx(row, col, t, 40.0, 60.0, 75.0);
}

// v6.2: continu — −55 dBm groen → −85 dBm rood
void rssiPx(int row, int col, int rssi) { groenRoodPx(row, col, -rssi, 55, 85); }

// v6.2: continu — ≥ 40 KB groen → ≤ 20 KB rood
void heapPx(int row, int col, float kb) { groenRoodPx(row, col, 40 - kb, 0, 20); }

void cyanLevel(int row, int col, int pct_0_100) {
  if (pct_0_100 <= 0) { rustPx(row, col); return; }
  uint8_t v = (uint8_t)map(pct_0_100, 1, 100, 60, 255);
  matPx(row, col, 0, v/2, v);
}

// v6.2: gemeenschappelijke kolommen room/Photon
void vochtPx(int row, int col, int h) {                 // 40 % groen → 85 % rood
  if (h < 10) { matPx(row, col, 40, 0, 40); return; }
  groenRoodPx(row, col, h, 40, 85);
}
void co2Px(int row, int col, int ppm) {                 // 450 groen → 1500 ppm rood
  if (ppm <= 0) { rustPx(row, col); return; }
  groenRoodPx(row, col, ppm, 450, 1500);
}
void aanPx(int row, int col, bool aan, uint8_t r, uint8_t g, uint8_t b) {
  if (aan) matPx(row, col, r, g, b); else rustPx(row, col);
}

// ============================================================
// JSON key extractor — v5.2: const char* i.p.v. const String&
// Werkt direct op char last_json[] via strstr/atof/atoi.
// Geen String-kopie, geen heap-allocatie.
// ============================================================
static float jF(const char* j, const char* key, float def = 0.0f) {
  char buf[28];
  snprintf(buf, sizeof(buf), "\"%s\":", key);
  const char* pos = strstr(j, buf);
  if (!pos) return def;
  return (float)atof(pos + strlen(buf));
}
static int jI(const char* j, const char* key, int def = 0) {
  char buf[28];
  snprintf(buf, sizeof(buf), "\"%s\":", key);
  const char* pos = strstr(j, buf);
  if (!pos) return def;
  return atoi(pos + strlen(buf));
}

// ============================================================
// MATRIX — ROOM rij renderer (v4.0)
// v5.2: const char* j i.p.v. const String& j
// ============================================================
void renderRoomRow(int row, int ci) {
  const char* j = controllers[ci].last_json;
  bool online    = (controllers[ci].status == STATUS_ONLINE);

  statusPx(row, 0, controllers[ci].status);

  if (!online) {
    for (int c = 1; c < MATRIX_WIDTH; c++) matPx(row, c, 25, 0, 0);
    return;
  }

  // v6.2: rust = zacht grijs, actief = fel, grootheden continu
  // Col 1: HOME — blauw = thuis (legende), weg = rust
  aanPx(row, 1, jI(j, "v"), 0, 60, 255);

  // Col 2: Verwarming AAN — fel oranje
  aanPx(row, 2, jI(j, "b"), 255, 140, 0);

  // Col 3: Temperatuur t.o.v. setpoint (−3 °C blauw … +3 °C rood); zonder setpoint absoluut
  {
    float t  = jF(j, "e", 0.0f);
    float sp = jF(j, "c", 0.0f);
    if (t < 1.0f || t > 80.0f) matPx(row, 3, 40, 0, 40);
    else if (sp > 5.0f)        blauwRoodPx(row, 3, t, sp - 3.0f, sp + 3.0f);
    else                       tempPx(row, 3, t);
  }

  // Col 4: Vochtigheid (40 % groen → 85 % rood)
  vochtPx(row, 4, jI(j, "h"));

  // Col 5: CO2 (450 groen → 1500 ppm rood)
  co2Px(row, 5, jI(j, "k"));

  // Col 6-7: Beweging MOV1 / MOV2 — paars (legende)
  aanPx(row, 6, jI(j, "y"), 200, 60, 255);
  aanPx(row, 7, jI(j, "z"), 200, 60, 255);

  // Col 8: TSTAT — fel oranje
  aanPx(row, 8, jI(j, "d"), 255, 140, 0);

  // Col 9: Dauwpunt-alert — geel (legende), anders rust
  aanPx(row, 9, jI(j, "j"), 255, 200, 0);

  // Col 10: NeoPixel kamerkleur (volle kleur)
  {
    int pr = jI(j, "q"), pg = jI(j, "r"), pb = jI(j, "s");
    if (pr + pg + pb > 0) matPx(row, 10, (uint8_t)pr, (uint8_t)pg, (uint8_t)pb);
    else                  rustPx(row, 10);
  }

  // Col 11: Dag geel / Nacht paars
  if (jI(j, "o")) matPx(row, 11, 110, 0, 255);
  else            matPx(row, 11, 255, 200, 0);

  // Col 12: LDR omgevingslicht — continu (donker = rust, licht = fel geel)
  //         (room-sketch: oude schaal, 100 = donker)
  {
    int lv = 100 - constrain(jI(j, "m"), 0, 100);          // 0 = donker, 100 = licht
    if (lv <= 2) rustPx(row, 12);
    else matPx(row, 12, (uint8_t)(lv * 2.55f), (uint8_t)(lv * 2.0f), 0);
  }

  // Col 13: Aandeel actieve pixels (key "t" = "P=00111") — continu wit
  {
    int pct = -1;
    const char* eq = strstr(j, "\"t\":\"");
    if (eq) {
      const char* start = eq + 5;
      const char* end   = strchr(start, '"');
      if (end && end > start) {
        const char* sep = strchr(start, '=');
        if (sep && sep < end) {
          sep++;
          int total = (int)(end - sep), ones = 0;
          for (const char* q = sep; q < end; q++) if (*q == '1') ones++;
          if (total > 0) pct = ones * 100 / total;
        }
      }
    }
    if (pct <= 0) rustPx(row, 13);
    else { uint8_t v = (uint8_t)(60 + pct * 195 / 100); matPx(row, 13, v, v, v); }
  }

  // Col 14: Heap largest block KB
  heapPx(row, 14, jF(j, "ae", 0.0f));

  // Col 15: WiFi RSSI
  rssiPx(row, 15, jI(j, "ac", -100));
}

// ============================================================
// MATRIX — PHOTON rij renderer (v4.7)
// v5.2: const char* j i.p.v. const String& j
// ============================================================
void renderPhotonRow(int row, int ci) {
  const char* j = controllers[ci].last_json;
  bool online    = (controllers[ci].status == STATUS_ONLINE);

  statusPx(row, 0, controllers[ci].status);

  if (!online) {
    for (int c = 1; c < MATRIX_WIDTH; c++) matPx(row, c, 25, 0, 0);
    return;
  }

  // Col 1: niet beschikbaar
  matPx(row, 1, 0, 0, 0);

  // v6.2: zelfde kleurtaal als de ESP32-rooms
  // Col 2: TSTATon (key "l")
  aanPx(row, 2, jI(j, "l"), 255, 140, 0);

  // Col 3: Temp1 DHT22 (key "g") — absoluut 15-28 °C (Photon kent geen setpoint-key)
  tempPx(row, 3, jF(j, "g", 0.0f));

  // Col 4: Vochtigheid (key "d")
  vochtPx(row, 4, jI(j, "d"));

  // Col 5: CO2 (key "a")
  co2Px(row, 5, jI(j, "a"));

  // Col 6-7: MOV1 / MOV2 (keys "i", "j")
  aanPx(row, 6, jI(j, "i"), 200, 60, 255);
  aanPx(row, 7, jI(j, "j"), 200, 60, 255);

  // Col 8: niet beschikbaar
  matPx(row, 8, 0, 0, 0);

  // Col 9: DewAlert (key "k")
  aanPx(row, 9, jI(j, "k"), 255, 200, 0);

  // Col 10: RGB pixels (keys "s","t","u")
  {
    int pr = jI(j, "s"), pg = jI(j, "t"), pb = jI(j, "u");
    if (pr + pg + pb > 0) matPx(row, 10, (uint8_t)pr, (uint8_t)pg, (uint8_t)pb);
    else                  rustPx(row, 10);
  }

  // Col 11: Night (key "q")
  if (jI(j, "q")) matPx(row, 11, 110, 0, 255);
  else            matPx(row, 11, 255, 200, 0);

  // Col 12: LDR licht (key "e", 100 = donker)
  {
    int lv = 100 - constrain(jI(j, "e"), 0, 100);
    if (lv <= 2) rustPx(row, 12);
    else matPx(row, 12, (uint8_t)(lv * 2.55f), (uint8_t)(lv * 2.0f), 0);
  }

  // Col 13: niet beschikbaar
  matPx(row, 13, 0, 0, 0);

  // Col 14: FreeMem% (key "x")
  groenRoodPx(row, 14, 40 - jI(j, "x"), 0, 25);   // v6.2: ≥ 40 % groen → ≤ 15 % rood

  // Col 15: RSSI niet beschikbaar
  matPx(row, 15, 0, 0, 0);
}

// ============================================================
// MATRIX — ACCESS rij renderer (v6.4): de 4 buitenmuren W·Z·N·O
// Keys (JSON_status van R8-ACCESS): mw1/mw2/ms = beweging laatste 10 min,
// lw1/lw2/ls = licht aan, au = automatisch (PIR).
// ============================================================
void accessStatusPx(int row, int col, int ci, bool online, const char* j) {
  if (!online)           statusPx(row, col, controllers[ci].status);   // rood / geel
  else if (jI(j, "au", 1)) matPx(row, col, 0,   200, 0);                // automatisch
  else                   matPx(row, col, 255, 120, 0);                 // manueel
}

void renderAccessRow(int row) {
  // N en O: muren zonder controller → alleen een dimme statusled
  statusPx(row, 8,  STATUS_INACTIVE);
  statusPx(row, 12, STATUS_INACTIVE);

  int ci = -1;
  if (controllers[IDX_S_ACCESS].active && strlen(controllers[IDX_S_ACCESS].last_json) > 5) ci = IDX_S_ACCESS;
  else if (controllers[IDX_P_ACCESS].active) ci = IDX_P_ACCESS;
  if (ci < 0) {                                   // geen bron actief
    statusPx(row, 0, STATUS_INACTIVE);
    statusPx(row, 4, STATUS_INACTIVE);
    return;
  }

  const char* j = controllers[ci].last_json;
  bool online = (controllers[ci].status == STATUS_ONLINE) && strlen(j) > 5;

  // W (kol 0-3) en Z (kol 4-7) worden door dezelfde controller bediend
  accessStatusPx(row, 0, ci, online, j);
  accessStatusPx(row, 4, ci, online, j);
  if (!online) return;

  aanPx(row, 1, jI(j, "mw1") || jI(j, "mw2"), 200, 60, 255);   // PIR W
  aanPx(row, 2, jI(j, "lw1"), 255, 180, 60);                    // licht W1 Voordeur
  aanPx(row, 3, jI(j, "lw2"), 255, 180, 60);                    // licht W2 Veldkant
  aanPx(row, 5, jI(j, "ms"),  200, 60, 255);                    // PIR S
  aanPx(row, 6, jI(j, "ls"),  255, 180, 60);                    // licht S Terras
  // kol 7: vrij (uit)
}

// ============================================================
// MATRIX — ECO rij renderer (v4.5)
// v5.2: const char* j i.p.v. const String& j
// ============================================================
void renderEcoRow(int row, int ci) {
  const char* j = controllers[ci].last_json;
  bool online    = (controllers[ci].status == STATUS_ONLINE);

  statusPx(row, 0, controllers[ci].status);

  if (!online) {
    for (int c = 1; c < MATRIX_WIDTH; c++) matPx(row, c, 25, 0, 0);
    return;
  }

  // v6.3: continu
  // Col 1: Tsun collector temp — 0 °C blauw → 90 °C rood
  {
    float tsun = jF(j, "l", 0.0f);
    if (tsun < 1.0f) rustPx(row, 1);
    else             blauwRoodPx(row, 1, tsun, 0.0f, 90.0f);
  }

  // Col 2: dT rendement — groen, intensiteit 0-15 K
  levelPx(row, 2, jF(j, "m", 0.0f) / 15.0f, 0, 255, 0);

  // Col 3-8: zes boilertemperaturen
  boilerTempPx(row, 3, jF(j, "b", 0.0f));
  boilerTempPx(row, 4, jF(j, "c", 0.0f));
  boilerTempPx(row, 5, jF(j, "d", 0.0f));
  boilerTempPx(row, 6, jF(j, "e", 0.0f));
  boilerTempPx(row, 7, jF(j, "f", 0.0f));
  boilerTempPx(row, 8, jF(j, "g", 0.0f));

  // Col 9: EAv gemiddelde boilertemp
  boilerTempPx(row, 9, jF(j, "h", 0.0f));

  // Col 10: PWM pomp
  cyanLevel(row, 10, jI(j, "n") * 100 / 255);

  // Col 11: yield_today kWh — geel-groen, 0-10 kWh
  levelPx(row, 11, jF(j, "k", 0.0f) / 10.0f, 170, 255, 0);

  // Col 12: EQtot energie-inhoud — amber, 0-20 kWh
  levelPx(row, 12, jF(j, "i", 0.0f) / 20.0f, 255, 170, 0);

  // Col 13: dEQ delta kWh — groen, 0-2 kWh
  levelPx(row, 13, jF(j, "j", 0.0f) / 2.0f, 0, 255, 0);

  // Col 14: FreeHeap% — ≥ 40 % groen → ≤ 15 % rood
  groenRoodPx(row, 14, 40 - jI(j, "q"), 0, 25);

  // Col 15: RSSI — ⚠️ key "p" voor ECO (niet "ac"!)
  rssiPx(row, 15, jI(j, "p", -100));
}

// ============================================================
// MATRIX — ENERGY rij renderer (v5.8)
// S-ENERGY controller (192.168.0.73) — /json v1.26
//
// Col  0: Status pixel (groen/geel/rood/grijs)
// Col  1: Solar W         — groen gradient (0–6000W)
// Col  2: SCH afname W    — rood gradient  (0–10000W)
// Col  3: SCH injectie W  — cyaan gradient (0–6000W)
// Col  4: Netto SCH       — groen=injectie / rood=afname
// Col  5: WON W           — amber gradient (0–5000W, P1)
// Col  6: Solar dag kWh   — geel-groen (0–30 kWh)
// Col  7: SCH afname dag  — rood dim  (0–30 kWh)
// Col  8: SCH injectie dag— cyaan dim (0–15 kWh)
// Col  9: EPEX nu ct/kWh  — groen(<15)/geel(<25)/rood(>25)
// Col 10: EPEX +1u        — zelfde kleurschaal
// Col 11: Maandpiek %     — groen(<60%)/amber(<85%)/rood
// Col 12: sim_s0 vlag     — oranje=SIM / groen=LIVE
// Col 13: sim_p1 vlag     — oranje=SIM / groen=LIVE
// Col 14: Heap largest KB — groen/amber/rood
// Col 15: WiFi RSSI       — groen/amber/rood
// ============================================================
void renderEnergyRow(int row, int ci) {
  const char* j = controllers[ci].last_json;
  bool online    = (controllers[ci].status == STATUS_ONLINE);

  statusPx(row, 0, controllers[ci].status);

  if (!online) {
    for (int c = 1; c < MATRIX_WIDTH; c++) matPx(row, c, 25, 0, 0);
    return;
  }

  // v6.3: intensiteit (0 = rust) en continue schalen
  // Col 1: Solar W — groen (0-6000 W)
  levelPx(row, 1, jI(j, "a") / 6000.0f, 0, 255, 0);
  // Col 2: SCH afname W — rood (0-10000 W)
  levelPx(row, 2, jI(j, "c") / 10000.0f, 255, 0, 0);
  // Col 3: SCH injectie W — cyaan (0-6000 W)
  levelPx(row, 3, jI(j, "d") / 6000.0f, 0, 160, 255);
  // Col 4: Netto SCH — groen = injectie, rood = afname (0-5000 W)
  {
    int netto = jI(j, "e");
    if (netto > 0)      levelPx(row, 4,  netto / 5000.0f, 0, 255, 0);
    else if (netto < 0) levelPx(row, 4, -netto / 5000.0f, 255, 0, 0);
    else                rustPx(row, 4);
  }
  // Col 5: WON W — amber (0-5000 W)
  levelPx(row, 5, abs(jI(j, "b")) / 5000.0f, 255, 170, 0);
  // Col 6: Solar dag — geel-groen (0-30 kWh)
  levelPx(row, 6, jI(j, "h") / 30000.0f, 170, 255, 0);
  // Col 7: SCH afname dag — rood (0-30 kWh)
  levelPx(row, 7, jI(j, "j") / 30000.0f, 255, 0, 0);
  // Col 8: SCH injectie dag — cyaan (0-15 kWh)
  levelPx(row, 8, jI(j, "k") / 15000.0f, 0, 160, 255);
  // Col 9-10: EPEX nu / +1u — 5 ct groen → 35 ct rood, negatief cyaan
  {
    float ct = jF(j, "n") / 100.0f;
    if (ct < 0) matPx(row, 9, 0, 200, 255); else groenRoodPx(row, 9, ct, 5, 35);
    ct = jF(j, "n2") / 100.0f;
    if (ct < 0) matPx(row, 10, 0, 200, 255); else groenRoodPx(row, 10, ct, 5, 35);
  }
  // Col 11: Maandpiek — 40 % groen → 100 % rood (schaal 10 kW)
  groenRoodPx(row, 11, jI(j, "pt") / 100.0f, 40, 100);

  // Col 12: sim_s0 — oranje=SIM actief / groen=live hardware
  {
    int s = jI(j, "sim_s0");
    if (s) matPx(row, 12, 180, 80, 0);   // oranje: S0 gesimuleerd ⚠️
    else   matPx(row, 12,   0, 120, 0);  // groen: live S0 pulsen ✅
  }

  // Col 13: sim_p1 — oranje=SIM actief / groen=live P1-dongle
  {
    int s = jI(j, "sim_p1");
    if (s) matPx(row, 13, 180, 80, 0);   // oranje: P1 gesimuleerd ⚠️
    else   matPx(row, 13,   0, 120, 0);  // groen: live HomeWizard P1 ✅
  }

  // Col 14: Heap largest block KB (ae = bytes)
  heapPx(row, 14, jF(j, "ae") / 1024.0f);

  // Col 15: WiFi RSSI (ac = dBm)
  rssiPx(row, 15, jI(j, "ac", -100));
}

// ============================================================
// MATRIX — HVAC rij renderer (v4.1)
// v5.2: const char* j i.p.v. const String& j
// ============================================================
void renderHvacRow(int row, int ci) {
  const char* j = controllers[ci].last_json;
  bool online    = (controllers[ci].status == STATUS_ONLINE);

  statusPx(row, 0, controllers[ci].status);

  if (!online) {
    for (int c = 1; c < MATRIX_WIDTH; c++) matPx(row, c, 25, 0, 0);
    return;
  }

  // Circuits 1-7
  const char* ckeys[7] = {
    HVAC_KEY_CIRCUIT1, HVAC_KEY_CIRCUIT2, HVAC_KEY_CIRCUIT3,
    HVAC_KEY_CIRCUIT4, HVAC_KEY_CIRCUIT5, HVAC_KEY_CIRCUIT6,
    HVAC_KEY_CIRCUIT7
  };
  for (int k = 0; k < 7; k++) {
    int on = jI(j, ckeys[k]);
    aanPx(row, 1+k, on, 0, 255, 40);            // v6.2: aan fel groen, uit rust
  }

  // Col 8: Pomp SCH
  int psch = jI(j, HVAC_KEY_PUMP_SCH);
  aanPx(row, 8, psch, 0, 80, 255);

  // Col 9: Pomp WON
  int pwon = jI(j, HVAC_KEY_PUMP_WON);
  aanPx(row, 9, pwon, 0, 80, 255);

  // Col 10: Ventilatie %
  cyanLevel(row, 10, jI(j, HVAC_KEY_VENT));

  // Col 11: KSAv gemiddelde boilertemp
  boilerTempPx(row, 11, jF(j, HVAC_KEY_TEMP1, 0.0f));

  // Col 12-13: gereserveerd
  matPx(row, 12, 0, 0, 0);   // v6.3: gereserveerd = uit
  matPx(row, 13, 0, 0, 0);

  // Col 14: Heap largest block KB
  heapPx(row, 14, jF(j, HVAC_KEY_HEAP, 0.0f));

  // Col 15: RSSI
  rssiPx(row, 15, jI(j, HVAC_KEY_RSSI, -100));
}

// ============================================================
// MATRIX — updateMatrix() (v5.0)
// v5.2: strlen() i.p.v. last_json.length()
// ============================================================
void updateMatrix() {
  matrix.clear();

  for (int row = 0; row < MATRIX_HEIGHT; row++) {
    const MatrixRowDef& rd = MROW[row];

    if (rd.sys_idx == -2) continue;
    if (row == ACCESS_ROW) { renderAccessRow(row); continue; }   // v6.4
    if (rd.sys_idx == -1 && rd.esp_idx == -1 && rd.photon_idx == -1) continue;
    if (rd.sys_idx >= 0) {
      if (!controllers[rd.sys_idx].active) continue;
      if      (strcmp(controllers[rd.sys_idx].name, "S-ECO")    == 0) renderEcoRow(row,    rd.sys_idx);
      else if (strcmp(controllers[rd.sys_idx].name, "S-HVAC")   == 0) renderHvacRow(row,   rd.sys_idx);
      else if (strcmp(controllers[rd.sys_idx].name, "S-ENERGY") == 0) renderEnergyRow(row, rd.sys_idx);
      else { statusPx(row, 0, controllers[rd.sys_idx].status); }
      continue;
    }

    bool esp_ok = (rd.esp_idx >= 0)
               && controllers[rd.esp_idx].active
               && strlen(controllers[rd.esp_idx].last_json) > 5;

    if (esp_ok) {
      renderRoomRow(row, rd.esp_idx);
      continue;
    }

    bool pho_ok = (rd.photon_idx >= 0)
               && controllers[rd.photon_idx].active
               && strlen(controllers[rd.photon_idx].last_json) > 5;

    if (pho_ok) {
      renderPhotonRow(row, rd.photon_idx);
      continue;
    }

    if (rd.esp_idx >= 0 && controllers[rd.esp_idx].active) {
      statusPx(row, 0, controllers[rd.esp_idx].status);
      continue;
    }
  }

  matrix.show();
  last_matrix_update = millis();
  if (!poll_live) Serial.printf("[Matrix] Bijgewerkt | heap largest: %d bytes\n",
                heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
}

// ============================================================
// MATRIX — boot-animatie (v4.2)
// ============================================================
void matrixBootAnimation() {
  matrix.clear();
  matrix.show();
  delay(150);

  const int sys_rows[] = {0, 1};
  for (int ri = 0; ri < 2; ri++) {
    int row = sys_rows[ri];
    for (int c = 0; c < MATRIX_WIDTH; c++) {
      matrix.clear();
      matPx(row, c, 220, 0, 0);
      if (c >= 1) matPx(row, c-1, 60, 0, 0);
      if (c >= 2) matPx(row, c-2, 20, 0, 0);
      matrix.show();
      delay(28);
    }
    matrix.clear();
    matrix.show();
    delay(80);
  }

  delay(200);

  const int room_rows[] = {5, 6, 7, 8, 9, 10, 11};
  for (int ri = 0; ri < 7; ri++) {
    int row = room_rows[ri];
    for (int c = 0; c < MATRIX_WIDTH; c++) {
      matrix.clear();
      matPx(row, c, 0, 0, 220);
      if (c >= 1) matPx(row, c-1, 0, 0, 60);
      if (c >= 2) matPx(row, c-2, 0, 0, 20);
      matrix.show();
      delay(28);
    }
    matrix.clear();
    matrix.show();
    delay(60);
  }

  for (int c = 0; c < MATRIX_WIDTH; c++) {
    matPx(0, c, 30, 30, 30);
    matPx(1, c, 30, 30, 30);
    for (int r = 5; r <= 11; r++) matPx(r, c, 30, 30, 30);
  }
  matrix.show();
  delay(250);
  matrix.clear();
  matrix.show();
  Serial.println(F("[Matrix] Boot-animatie klaar"));
}

// ============================================================
// MATRIX — testpatroon (v4.0)
// ============================================================
void matrixTestPattern() {
  matrix.clear();

  const uint8_t rowcols[12][3] = {
    {200, 0,   0  }, // rij 0: HVAC → rood
    {200, 80,  0  }, // rij 1: ECO  → oranje
    {80,  0,   80 }, // rij 2: ENERGY  → paars
    {80,  0,   80 }, // rij 3: ACCESS  → paars
    {60,  30,  0  }, // rij 4: separator → amber
    {0,   200, 0  }, // rij 5: R-BandB → groen
    {0,   180, 20 }, // rij 6: R-BADK
    {0,   160, 40 }, // rij 7: R-INKOM
    {0,   140, 60 }, // rij 8: R-KEUKEN
    {0,   120, 80 }, // rij 9: R-WASPL
    {0,   100, 100}, // rij 10: R-EETPL
    {0,   80,  120}, // rij 11: R-ZITPL
  };

  for (int row = 0; row < 12; row++) {
    for (int c = 0; c < MATRIX_WIDTH; c++) {
      uint8_t r = rowcols[row][0], g = rowcols[row][1], b = rowcols[row][2];
      if (c == 0 || c == MATRIX_WIDTH-1) matPx(row, c, 200, 200, 200);
      else                               matPx(row, c, r/3, g/3, b/3);
    }
  }
  matPx(0, 0,  255, 255, 255);
  matPx(0, 15, 255, 255, 255);
  matPx(11,0,  255, 255, 255);
  matPx(11,15, 255, 255, 255);

  matrix.show();
  Serial.println("[Matrix] Testpatroon actief — controleer orientatie!");
  Serial.println("  Rij 0 (rood)   = HVAC = BOVENSTE rij matrix");
  Serial.println("  Rij 11 (blauw) = R-ZITPL = ONDERSTE aktieve rij");
  Serial.println("  Hoekpixels wit = voor orientatiecheck");
  Serial.println("  Na verificatie: normaal updaten via updateMatrix()");
}

// ============================================================
// WIFI TESTER — HTML pagina
// ============================================================
String getWifiPage() {
  String h = F("<!DOCTYPE html><html lang='nl'><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Zarlar WiFi</title>"
    "<style>"
    "body{margin:0;background:#111;color:#eee;font-family:monospace;font-size:13px}"
    ".hdr{background:#1a1200;border-bottom:2px solid #f0a500;padding:10px 16px}"
    ".hdr-t{color:#f0a500;font-size:18px;letter-spacing:2px}"
    ".nav{background:#161b22;padding:8px 16px;display:flex;gap:6px;flex-wrap:wrap;"
         "border-bottom:1px solid #333}"
    ".nav a{color:#888;text-decoration:none;padding:4px 10px;border:1px solid #333;"
            "border-radius:3px;font-size:11px}"
    ".nav a.cur,.nav a:hover{color:#f0a500;border-color:#f0a500}"
    ".wrap{padding:14px 16px}"
    "#big{font-size:18pt;font-weight:bold;text-align:center;padding:14px 0 4px}"
    ".bw{margin:0 0 16px;background:#222;border-radius:3px;height:8px}"
    ".bi{height:8px;border-radius:3px;transition:width .5s,background .5s}"
    "table{border-collapse:collapse;width:100%}"
    "tr{border-bottom:1px solid #1a1a1a}"
    "td{padding:5px 6px;font-size:12px;vertical-align:middle}"
    ".cn{cursor:pointer;color:#aaa;white-space:nowrap;width:110px}"
    ".cn:hover{color:#f0a500}"
    ".g{color:#2ecc40}.y{color:#f0c040}.o{color:#e05a00}.r{color:#e74c3c}"
    ".dim{color:#444}"
    "</style></head><body>"
    "<div class='hdr'><span class='hdr-t'>⬡ ZARLAR</span></div>"
    "<div class='nav'>"
    "<a href='/'>← Dashboard</a>"
    "<a href='/settings'>⚙ Settings</a>"
    "<a href='/wifi' class='cur'>📶 WiFi</a></div>"
    "<div class='wrap'>"
    "<div id='big'>—</div>"
    "<div class='bw'><div class='bi' id='bar' style='width:0'></div></div>"
    "<table id='tbl'></table>"
    "</div>"
    "<script>"
    "var C=");
  h += "[";
  bool first = true;
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    if (controllers[i].type == TYPE_PHOTON) continue;
    if (!first) h += ",";
    h += "{i:"; h += i; h += ",n:\""; h += controllers[i].name; h += "\"}";
    first = false;
  }
  h += "];";
  h += F(
    "var S={};"
    "function rc(r){return r>=-60?'g':r>=-70?'y':r>=-80?'o':'r';}"
    "function pc(p){return p<0?'r':p<8?'g':p<20?'y':p<40?'o':'r';}"
    "var QC=['#2ecc40','#f0c040','#e05a00','#e74c3c'];"
    "function build(){var h='';C.forEach(function(c){"
    "  var d=S[c.i];"
    "  var sd='<span class=\"dim\">—</span>';"
    "  if(d===null)sd='<span class=\"dim\">…</span>';"
    "  else if(d){sd='<span class=\"'+rc(d.rssi)+'\">'+d.rssi+'</span>';"
    "    var ps=d.ping>=0?'<span class=\"'+pc(d.ping)+'\">'+d.ping+'</span>':'<span class=\"r\">—</span>';"
    "    sd+=' '+ps;"
    "    d.nets.forEach(function(n){sd+=' <span style=\"color:#ccc\">'+n.s+'</span><span class=\"'+rc(n.r)+'\"> '+n.r+'</span>';});}"
    "  h+='<tr><td class=\"cn\" onclick=\"tog('+c.i+')\">'+c.n+'</td><td>'+sd+'</td></tr>';"
    "});document.getElementById('tbl').innerHTML=h;}"
    "function tog(idx){if(S[idx]!==undefined){delete S[idx];build();return;}"
    "  S[idx]=null;build();"
    "  fetch('/wifi_snap?start=1').then(r=>r.json()).then(d=>{"
    "    if(S[idx]===undefined)return;"
    "    if(d.status==='scanning')poll(idx);else done(idx,d);});}"
    "function poll(idx){setTimeout(function(){"
    "    if(S[idx]===undefined)return;"
    "    fetch('/wifi_snap').then(r=>r.json()).then(d=>{"
    "      if(S[idx]===undefined)return;"
    "      if(d.status==='scanning')poll(idx);else done(idx,d);});},600);}"
    "function done(idx,d){if(S[idx]===undefined)return;"
    "  if(d.status==='ok')S[idx]={rssi:d.rssi,ping:d.ping,nets:d.nets||[]};"
    "  else delete S[idx];build();}"
    "function updR(){fetch('/wifi_json').then(r=>r.json()).then(d=>{"
    "    var ci=d.q>=70?0:d.q>=40?1:d.q>=20?2:3;var c=QC[ci];"
    "    var el=document.getElementById('big');el.textContent=d.r+' dBm';el.style.color=c;"
    "    var b=document.getElementById('bar');b.style.width=d.q+'%';b.style.background=c;"
    "  }).catch(function(){})}"
    "build();setInterval(updR,1000);updR();"
    "</script></body></html>");
  return h;
}

// ============================================================
// STATUS JSON
// v5.4: static char buf[2400] in BSS — nul heap-allocatie.
// Wordt elke 15s door de browser opgeroepen → was de meest
// frequente heap alloc/free cyclus, veroorzaakte fragmentatie.
// ============================================================
void handleStatusJson() {
  static char buf[2400];  // BSS — vaste allocatie, nooit heap
  int pos = 0;
  pos += snprintf(buf + pos, sizeof(buf) - pos, "[");
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    int rssi = 0;
    if (controllers[i].type != TYPE_PHOTON && controllers[i].last_json[0] != '\0') {
      const char* key = (strcmp(controllers[i].name, "S-ECO") == 0) ? "\"p\":" : "\"ac\":";
      const char* p = strstr(controllers[i].last_json, key);
      if (p) rssi = atoi(p + strlen(key));
    }
    pos += snprintf(buf + pos, sizeof(buf) - pos,
      "%s{\"n\":\"%s\",\"t\":%d,\"ip\":\"%s\",\"pid\":\"%s\",\"a\":%d,\"s\":%d,\"r\":%d}",
      i > 0 ? "," : "",
      controllers[i].name, controllers[i].type,
      controllers[i].ip, controllers[i].photon_id,
      controllers[i].active ? 1 : 0,
      controllers[i].status, rssi);
  }
  snprintf(buf + pos, sizeof(buf) - pos, "]");
  server.send(200, "application/json", buf);
}

// ============================================================
// HOOFDPAGINA
// ============================================================
String getMainPage() {
  String h = F("<!DOCTYPE html><html lang='nl'><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Zarlar</title>"
    "<style>"
    "body{margin:0;background:#111;color:#eee;font-family:monospace;font-size:14px;-webkit-text-size-adjust:none}"
    ".hdr{background:#1a1200;border-bottom:2px solid #f0a500;padding:10px 16px;"
         "display:flex;align-items:center;justify-content:space-between}"
    ".hdr-t{color:#f0a500;font-size:18px;letter-spacing:2px}"
    ".hdr-r{color:#888;font-size:11px}"
    ".nav{background:#161b22;padding:8px 16px;display:flex;gap:6px;flex-wrap:wrap;"
         "border-bottom:1px solid #333}"
    ".nav a{color:#888;text-decoration:none;padding:4px 10px;border:1px solid #333;"
            "border-radius:3px;font-size:11px}"
    ".nav a:hover{color:#f0a500;border-color:#f0a500}"
    ".wrap{padding:14px 16px;max-width:1200px}"
    ".sec{background:#161b22;border:1px solid #333;border-radius:6px;margin-bottom:14px}"
    ".sec-t{padding:8px 14px;font-size:11px;color:#f0a500;letter-spacing:2px;"
            "border-bottom:1px solid #333;text-transform:uppercase}"
    ".grid{display:flex;flex-wrap:wrap;gap:7px;padding:12px 14px}"
    ".btn{display:flex;align-items:center;gap:7px;padding:7px 12px;"
         "background:#1c2128;border:1px solid #333;border-radius:5px;"
         "cursor:pointer;font-family:monospace;font-size:12px;color:#ccc;"
         "min-width:150px;transition:border-color .15s}"
    ".btn:hover{border-color:#f0a500;color:#f0a500}"
    ".btn.off{opacity:.4;cursor:default}"
    ".btn.off:hover{border-color:#333;color:#ccc}"
    ".dot{width:8px;height:8px;border-radius:50%;flex-shrink:0}"
    "#portal{padding:0}"
    "#portal iframe{width:100%;height:calc(100vh - 84px);border:none;display:block;background:#fff}"
    "#portal .empty{padding:40px;text-align:center;color:#555}"
    "@media(max-width:600px){.btn{min-width:100px}}"
    ".hbtn{padding:4px 10px;border-radius:3px;font-size:11px;cursor:pointer;font-family:monospace}"
    ".hbtn.on{background:#1a3a1a;color:#2ecc40;border:1px solid #2ecc40}"
    ".hbtn.off{background:#1c2128;color:#666;border:1px solid #444}"
    "</style></head><body>"
    "<div class='hdr'><span class='hdr-t'>⬡ ZARLAR</span>"
    "<span class='hdr-r' id='clk'>192.168.0.60</span></div>"
    "<div class='nav'>"
    "<a href='/'>Dashboard</a>"
    "<a href='/settings'>⚙ Settings</a>"
    "<a href='/wifi'>📶 WiFi</a>"
    "<button id='hb' class='hbtn ");
  h += home_mode_global ? "on' onclick='setH(0)'>● HOME" : "off' onclick='setH(1)'>○ UIT";
  h += F("</button></div>"
    "<div class='wrap'>"
    "<div class='sec'><div class='sec-t'>▸ Systeem</div>"
    "<div class='grid' id='gs'></div></div>"
    "<div class='sec'><div class='sec-t'>▸ Rooms</div>"
    "<div class='grid' id='gr'></div></div>"
    "<div class='sec'><div class='sec-t'>☁ Photon</div>"
    "<div class='grid' id='gp'></div></div>"
    "<div class='sec'><div class='sec-t' id='pt'>▸ Portal</div>"
    "<div id='portal'><div class='empty'>Kies een controller</div></div></div>"
    "</div>"
    "<script>"
    "var C=[];"
    "var PL={'datum':'Time','a':'CO2','b':'Dust','c':'Dew','d':'Humi','e':'Light',"
    "'f':'SUNLight','g':'Temp1','h':'Temp2','i':'MOV1','j':'MOV2',"
    "'k':'DewAlert','l':'TSTATon','m':'MOV1light','n':'MOV2light',"
    "'o':'BEAMvalue','p':'BEAMalert','q':'Night','r':'Bed','s':'R',"
    "'t':'G','u':'B','v':'Strength','w':'Quality','x':'FreeMem'};"
    "var SC=['#555','#f0c040','#2ecc40','#e74c3c'];"
    "function rc(r){return r>=-60?'#2ecc40':r>=-70?'#f0c040':r>=-80?'#e05a00':'#e74c3c';}"
    "function dot(s){return '<span class=\"dot\" style=\"background:'+SC[s]+';box-shadow:0 0 5px '+SC[s]+';\"></span>';}"
    "function build(){"
    "  var gs=document.getElementById('gs');"
    "  var gr=document.getElementById('gr');"
    "  var gp=document.getElementById('gp');"
    "  gs.innerHTML=gr.innerHTML=gp.innerHTML='';"
    "  C.forEach(function(c,i){"
    "    var b=document.createElement('button');"
    "    b.className='btn'+(c.a?'':' off');"
    "    b.id='b'+i;"
    "    var rs=c.r!==0?'<span class=\"rs\" style=\"color:'+rc(c.r)+';font-size:11px\">'+Math.abs(c.r)+'</span>':'';"
    "    b.innerHTML=dot(c.s)+c.n+rs;"
    "    b.title=c.ip||c.pid||'';"
    "    if(c.a)b.onclick=function(){open(i);};"
    "    if(c.t===0)gp.appendChild(b);"
    "    else if(c.t===1)gs.appendChild(b);"
    "    else gr.appendChild(b);"
    "  });"
    "}"
    "function updDots(){"
    "  C.forEach(function(c,i){"
    "    var b=document.getElementById('b'+i);"
    "    if(!b)return;"
    "    var d=b.querySelector('.dot');"
    "    if(d)d.style.background=SC[c.s];"
    "    var rs=b.querySelector('.rs');"
    "    if(rs&&c.r!==0){rs.textContent=Math.abs(c.r);rs.style.color=rc(c.r);}"
    "  });"
    "}"
    "function open(i){"
    "  var c=C[i];"
    "  var pu=c.t===0?'':`http://${c.ip}/`;"
    "  document.getElementById('pt').innerHTML=pu?"
    "    `<a href='${pu}' target='_blank' style='color:#f0a500;text-decoration:none'>▸ ${c.n} ↗</a>`:"
    "    '▸ '+c.n;"
    "  var p=document.getElementById('portal');"
    "  if(c.t===0){"
    "    p.innerHTML='<div class=\"empty\" style=\"color:#f0c040\">⏳ Laden '+c.n+'...</div>';"
    "    fetchPhoton(i,c.pid,c.n);"
    "  } else {"
    "    p.innerHTML='<iframe src=\"http://'+c.ip+'/\" title=\"'+c.n+'\"></iframe>';"
    "  }"
    "  document.querySelector('.sec:last-child').scrollIntoView({behavior:'smooth'});"
    "}"
    "async function fetchPhoton(i,id,name){"
    "  var p=document.getElementById('portal');"
    "  p.innerHTML='<div class=\"empty\" style=\"color:#f0c040\">&#x23F3; Laden '+name+'...</div>';"
    "  try{"
    "    var r=await fetch('https://controllers-diagnose.filip-delannoy.workers.dev/sensor?id='+id,{cache:'no-store'});"
    "    if(!r.ok)throw new Error('HTTP '+r.status);"
    "    var obj=await r.json();"
    "    if(!obj.online)throw new Error('Photon offline');"
    "    C[i].s=2;updDots();"
    "    var h='<div style=\"padding:14px;color:#2ecc40\">&#x2713; '+name+' online ('+obj.last_min+'m geleden)</div>';"
    "    h+='<div style=\"padding:0 14px 14px\">';"
    "    for(var k in obj){if(k==='online'||k==='last_min')continue;"
    "      var v=obj[k];var l=PL[k]||k;"
    "      h+='<div style=\"display:flex;justify-content:space-between;padding:5px 0;"
    "border-bottom:1px solid #333\"><span style=\"color:#f0a500\">'+l+'</span>"
    "<span>'+v+'</span></div>';}"
    "    h+='</div>';"
    "    p.innerHTML=h;"
    "  }catch(e){C[i].s=3;updDots();"
    "    p.innerHTML='<div class=\"empty\" style=\"color:#e74c3c\">&#x274C; '+e.message+'</div>';}"
    "}"
    "function pollStatus(){"
    "  fetch('/status_json').then(function(r){return r.json();})"
    "  .then(function(d){C=d;updDots();}).catch(function(){});"
    "}"
    "fetch('/status_json').then(function(r){return r.json();})"
    ".then(function(d){C=d;build();});"
    "setInterval(pollStatus,15000);"
    "setInterval(function(){document.getElementById('clk').textContent="
    "'192.168.0.60 | '+new Date().toLocaleTimeString('nl-BE');},1000);"
    "function setH(v){fetch('/set_home_global?v='+v).then(function(){var b=document.getElementById('hb');b.className='hbtn '+(v?'on':'off');b.textContent=v?'● HOME':'○ UIT';b.onclick=function(){setH(v?0:1);};});}"
    "</script></body></html>");
  return h;
}

// ============================================================
// SETTINGS PAGINA (v5.2: reboot-knop toegevoegd)
// ============================================================
String getSettingsPage() {
  String h = "<!DOCTYPE html><html lang='nl'><head>"
    "<meta charset='utf-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Zarlar Settings</title>"
    "<style>"
    "body{margin:0;background:#111;color:#eee;font-family:monospace;font-size:13px}"
    ".hdr{background:#1a1200;border-bottom:2px solid #f0a500;padding:10px 16px}"
    ".hdr-t{color:#f0a500;font-size:18px;letter-spacing:2px}"
    ".nav{background:#161b22;padding:8px 16px;display:flex;gap:6px;border-bottom:1px solid #333}"
    ".nav a{color:#888;text-decoration:none;padding:4px 10px;border:1px solid #333;border-radius:3px;font-size:11px}"
    ".nav a:hover{color:#f0a500;border-color:#f0a500}"
    ".wrap{padding:14px 16px;max-width:900px}"
    ".sec{background:#161b22;border:1px solid #333;border-radius:6px;margin-bottom:14px;overflow:hidden}"
    ".sec-t{padding:8px 14px;font-size:11px;color:#f0a500;letter-spacing:2px;border-bottom:1px solid #333;text-transform:uppercase}"
    "table{width:100%;border-collapse:collapse}"
    "td{padding:8px 14px;border-bottom:1px solid #222;vertical-align:middle}"
    "td:first-child{color:#888;width:200px}"
    "input[type=text],input[type=password],input[type=number]"
    "{background:#0d1117;border:1px solid #333;color:#eee;padding:5px 8px;"
    "border-radius:3px;font-size:12px;width:100%;font-family:monospace}"
    "input:focus{outline:none;border-color:#f0a500}"
    "input[type=checkbox]{accent-color:#f0a500;width:16px;height:16px}"
    "input[type=range]{accent-color:#f0a500;width:160px;vertical-align:middle}"
    ".badge{font-size:10px;padding:1px 5px;border-radius:2px;margin-left:4px}"
    ".bs{background:#e05a0022;color:#e05a00;border:1px solid #e05a0044}"
    ".br{background:#2ecc4022;color:#2ecc40;border:1px solid #2ecc4044}"
    ".bp{background:#0077ff22;color:#4af;border:1px solid #0077ff44}"
    ".save{background:#f0a500;color:#000;border:none;padding:10px 28px;"
          "border-radius:5px;font-size:13px;font-weight:bold;cursor:pointer;margin:14px 14px}"
    ".save:hover{background:#ffd000}"
    ".rbtn{background:#1c2128;color:#e05a00;border:1px solid #e05a00;padding:10px 28px;"
          "border-radius:5px;font-size:13px;font-weight:bold;cursor:pointer;margin:14px 0 14px 0}"
    ".rbtn:hover{background:#e05a00;color:#000}"
    ".mbtn{background:#1c2128;color:#f0a500;border:1px solid #f0a500;padding:6px 14px;"
           "border-radius:4px;font-size:12px;cursor:pointer;font-family:monospace}"
    ".mbtn:hover{background:#f0a500;color:#000}"
    "</style></head><body>"
    "<div class='hdr'><span class='hdr-t'>⬡ ZARLAR SETTINGS</span></div>"
    "<div class='nav'><a href='/'>← Dashboard</a>"
    "<a href='/settings'>⚙ Settings</a>"
    "<a href='/wifi'>📶 WiFi</a></div>"
    "<div class='wrap'><form action='/save_settings' method='get'>"
    "<div class='sec'><div class='sec-t'>▸ Netwerk</div><table>"
    "<tr><td>WiFi SSID</td><td><input type='text' name='wifi_ssid' value='";
  h += wifi_ssid;
  h += "'></td></tr><tr><td>WiFi Wachtwoord</td>"
       "<td><input type='password' name='wifi_pass' value='";
  h += wifi_pass;
  h += "'></td></tr></table></div>"
       "<div class='sec'><div class='sec-t'>▸ Logging</div><table>"
       "<tr><td>Poll interval (min)</td><td><input type='number' name='poll_min' min='1' max='60' value='";
  h += String(poll_minutes);
  h += "'> <span style='color:#555;font-size:11px'>Sheets + Photons</span></td></tr>"
       "<tr><td>Live-update (s)</td><td><input type='number' name='live_sec' min='0' max='300' value='";
  h += String(live_sec);
  h += "'> <span style='color:#555;font-size:11px'>ESP32's + matrix, 0 = uit</span></td></tr><tr><td>Room Script URL</td>"
       "<td><input type='text' name='room_script' value='";
  h += room_script_url;
  h += "' placeholder='https://script.google.com/...'></td></tr></table></div>"
       "<div class='sec'><div class='sec-t'>▸ Controllers</div><table>"
       "<tr style='background:#1a1a1a'>"
       "<td style='color:#f0a500;font-size:11px'>Controller</td>"
       "<td style='color:#f0a500;font-size:11px'>IP / ID</td>"
       "<td style='color:#f0a500;font-size:11px;text-align:center;width:60px'>Actief</td>"
       "<td style='color:#f0a500;font-size:11px'>Sheets URL</td></tr>";
  for (int i = 0; i < NUM_CONTROLLERS; i++) {
    String tb = (controllers[i].type == TYPE_PHOTON) ? "bp" :
                (controllers[i].type == TYPE_SYSTEM) ? "bs" : "br";
    String tn = (controllers[i].type == TYPE_PHOTON) ? "P" :
                (controllers[i].type == TYPE_SYSTEM) ? "S" : "R";
    String ipid = strlen(controllers[i].ip) > 0 ?
                  String(controllers[i].ip) : String(controllers[i].photon_id).substring(0,12) + "...";
    h += "<tr><td>" + String(controllers[i].name) +
         "<span class='badge " + tb + "'>" + tn + "</span></td>";
    h += "<td style='color:#555;font-size:11px'>" + ipid + "</td>";
    h += "<td style='text-align:center'><input type='checkbox' name='act_" + String(i) +
         "' value='1'" + (controllers[i].active ? " checked" : "") + "></td>";
    if (controllers[i].type == TYPE_ROOM || controllers[i].type == TYPE_PHOTON) {
      h += "<td style='color:#444;font-size:11px'>— gedeeld script —</td>";
    } else {
      h += "<td><input type='text' name='url_" + String(i) +
           "' value='" + controllers[i].sheets_url +
           "' placeholder='https://script.google.com/...'></td>";
    }
    h += "</tr>";
  }
  h += "</table></div>"
       "<button type='submit' class='save'>💾 Opslaan</button>"
       "</form>";

  // ── Statusmatrix ─────────────────────────────────────────────────────────
  h += "<div class='sec'><div class='sec-t'>▸ Statusmatrix 16×16</div><table>"
       "<tr><td>Helderheid</td><td>"
       "<input type='range' id='mbr' min='5' max='200' step='5' value='";
  h += String(matrix_brightness);
  h += "' oninput=\"document.getElementById('mbv').textContent=this.value;"
       "fetch('/matrix_bright?v='+this.value);\">"
       " <span id='mbv'>";
  h += String(matrix_brightness);
  h += "</span><span style='color:#555'>/200</span></td></tr>"
       "<tr><td>Testpatroon</td><td>"
       "<button class='mbtn' onclick=\"fetch('/matrix_test').then(function(){"
       "document.getElementById('mts').textContent='✓ Testpatroon actief';})\">"
       "🔆 Activeer test</button>"
       " <button class='mbtn' onclick=\"fetch('/matrix_update').then(function(){"
       "document.getElementById('mts').textContent='✓ Matrix bijgewerkt';})\">"
       "↺ Live data</button>"
       " <span id='mts' style='color:#f0a500;font-size:11px'></span></td></tr>"
       "<tr><td>Oriëntatie</td><td style='color:#555;font-size:11px'>"
       "MATRIX_FLIP_H = ";
  h += MATRIX_FLIP_H ? "true" : "false";
  h += " — pas define aan in sketch als pixels gespiegeld zijn</td></tr>"
       "</table></div>";


  // ── Systeem info ─────────────────────────────────────────────────────────
  h += "<div class='sec'><div class='sec-t'>▸ Systeem info</div><table>"
       "<tr><td>IP adres</td><td>";
  h += ap_mode ? "AP " + WiFi.softAPIP().toString() : WiFi.localIP().toString();
  h += "</td></tr><tr><td>Mode</td><td>";
  h += ap_mode ? "AP (Zarlar-Setup)" : "STA";
  h += "</td></tr><tr><td>WiFi RSSI</td><td>";
  h += String(WiFi.RSSI()) + " dBm";
  h += "</td></tr><tr><td>Uptime</td><td>";
  h += String(millis() / 1000) + " s";
  h += "</td></tr><tr><td>Free heap</td><td>";
  h += String(ESP.getFreeHeap()) + " bytes";
  h += "</td></tr><tr><td>Largest block</td><td>";
  h += String(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT)) + " bytes";
  h += "</td></tr><tr><td>Poll interval</td><td>";
  h += String(poll_minutes) + " min · live " + (live_sec > 0 ? String(live_sec) + " s" : String("uit"));
  h += "</td></tr></table>"
       // ── Reboot knop (v5.2) ────────────────────────────────────────────
       "<div style='padding:10px 14px'>"
       "<button class='rbtn' onclick='if(confirm(\"Dashboard herstarten?\")){"
       "this.textContent=\"⟳ Herstart...\";this.disabled=true;"
       "fetch(\"/reboot\");}'>⟳ Herstart dashboard</button>"
       " <button class='rbtn' onclick='location.href=\"/update\"'>⬆ OTA update</button>"
       "</div></div>"
       "</div></body></html>";
  return h;
}

// ============================================================
// WEB SERVER
// ============================================================
void setupWebServer() {
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", getMainPage());
  });
  server.on("/status_json", HTTP_GET, []() {
    handleStatusJson();
  });
  server.on("/settings", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", getSettingsPage());
  });
  server.on("/save_settings", HTTP_GET, []() {
    if (server.hasArg("wifi_ssid"))    wifi_ssid       = server.arg("wifi_ssid");
    if (server.hasArg("wifi_pass"))    wifi_pass       = server.arg("wifi_pass");
    if (server.hasArg("poll_min"))     poll_minutes    = server.arg("poll_min").toInt();
    if (server.hasArg("live_sec")) {
      int ls = server.arg("live_sec").toInt();
      live_sec = (ls <= 0) ? 0 : constrain(ls, 5, 300);   // v6.2
    }
    if (server.hasArg("room_script"))  room_script_url = server.arg("room_script");
    for (int i = 0; i < NUM_CONTROLLERS; i++) {
      controllers[i].active = server.hasArg("act_" + String(i));
      controllers[i].status = controllers[i].active ? STATUS_PENDING : STATUS_INACTIVE;
      String uk = "url_" + String(i);
      if (server.hasArg(uk)) controllers[i].sheets_url = server.arg(uk);
    }
    saveNVS();
    server.send(200, "text/html",
      "<html><body style='background:#111;color:#f0a500;font-family:monospace;"
      "padding:40px;text-align:center'>"
      "<h2>✓ Opgeslagen!</h2><p>Herstart...</p>"
      "<script>setTimeout(()=>location.href='/',2500);</script>"
      "</body></html>");
    delay(2000);
    ESP.restart();
  });

  // v5.2: reboot endpoint
  // v6.1: OTA — GET toont het uploadformulier, POST flasht het .bin-bestand
  server.on("/update", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8",
      "<!DOCTYPE html><html><head><meta charset='utf-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'><title>Dashboard OTA</title>"
      "<style>body{background:#111;color:#ddd;font-family:monospace;padding:24px}"
      "h2{color:#f0a500;letter-spacing:3px}input,button{font-size:15px;margin:10px 0}"
      "button{background:#f0a500;color:#111;border:0;border-radius:6px;padding:10px 20px;font-weight:bold}"
      "#s{color:#f0a500;margin-top:12px}</style></head><body>"
      "<h2>▸ OTA UPDATE</h2><div>Zarlar Dashboard v" DASH_VERSION "</div>"
      "<p>Arduino IDE: Sketch → Export Compiled Binary → kies <b>ESP32_C6_Zarlar_Dashboard_Matrix.ino.bin</b>"
      " (niet de .bootloader-, .partitions- of .merged-bestanden).</p>"
      "<input type='file' id='f' accept='.bin'><br><button onclick='up()'>⬆ Uploaden</button>"
      "<div id='s'></div><p><a href='/settings' style='color:#888'>← terug</a></p>"
      "<script>function up(){var f=document.getElementById('f').files[0];if(!f)return;"
      "var s=document.getElementById('s'),d=new FormData();d.append('update',f);"
      "var x=new XMLHttpRequest();x.open('POST','/update');"
      "x.upload.onprogress=function(e){if(e.lengthComputable)s.textContent='Uploaden… '+Math.round(e.loaded*100/e.total)+' %';};"
      "x.onload=function(){s.textContent=x.responseText=='OK'?'✓ Gelukt — herstart… (±15 s)':'✗ Mislukt: '+x.responseText;"
      "if(x.responseText=='OK')setTimeout(function(){location.href='/';},15000);};"
      "x.onerror=function(){s.textContent='✗ Verbinding verbroken';};x.send(d);}</script></body></html>");
  });
  server.on("/update", HTTP_POST, []() {
    bool ok = !Update.hasError();
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", ok ? "OK" : Update.errorString());
    if (ok) { delay(500); ESP.restart(); }
  }, []() {
    HTTPUpload& up = server.upload();
    if (up.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] Start: %s\n", up.filename.c_str());
      matrix.clear(); matrix.show();             // matrix uit tijdens het flashen
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (up.status == UPLOAD_FILE_WRITE) {
      if (Update.write(up.buf, up.currentSize) != up.currentSize) Update.printError(Serial);
    } else if (up.status == UPLOAD_FILE_END) {
      if (Update.end(true)) Serial.printf("[OTA] OK: %u bytes — herstart\n", up.totalSize);
      else                  Update.printError(Serial);
    } else if (up.status == UPLOAD_FILE_ABORTED) {
      Update.abort();
      Serial.println("[OTA] Afgebroken");
    }
  });

  server.on("/reboot", HTTP_GET, []() {
    server.send(200, "text/html",
      "<html><body style='background:#111;color:#f0a500;font-family:monospace;"
      "padding:40px;text-align:center'>"
      "<h2>⟳ Herstart...</h2><p>Dashboard herstart. Even wachten...</p>"
      "<script>setTimeout(()=>location.href='/settings',9000);</script>"
      "</body></html>");
    delay(500);
    ESP.restart();
  });

  server.on("/set_home_global", HTTP_GET, []() {
    int v = server.hasArg("v") ? constrain(server.arg("v").toInt(), 0, 1) : 0;
    home_mode_global = (v == 1);
    preferences.begin(NVS_NS, false);
    preferences.putBool(NVS_HOME_GLOBAL, home_mode_global);
    preferences.end();
    server.send(200, "text/plain", "OK");
    setAllRoomsHomeMode(v);
    Serial.printf("[HOME] → %s\n", v ? "THUIS" : "WEG");
  });


  // v6.0: thuis/weg leesbaar voor de RPi-bridge (Apple Home-tegel "Home")
  server.on("/home_json", HTTP_GET, []() {
    server.send(200, "application/json", home_mode_global ? "{\"home\":1}" : "{\"home\":0}");
  });

  server.on("/matrix_bright", HTTP_GET, []() {
    if (server.hasArg("v")) {
      int v = constrain(server.arg("v").toInt(), 5, 200);
      matrix_brightness = (uint8_t)v;
      updateMatrix();                 // v6.2: helderheid zit in matPx()
      preferences.begin(NVS_NS, false);
      preferences.putUChar(NVS_MATRIX_BRIGHT, matrix_brightness);
      preferences.end();
    }
    char buf[24];
    snprintf(buf, sizeof(buf), "{\"bright\":%d}", matrix_brightness);
    server.send(200, "application/json", buf);
  });

  server.on("/matrix_test", HTTP_GET, []() {
    matrixTestPattern();
    server.send(200, "text/plain", "Testpatroon actief");
  });

  server.on("/matrix_update", HTTP_GET, []() {
    updateMatrix();
    server.send(200, "text/plain", "Matrix bijgewerkt");
  });

  server.on("/wifi", HTTP_GET,      []() { server.send(200, "text/html; charset=utf-8", getWifiPage()); });
  server.on("/wifi_json", HTTP_GET, []() { handleWifiJson(); });
  server.on("/wifi_snap", HTTP_GET, []() { handleWifiSnap(); });

  auto cpRedirect = []() {
    if (ap_mode) {
      server.sendHeader("Location", "http://" + WiFi.softAPIP().toString() + "/settings");
      server.send(302, "text/plain", "");
    } else {
      server.send(404, "text/plain", "404");
    }
  };
  server.on("/hotspot-detect.html",       HTTP_GET, cpRedirect);
  server.on("/library/test/success.html", HTTP_GET, cpRedirect);
  server.on("/generate_204",              HTTP_GET, cpRedirect);
  server.on("/connecttest.txt",           HTTP_GET, cpRedirect);
  server.onNotFound(cpRedirect);

  server.begin();
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("\n\n╔══════════════════════════════════════╗");
  Serial.println("║  Zarlar Dashboard v" DASH_VERSION "               ║");
  Serial.println("║  192.168.0.60 — Statusmatrix 16×16  ║");
  Serial.println("╚══════════════════════════════════════╝\n");

  loadNVS();

  matrix.begin();
  matrix.setBrightness(255);          // v6.2: helderheid + ondergrens in matPx()
  matrix.clear();
  matrix.show();
  Serial.printf("[Matrix] Init: %d LEDs, helderheid %d/255\n",
                MATRIX_LEDS, matrix_brightness);
  Serial.printf("[Matrix] Heap na init: %d bytes (largest: %d)\n",
                ESP.getFreeHeap(),
                heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

  connectWiFi();
  matrixBootAnimation();


  last_poll_cycle = millis() - (unsigned long)(poll_minutes * 60000UL) + 30000UL;
  setupWebServer();
  Serial.println("READY — http://192.168.0.60/\n");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  if (ap_mode) dnsServer.processNextRequest();
  server.handleClient();
  handlePolling();


  // ── Matrix periodieke refresh elke 60s ───────────────────────────────────
  if (millis() - last_matrix_update > 60000UL) {
    updateMatrix();
  }

  // ── Automatische herstart bij persistente heap-uitputting (v5.2) ─────────
  // Als largest_block > 60s aaneensluitend onder 25KB daalt → herstart.
  // Vangt de root cause op als failsafe, mocht fragmentatie toch optreden.
  {
    static unsigned long low_heap_since = 0;
    uint32_t lb = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
    if (lb < 25000) {
      if (low_heap_since == 0) low_heap_since = millis();
      else if (millis() - low_heap_since > 60000UL) {
        Serial.printf("[HEAP] Largest block %u bytes < 25KB voor >60s — herstart\n", lb);
        delay(200);
        ESP.restart();
      }
    } else {
      low_heap_since = 0;
    }
  }

  // ── Serial commando's ─────────────────────────────────────────────────────
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.equalsIgnoreCase("reset-all")) {
      Serial.println(F("Alles wissen + reboot..."));
      preferences.begin(NVS_NS, false);
      preferences.clear();
      preferences.end();
      nvs_flash_erase();
      delay(300);
      ESP.restart();
    }
    if (cmd.equalsIgnoreCase("status")) {
      Serial.printf("\n=== Zarlar Dashboard v%s | Uptime: %lu s ===\n", DASH_VERSION, millis()/1000);
      Serial.printf("IP: %s  RSSI: %d dBm\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
      Serial.printf("Heap free: %u  Largest: %u\n",
                    ESP.getFreeHeap(),
                    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      Serial.printf("HOME: %s  Matrix brightness: %d\n",
                    home_mode_global ? "THUIS" : "WEG", matrix_brightness);
      Serial.println(F("\n--- Actieve controllers (idx | naam | status | json?) ---"));
      for (int i = 0; i < 22; i++) {
        if (controllers[i].active) {
          Serial.printf("  idx %2d | %-16s | %s | json=%s\n",
            i,
            controllers[i].name,
            controllers[i].status == STATUS_ONLINE  ? "ONLINE " :
            controllers[i].status == STATUS_OFFLINE ? "OFFLINE" :
            controllers[i].status == STATUS_PENDING ? "PENDING" : "INACTF ",
            strlen(controllers[i].last_json) > 5 ? "ja" : "nee");
        }
      }
      Serial.println(F("--- MROW matrix-rij mapping (ESP32 heeft voorrang op Photon) ---"));
      for (int r = 0; r < MATRIX_HEIGHT; r++) {
        const MatrixRowDef& rd = MROW[r];
        if (rd.sys_idx == -2) { Serial.printf("  rij %2d → separator\n", r); continue; }
        if (rd.sys_idx == -1 && rd.esp_idx == -1 && rd.photon_idx == -1) continue;

        if (rd.sys_idx >= 0) {
          Serial.printf("  rij %2d → SYS  idx %2d (%s) actief=%s\n",
            r, rd.sys_idx, controllers[rd.sys_idx].name,
            controllers[rd.sys_idx].active ? "JA" : "NEE");
          continue;
        }

        bool esp_ok = (rd.esp_idx >= 0) && controllers[rd.esp_idx].active
                   && strlen(controllers[rd.esp_idx].last_json) > 5;
        bool pho_ok = (rd.photon_idx >= 0) && controllers[rd.photon_idx].active
                   && strlen(controllers[rd.photon_idx].last_json) > 5;

        const char* esp_name = (rd.esp_idx >= 0) ? controllers[rd.esp_idx].name : "—";
        const char* pho_name = (rd.photon_idx >= 0) ? controllers[rd.photon_idx].name : "—";

        if (esp_ok) {
          Serial.printf("  rij %2d → ESP32 idx %2d (%s) ✓ actief\n",
            r, rd.esp_idx, esp_name);
        } else if (pho_ok) {
          Serial.printf("  rij %2d → Photon idx %2d (%s) ✓ fallback [ESP32 %s inactief]\n",
            r, rd.photon_idx, pho_name, esp_name);
        } else {
          Serial.printf("  rij %2d → leeg  [ESP32 %s / Photon %s]\n",
            r, esp_name, pho_name);
        }
      }
    }
    if (cmd.equalsIgnoreCase("matrix-test")) {
      matrixTestPattern();
    }
    if (cmd.equalsIgnoreCase("matrix-update")) {
      updateMatrix();
      Serial.println(F("[Matrix] Manueel bijgewerkt"));
    }
  }

  delay(2);
}
