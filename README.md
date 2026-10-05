# GTG — Guerra Termonucleare Globale

GTG è un gioco strategico didattico scritto in C17 con raylib, ispirato
all'estetica dei terminali WOPR del film *WarGames*.

Il progetto è sviluppato per round incrementali. Il Round 05 aggiunge il modello
del mondo indipendente dal renderer: fazioni, regioni, bersagli, piattaforme e
attacchi in volo sono rappresentati da strutture C17 a capacità fissa.

## Requisiti

- compilatore con supporto ISO C17 (Apple Clang, Clang o GCC);
- CMake 3.25 o successivo;
- Ninja consigliato;
- Git e accesso a Internet durante la prima configurazione.

raylib 6.0 non deve essere installata globalmente: CMake la scarica e la
compila tramite `FetchContent`, mantenendo la versione del progetto bloccata.

Su macOS è necessario avere installato Xcode oppure i Command Line Tools:

```sh
xcode-select --install
```

## Configurazione e compilazione

Debug:

```sh
cmake --preset debug
cmake --build --preset debug
```

Release:

```sh
cmake --preset release
cmake --build --preset release
```

## Esecuzione

```sh
./build/debug/gtg
```

Il programma apre una finestra ridimensionabile con un terminale WOPR. Si chiude
con `Esc`, `F10`, il comando `QUIT` o il pulsante di chiusura della finestra.

Controlli disponibili:

- testo + `Invio`: esegue un comando nella shell;
- `Freccia su` / `Freccia giù`: naviga la cronologia dei comandi;
- `Backspace`: corregge l'input;
- `F1`: passa dal menu allo stato di gioco;
- `F2`: mette in pausa o riprende gli aggiornamenti logici;
- `F3`: avanza di un singolo tick mentre il gioco è in pausa;
- `F4`: abilita o disabilita l'effetto telescrivente;
- `F10`: termina la sessione.

Comandi della shell: `HELP`, `CLEAR`, `ABOUT` e `QUIT`. Il parser ignora
maiuscole/minuscole e gli spazi iniziali o finali. Input, output e cronologia
usano buffer a capacità fissa: non vengono effettuate allocazioni per frame.

L'overlay mostra stato, FPS, tick/s, tick totali, tick eseguiti nell'ultimo
frame e fattore di interpolazione. Il rendering e l'effetto telescrivente
restano attivi durante la pausa. Il testo diagnostico usa campi a larghezza
fissa e viene aggiornato quattro volte al secondo per evitare sfarfallii, senza
rallentare il game loop.

Il renderer prova a caricare `assets/fonts/ShareTechMono-Regular.ttf`; se il
file non è presente o non è valido usa in sicurezza il font monospazio integrato
in raylib.

Per aprire la finestra, disegnare un solo frame e terminare automaticamente:

```sh
./build/debug/gtg --smoke-test
```

## Test

```sh
ctest --preset debug
```

I test usano un backend finto e non richiedono un contesto grafico. Verificano
il lifecycle, il cleanup degli errori, l'indipendenza dei tick dal frame rate,
la pausa, il single-step, le transizioni di stato, il limite applicato ai frame
eccezionalmente lunghi e il comportamento della console bounded. Il target
`gtg_world_tests`, collegato soltanto alla libreria `gtg_world`, verifica inoltre
fixture, coordinate normalizzate, relazioni tra entità e rigetto degli ID non
validi senza dipendere da raylib.

## Modello del mondo

Gli ID di fazione, regione, bersaglio, piattaforma e attacco sono tipi distinti:
il compilatore può quindi intercettare lo scambio accidentale tra categorie. Le
query restituiscono puntatori `const` e verificano sia il range sia la
corrispondenza fra ID e slot prima di accedere agli array.

`World` conserva le entità in array contigui a capacità fissa. Questa scelta
rende il costo della memoria prevedibile, evita allocazioni durante la
simulazione e favorisce accessi sequenziali cache-friendly. `world_validate()`
controlla conteggi, riferimenti, percentuali, coordinate e terminazione delle
stringhe; `world_assert_valid()` rende immediatamente visibili le violazioni
nelle build con assertion abilitate.

## Struttura iniziale

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── assets/
├── src/
│   ├── main.c
│   ├── app/
│   │   ├── app.c
│   │   ├── app.h
│   │   ├── app_config.c
│   │   ├── app_config.h
│   │   ├── app_internal.h
│   │   └── app_raylib.c
│   ├── resources/
│   │   ├── resources.c
│   │   └── resources.h
│   ├── ui/
│   │   ├── console.c
│   │   └── console.h
│   ├── world/
│   │   ├── world.c
│   │   └── world.h
│   └── tests/
│       ├── test_main.c
│       └── test_world.c
└── README.md
```

Tutto il codice C, inclusi i test e i futuri header privati, risiede sotto
`src/`. Il progetto non espone una directory `include/` pubblica.

## Licenza

Il codice di GTG è distribuito con licenza MIT. raylib conserva la propria
licenza zlib/libpng.
