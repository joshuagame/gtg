# GTG — Guerra Termonucleare Globale

GTG è un gioco strategico didattico scritto in C17 con raylib, ispirato
all'estetica dei terminali WOPR del film *WarGames*.

Il progetto è sviluppato per round incrementali. Il Round 03 introduce un game
loop a passo fisso, una macchina a stati applicativa e un overlay diagnostico;
la logica del mondo verrà aggiunta nei round successivi.

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

Il programma apre una finestra ridimensionabile con lo sfondo WOPR iniziale. Si
chiude con `Esc` o con il pulsante di chiusura della finestra.

Controlli disponibili:

- `Invio`: passa dal menu allo stato di gioco;
- `P`: mette in pausa o riprende gli aggiornamenti logici;
- `N`: avanza di un singolo tick mentre il gioco è in pausa;
- `Q`: passa allo stato di uscita e chiude il programma.

L'overlay mostra stato, FPS, tick/s, tick totali, tick eseguiti nell'ultimo
frame e fattore di interpolazione. Il rendering resta attivo durante la pausa;
il testo diagnostico usa campi a larghezza fissa e viene aggiornato quattro
volte al secondo per evitare sfarfallii, senza rallentare il game loop.

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
la pausa, il single-step, le transizioni di stato e il limite applicato ai frame
eccezionalmente lunghi.

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
│   └── tests/
│       └── test_main.c
└── README.md
```

Tutto il codice C, inclusi i test e i futuri header privati, risiede sotto
`src/`. Il progetto non espone una directory `include/` pubblica.

## Licenza

Il codice di GTG è distribuito con licenza MIT. raylib conserva la propria
licenza zlib/libpng.
