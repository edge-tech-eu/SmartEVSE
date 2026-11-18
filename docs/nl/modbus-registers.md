[English](../en/modbus-registers.md) · Nederlands

# Modbus-registeroverzicht — laadmodule EVSE-C-3x32A

> Dezelfde bron als de registerpagina op edgetech.eu. Firmware 2.10.17 en later.

Geldt voor EVSE-C-3x32A met firmware 2.10.17 en later. Modbus RTU via RS485, 9600 baud 8N1 (19200 als een EVSE-SC-02 de module aanstuurt), fabrieksadres 0x01.

**Adres 0x01.** De laadpaal verlaat de fabriek met adres 0x01. Dit is een bijzonder adres: na het opstarten antwoordt de laadpaal daar niet, pas na een state-verandering (bijvoorbeeld de knop op de print, of een auto aansluiten). Een EdgeTech-wifi-module zet elke nieuwe laadpaal op een eigen adres, zodat alle laadpalen op de bus bereikbaar zijn. Doe met je eigen controller hetzelfde: schrijf na een state-verandering een nieuw adres van 0x02 of hoger naar register 0x0200 en sla het op.

**Eenheden.** Alle analoge waarden gebruiken eenheden van 1/256: spanning in 1/256 V, stroom in 1/256 A, energie in 1/256 kWh. Deel door 256.

**Timing.** De print verwacht een antwoordvenster van 40 ms. Gebruik een bedrade RS485-bus. Modbus via ethernet (een RS485-naar-ethernet-gateway, Modbus TCP) werkt in principe, maar de reactie is trager: prima om vanaf een pc te testen, maar niet met EdgeTech-wifi-modules, die snelle antwoorden van de laadpaal nodig hebben. Draadloze Modbus werkt niet.

## De twee registers die je nodig hebt

| Register | fc | Betekenis |
|---|---|---|
| `0x0103` | 4 | Lage byte: IEC 61851-state (1 = A, 2 = B, 3 = C, 4 = D). Hoge byte: foutbitmasker. |
| `0x0201` lage byte | 6 | RemoteMaxCurrent in ampère. Minstens één keer per seconde schrijven. `0` stopt het laden. |

Al het andere is monitoring en configuratie.

## Alleen-lezen-registers (fc 4)

### Systeemregisters 0x0000–0x0017

| Adres | Naam | Grootte | Beschrijving |
|---|---|---|---|
| `0x0000` | SerialNumber | 5 | Serienummer van het apparaat, 5 opeenvolgende registers |
| `0x0005` | FirmwareVersion | 1 | Versie van de applicatiefirmware |
| `0x0006` | MsSinceBoot | 4 | Milliseconden sinds het opstarten, 64-bit |
| `0x000A` | SaveEEPROM | 1 | Instellingen opslaan in EEPROM. Beveiligd |
| `0x000B` | FactoryDefaults | 1 | Alle instellingen resetten. Beveiligd |
| `0x000C` | Reset | 1 | De applicatie herstarten. Beveiligd |
| `0x000D` | EnterBootloader | 1 | Naar de bootloader gaan. Beveiligd |
| `0x000E` | ChallengeResponse | 1 | Magische code controleren |
| `0x000F` | EnableTestMode | 1 | Niet gebruikt |
| `0x0010` | ExternalLock | 1 | Kabelvergrendeling: `0` ontgrendeld, `1` vergrendeld, `0xFFFF` vergrendelingspoort gebruikt als CP disconnect |
| `0x0011` | I2cErrors | 1 | Intern aantal I²C-fouten lezen en resetten |
| `0x0012` | QuietReset | 1 | Resetten zonder CP-glitch |
| `0x0013` | ConditionalAnswer | 1 | Voorwaardelijk antwoord |
| `0x0014` | ConditionalAddress | 1 | Voorwaardelijk antwoord |
| `0x0015` | LockLock | 1 | Kabelvergrendeling inschakelen. Geeft `1` terug, of `0xFFFF` als de poort als CP disconnect wordt gebruikt |
| `0x0016` | UnlockLock | 1 | Kabelvergrendeling uitschakelen. Geeft `0` terug, of `0xFFFF` |
| `0x0017` | DisconnectCP | 1 | Kortstondige CP disconnect (EV wake-up). Geeft `1` terug, of `0xFFFF` als dit niet is geconfigureerd |

Voor beveiligde registers moet eerst de magische code (standaard `0x1234`) naar `0x0202` worden geschreven.

### Statusregisters 0x0100–0x0113

| Adres | Naam | Grootte | Eenheid | Beschrijving |
|---|---|---|---|---|
| `0x0100` | MinPilot | 1 | ruw | Minimaal gemeten pilotniveau |
| `0x0101` | MaxPilot | 1 | ruw | Maximaal gemeten pilotniveau |
| `0x0102` L | MaxCableCurrent | ½ | A | Belastbaarheid van de kabel volgens de PP-weerstand |
| `0x0102` H | MaxCurrent | ½ | A | Laadstroom die nu wordt doorgegeven |
| `0x0103` L | State | ½ | enum | IEC 61851-state, zie hieronder |
| `0x0103` H | Error | ½ | bitmasker | Foutbits, zie hieronder |
| `0x0104` | Temperature | 1 | °C | Temperatuur van de print |
| `0x0105` | CurrentL1 | 1 | 1/256 A | Stroom op L1 |
| `0x0106` | CurrentL2 | 1 | 1/256 A | Stroom op L2 |
| `0x0107` | CurrentL3 | 1 | 1/256 A | Stroom op L3 |
| `0x0108` | EnergySession | 1 | 1/256 kWh | Energie van de laadsessie, alle fasen |
| `0x0109` | VoltageL1 | 1 | 1/256 V | Uitgangsspanning L1, alleen ongelijk aan nul in state C/D |
| `0x010A` | VoltageL2 | 1 | 1/256 V | Uitgangsspanning L2 |
| `0x010B` | VoltageL3 | 1 | 1/256 V | Uitgangsspanning L3 |
| `0x010C` | Options | 1 | bits | Bit 0: DC-aardlekdetector (RDC-DD) ingeschakeld. Bit 1: 32 A-modus (0 = 16 A) |
| `0x010D` | Energy | 2 | 1/256 kWh | Totale energie over de levensduur, 32-bit: `(reg[0x010E] << 16) \| reg[0x010D]` |
| `0x010F` | EnergySessionL1 | 1 | 1/256 kWh | Energie van de laadsessie L1 |
| `0x0110` | EnergySessionL2 | 1 | 1/256 kWh | Energie van de laadsessie L2 |
| `0x0111` | EnergySessionL3 | 1 | 1/256 kWh | Energie van de laadsessie L3 |
| `0x0112` | EarthQuality | 1 | ruw | Kwaliteit van de aarde, experimenteel |
| `0x0113` | UBRR | 1 | ruw | Huidige waarde van het baudrateregister |

Omrekening: `reg × 1000 / 256` geeft millivolt of milliampère. `0x086A` = 2154 → 8414 mA.

### State (0x0103 lage byte)

| Waarde | IEC-state | CP-spanning | Relais | Betekenis |
|---|---|---|---|---|
| 1 | A | +12 V | open | Geen voertuig |
| 2 | B | +9 V | open | Voertuig aangesloten, laadt niet |
| 3 | C | +6 V | gesloten | Laden |
| 4 | D | +3 V | gesloten | Laden, ventilatie vereist |

Het state-register begint bij 1. Spannings- en stroomregisters zijn alleen zinvol in state C of D.

### Foutbits (0x0103 hoge byte)

| Bit | Waarde | Fout | Ernst |
|---|---|---|---|
| 0 | 1 | Temperatuur te hoog | Kritiek |
| 1 | 2 | Vastzittend relais | Kritiek |
| 2 | 4 | Aardfout | Kritiek |
| 3 | 8 | Max CP te laag | Waarschuwing, pilot buiten bereik |
| 4 | 16 | Min CP te hoog | Waarschuwing, pilot buiten bereik |
| 5 | 32 | DC-aardlekstroom gedetecteerd door de DC-aardlekdetector (RDC-DD) op de print (EVSE-X-DCD) | Laden geblokkeerd |
| 6 | 64 | I²C-initialisatie mislukt | Intern |
| 7 | 128 | I²C-communicatiefout | Intern |

`0` betekent geen fout. Bits kunnen gecombineerd voorkomen.

## Lees-/schrijfregisters (fc 3 lezen, fc 6 schrijven)

### Configuratie 0x0200–0x020A

| Adres | Naam | Grootte | Beschrijving |
|---|---|---|---|
| `0x0200` L | ModbusAddress | ½ | Slave-adres. Beveiligd. Fabriek `0x01` (antwoordt alleen na een state-verandering); wifi-modules zetten elke laadpaal op een eigen adres |
| `0x0200` H | Discovery | ½ | Niet gebruikt |
| `0x0201` L | RemoteMaxCurrent | ½ | Doorgegeven laadstroomlimiet in ampère. `0` schakelt laden uit. Het stuurregister. |
| `0x0201` H | RemoteMaxCurrentNextSecond | ½ | Failsafe-limiet van één seconde; valt terug op RemoteMaxCurrent als hij niet binnen 1 s opnieuw wordt geschreven. Niet voor normale aansturing. |
| `0x0202` | MagicResetCode | 1 | De magische code wijzigen |
| `0x0203` | Challenge | 1 | Challenge-response voor beveiligde schrijfopdrachten |
| `0x0204` | Settings | 1 | Bitveld, zie hieronder |
| `0x0205` | Led | 1 | `0` GRB-led op de print, `1` adresseerbare ledstrip (36 leds) |
| `0x0206` | Condition | 1 | Voorwaardelijk antwoord |
| `0x0207` | UBRR | 1 | Baudrateregister (`0x33` = 19200) |
| `0x0208` | CPDisconnectTime | 1 | Duur van de CP disconnect, ms |
| `0x0209` | TimeBeforeCPDisconnect | 1 | Wachttijd na het doorgeven van stroom voordat op state C wordt gecontroleerd, ms |
| `0x020A` L | CPType | ½ | `0` normaal, `1` A1/B1-pilotniveaus |
| `0x020A` H | CPDisconnectType | ½ | `0` EV40, `1` uitgebreide EV40 |

### Settings-bitveld (0x0204)

| Bit | Naam | Betekenis |
|---|---|---|
| 0 | PHASES | 1-fase (0) of 3-fase (1) |
| 1 | LOCK_STATE | Gedrag van de kabelvergrendeling |
| 2 | DCL_MUST_BE_PRESENT | Laden blokkeren als de DC-aardlekdetector (RDC-DD) ontbreekt |
| 3 | MISUSE_LOCK_PORT_AS_CP_DISCONNECT | De vergrendelingspoort gebruiken voor CP disconnect |
| 4 | AUTO_CP_DISCONNECT | Automatische CP disconnect na een pauze (EV wake-up) |
| 5 | EV40_CP_DISCONNECT | CP disconnect in EV40-stijl |

## Bootloader (adres 0xF0)

| Adres | fc | Naam | Beschrijving |
|---|---|---|---|
| `0x0500` | 4 | BootloaderVersion | Versie van de bootloader, `0` als je hem op het applicatieadres leest |
| variabel | 0x10 | FlashPage | Firmwarepagina's van 64 registers schrijven; lage adresbyte = pagina, hoge byte gezet = laatste pagina |

De wifi-module gebruikt dit om de firmware van de laadmodule in het veld bij te werken.

## Minimale regellus voor een controller

```text
every 1 s:
    state, error = read fc4 0x0103           # lage byte state, hoge byte fout
    if error & 0b111: limit = 0              # temperatuur, vastzittend relais, aardfout
    else:            limit = min(site_headroom_A, cable_A, 32)
    write fc6 0x0201 = limit                 # ampère; 0 stopt
```

Mis je twee schrijfopdrachten achter elkaar, dan valt de print terug op zijn vaste limiet.
