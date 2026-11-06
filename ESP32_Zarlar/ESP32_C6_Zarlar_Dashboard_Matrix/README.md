# Zarlar Dashboard — sketchmap

Open deze map in de Arduino IDE. De sketch heet **`ESP32_C6_Zarlar_Dashboard_Matrix.ino`** en staat naast `partitions.csv` (16 MB, wordt automatisch gebruikt).

**Versie: v6.4** (6 nov 2026, in test). Nieuw: matrixrij 3 toont de buitenverlichting rondom het huis (W · Z · N · O), zie de changelog bovenaan de sketch en Master §8.6. v6.3 staat in de geschiedenis van de repo.

## Rij 3 — buitenverlichting (sinds v6.4)

De 4 muren rondom het huis, van links naar rechts W · Z · N · O. Bron: de Photon R8-ACCESS (variabele `JSON_status`, via de Worker), later de ESP32-controllers ACCESS1–3. Bijgewerkt om de ±9 min (rondgang van de Photons).

| Pixel | Label | Kleur → betekenis |
|---|---|---|
| 1 | W | groen = online, automatisch · oranje = online, manueel · rood = offline · geel = wacht op de eerste poll |
| 2 | PIR W | paars = beweging aan W1 of W2 in de laatste 10 min · zacht grijs = geen beweging |
| 3 | Voordeur | warm geel = licht W1 aan · zacht grijs = uit |
| 4 | Veldkant | warm geel = licht W2 aan · zacht grijs = uit |
| 5 | Z | zoals pixel 1 (zelfde controller) |
| 6 | PIR Z | paars = beweging aan S in de laatste 10 min · zacht grijs = geen beweging |
| 7 | Terras | warm geel = licht S aan · zacht grijs = uit |
| 8 | — | uit (vrij) |
| 9 | N | heel dim grijs = nog geen controller (ACCESS2) |
| 10–12 | — | uit (later PIR en licht noord) |
| 13 | O | heel dim grijs = nog geen controller (ACCESS3) |
| 14–16 | — | uit (later PIR en licht oost) |

Zolang de bron niet online is (pixel 1 en 5 geel of rood), blijven pixels 2–4 en 6–7 uit.

Board en uploaden: zie het Master-document §2.6 en §8.
