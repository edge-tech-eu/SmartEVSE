# DHZ laadpaal
Kijk, wij hebben alle voorbereidingen voor je gedaan!
We zullen je nu stap voor stap helpen om alles in elkaar te zetten.

Volg de stappen in onderstaande overzicht:
1. De basis-configuratie – de “domme laadpaal”, dus zonder stroomsturing (load-balancing)
2. Met stroomsturing (load-balancing). Kies dan uit één van onderstaande opties:
  - Gebruik een ESP-device of een Pi-pico gekoppeld aan de een DSMR 4 of 5  slimme meter en verbind deze via een RS485-kabel (Modbus) aan het laadstation
  - Gebruik een Raspberry Pi mini-computer met USB-kabel aan een DSMR 4 of 5 slimme meter en een RS485-interface. Gebruik het pakket EVCC om het laadstation aan te sturen
  - Gebruik de Edgetech WiFi-module:
    1. Heeft u een DSMR5 slimme meter? Dan kunt de EVSE-SC-02 module rechtstreeks aan de slimme meter koppelen
    2. Heeft u geen slimme meter of een ouder type? Dan moet de stroom met stroomklemmen (CT) gemeten worden en heeft u type EVSE-SC-01 nodig.

# *Veiligheid*
Dit document is geschreven voor ervaren hobbyïsten. Dus mensen die op een veilige manier apparaten op een 3-fase voedingskabel kunnen aansluiten.
Die weten wat aardlekautomaten zijn en de basis-principes kennen voor het aansluiten van grote stroomgebruikers via de meterkast.
Wij raden altijd aan om wijzigingen in de meterkast door een erkende elektricien uit te laten voeren.

# 1. De Basis-configuratie
Met de basis-configuratie maak je een laadpunt zonder stroomsturing. Dat betekent dat het laadpunt geen rekening houdt met andere stroomgebruikers in het huis. Dit kan mogelijk voor problemen zorgen bij te grote stroomvraag. Zorg dat je dus alleen laadt op die momenten dat er voldoende capaciteit beschikbaar is. Dat is typisch het geval vanaf ‘s avonds 22 uur tot 4 uur in de ochtend. Houdt dus rekening met moglijk aanwezigheid van warmtepompen, thuis-accu’s en elektrische boilers. De basis-configuratie bestaat uit deze onderdelen:
- laad-module
- DC-aardlek-module
- laadkabel
- behuizing en wartels
- montage-beugels
Je kunt deze behuizing zelf eenvoudig in elkaar zetten. Je hebt zelf nog de keuze om de montage-beugels te gebruiken als de standaard-montage van de laad-module niet geschikt is voor jouw situatie.

## Stappen voor de assemblage
- Hieronder het overzicht van alle aansluitingen:
[Aansluitingen](installation/installation.png)
- Bepaal waar het laadpunt wordt geplaatst.
- Kies de handige plaats voor montage van de wartels voor de ingaande voedingskabel en uitgaande laadkabel. Boor gaten voor de wartels met een stappenboor. Boor ook gaten in de achterwand voor stevige montage op een muur of paal. Let er op dat je de schroefgaten waterdicht maakt met b.v. een rubberring of druppel kit.
- Kies de oriëntatie van de laad-module – schroef deze vast of lijm de montagebeugels goed aan de achterwand van de kast.
- Monteer de laad-module in de kast.
- Verbind de DC-aardlek-module aan de laad-module met de meegeleverde draad
- Verbind de meegeleverde draad aardedraad (groen-geel) met het grijze blok met 5 ingangen. Plaats deze op ingang PE. Verbind de andere zijde met de meegeleverde WAGO-connector.
- Voer de laadkabel door de wartel heen. Vervolgens:
  - steek de aders in de juiste uitgangen op het grijze blok met 4 uitgangen. Let op de juiste kleuren voor het verbinden. Sluit de aarde-ader (groen-geel) aan op de WAGO-connector. Je kunt de uitgang ontgrendelen door een platte schroevendraaier in het rechthoekige gat te steken – op dat moment kan de ader in de uitgang worden gestoken.
  - Steek de dunne ader in het grijze blokje met twee oranje heveltje, kies daar de uitgang “PP”, in het plaatje hierboven aangeduid met gele draad met “CP”. Zorg dat je het heveltje helemaal omhoog klikt en de ader diep genoeg in de uitgang steekt. Klik dan het heveltje weer omlaag en controleer dat de ader goed vast zit.
- Schroef de behuizing nu vast op de muur of paal en steek de voedingskabel door de wartel. Vervolgens:
  - Steek de aders van de voedingskabel in de juiste ingangen, let goed op de kleuren op de grijze connector met 5 ingangen.
  - Verbind de aarde-ader (PE) met de WAGO-connector
  - Controleer dat de aders in de grijze connector onderling GEEN contact maken. Dat kan het geval zijn indien er aders met gevlochten draad gebruikt worden. Dan kunnen losse koper draadjes net buiten de ingang blijven zitten en kortsluiting veroorzaken. Let dus goed op!
- Ziet alles er goed uit? Dan kun je de aardlekautomaat in groepenkast activeren. Controleer dat het groene LEDje op de laad-module elke 3 seconden knippert. Zie onderstaand plaatje voor de plaats van het groene LEDje.
