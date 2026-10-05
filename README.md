# GTG — Guerra Termonucleare Globale

GTG è un gioco strategico didattico scritto in C17 con raylib, ispirato
all'estetica dei terminali WOPR del film *WarGames*.

Il progetto è sviluppato per round incrementali. Il Round 02 introduce la
finestra raylib, una configurazione validata e il ciclo di vita esplicito delle
risorse; la logica di gioco verrà aggiunta nei round successivi.

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

Per aprire la finestra, disegnare un solo frame e terminare automaticamente:

```sh
./build/debug/gtg --smoke-test
```

## Test

```sh
ctest --preset debug
```

I test del ciclo di vita usano un backend finto: verificano inizializzazione,
esecuzione ripetuta e cleanup degli errori senza richiedere un contesto grafico.

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
