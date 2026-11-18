[English](../en/ev-wake-up.md) · Nederlands

# EV wake-up (CP disconnect)

Sommige auto's gaan in diepe slaap als een laadsessie gepauzeerd wordt (IEC 61851 state B: aangesloten, laadt niet), bijvoorbeeld terwijl load balancing de beschikbare stroom aan het huis geeft. Slapen ze eenmaal, dan reageren ze niet als de laadpaal weer stroom aanbiedt, zodat het laden nooit vanzelf hervat.

De laadmodule kan ze wekken met een **CP disconnect**: een korte, bewuste onderbreking of wijziging van de control pilot (CP) waardoor de auto denkt dat de kabel is losgehaald en weer ingestoken. De auto start zijn verbindingsprocedure opnieuw en wordt wakker. Ondersteund vanaf firmware 2.10.17.

## De drie varianten

| Variant | Wat er op CP gebeurt | Hardware |
|---|---|---|
| 1. Fysieke onderbreking | CP wordt een paar seconden onderbroken (standaard 3 s) en daarna hersteld. Voor de auto is dit hetzelfde als de stekker eruit halen. | Vereist de optionele relaisprint voor CP disconnect op de vergrendelingspoort ([vraag het ons](mailto:support@edgetech.eu)) |
| 2. Laadpaal afwezig ("EV40") | De PWM stopt en CP wordt een ingestelde tijd (standaard 3 s) op −12 V gehouden ("laadpaal niet beschikbaar", state F), daarna komt het signaal terug. | Geen |
| 3. Uit- en insteekreeks ("343S") | Vast patroon van ongeveer 10 s: +12 V gedurende 3 s (rust, geen auto), −12 V gedurende 4 s (state F), +12 V gedurende 3 s, daarna normale signalering. Voor auto's die een vollediger uitsteekpatroon nodig hebben. | Geen |

## Wanneer het gebeurt

Een CP disconnect wordt alleen uitgevoerd terwijl de auto in state B is (aangesloten, laadt niet).

- **Automatisch:** elke keer dat de sessie state B ingaat (bij het aansluiten, of als het laden pauzeert) start een timer, standaard 30 s. Is de auto nog in state B als die afloopt, dan volgt één disconnect. Die herhaalt zich niet zolang de auto in state B blijft; de volgende keer dat de auto state B ingaat, wordt hij opnieuw geactiveerd.
- **Op commando:** een controller kan op elk moment om een disconnect vragen. Die wordt alleen in state B uitgevoerd, en nooit terwijl er al een disconnect loopt.

Zowel de automatische trigger als de variant zijn instellingen, en beide staan standaard uit.

## Configuratie via Modbus

Zie het [registeroverzicht](modbus-registers.md) voor alle registers. Modbus RTU via RS485, 9600 baud 8N1. Op fabrieksadres 0x01 antwoordt de print pas na een state-verandering, bijvoorbeeld via de knop op de print; geef hem eerst een eigen adres (register 0x0200).

### 1. Kies het mechanisme: settings-register 0x0204 (fc 3 / fc 6)

Lees het register, wijzig alleen deze bits en schrijf het terug: hetzelfde register bevat ook de instellingen voor fasen, vergrendeling en DC-aardlek.

| Bit | Masker | Naam | Effect |
|---|---|---|---|
| 3 | 0x0008 | MISUSE_LOCK_PORT_AS_CP_DISCONNECT | Variant 1: de poort van de kabelvergrendeling stuurt het relais voor CP disconnect aan. De vergrendelingsregisters (0x0010, 0x0015, 0x0016) geven dan 0xFFFF terug |
| 4 | 0x0010 | AUTO_CP_DISCONNECT | Automatische trigger na het ingaan van state B. Zonder deze bit gebeuren alleen disconnects op commando |
| 5 | 0x0020 | EV40_CP_DISCONNECT | Software-disconnect: variant 2 of 3, hieronder te kiezen |

Als bit 3 en bit 5 allebei gezet zijn, wint bit 3: het relais wordt gebruikt en de softwarevariant genegeerd. Zet maar één van de twee.

### 2. Kies de softwarevariant: hoge byte van 0x020A

Register 0x020A bevat twee bytes: lage byte CPType, hoge byte CPDisconnectType. Lees het, wijzig alleen de hoge byte en schrijf het terug. Hoge byte 0 = variant 2 (EV40, standaard), 1 = variant 3 (343S).

### 3. Timing: 0x0208 en 0x0209

- **CPDisconnectTime** (0x0208): duur van de disconnect in ms voor variant 1 en 2, standaard 3000. Variant 3 gebruikt altijd 3 s / 4 s / 3 s.
- **TimeBeforeCPDisconnect** (0x0209): wachttijd in ms na het ingaan van state B vóór de automatische disconnect, standaard 30000.

### 4. Sla de instellingen op

Geschreven registers werken direct, maar gaan bij een herstart verloren zolang ze niet zijn opgeslagen:

1. Schrijf de magic code (standaard 0x1234) naar MagicResetCode (0x0202, fc 6).
2. Lees in de direct volgende transactie SaveEEPROM (input register 0x000A, fc 4). Dat geeft 0x0000 terug als het is opgeslagen, of 0xFFFF als de ontgrendeling is gemist. De ontgrendeling geldt alleen voor die ene volgende transactie.

### 5. Vraag om een disconnect

Lees input register Disconnect CP (0x0017, fc 4). Dat geeft 1 terug als de disconnect is uitgevoerd, en 0xFFFF als de laadpaal niet is ingesteld voor CP disconnect (bit 3 noch bit 5 gezet). De disconnect wordt alleen in state B uitgevoerd, en nooit terwijl er al een disconnect loopt.

## Voorbeeld: automatisch wekken met variant 3

1. Lees 0x0204, OR er 0x0030 bij (bits 4 en 5) en schrijf het terug.
2. Lees 0x020A, zet de hoge byte op 1, laat de lage byte staan en schrijf het terug.
3. Optioneel: wijzig 0x0209 (wachttijd vóór de disconnect, ms).
4. Schrijf 0x1234 naar 0x0202, lees dan 0x000A en verwacht 0x0000.
