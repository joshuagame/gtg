# GTG — Guerra Termonucleare Globale

GTG è un gioco strategico didattico scritto in C17 con raylib, ispirato
all'estetica dei terminali WOPR del film *WarGames*.

Il progetto è sviluppato per round incrementali. Il Round 01 fornisce una base
compilabile, riproducibile e priva di logica di gioco; la finestra raylib e il
ciclo di vita grafico verranno introdotti nel Round 02.

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

Nel Round 01 il programma stampa un messaggio di conferma e termina senza
aprire una finestra.

## Test

```sh
ctest --preset debug
```

Il test di bootstrap è indipendente da raylib e non richiede un contesto
grafico.

## Struttura iniziale

```text
.
├── CMakeLists.txt
├── CMakePresets.json
├── assets/
├── src/
│   ├── main.c
│   └── tests/
│       └── test_main.c
└── README.md
```

Tutto il codice C, inclusi i test e i futuri header privati, risiede sotto
`src/`. Il progetto non espone una directory `include/` pubblica.

## Licenza

Il codice di GTG è distribuito con licenza MIT. raylib conserva la propria
licenza zlib/libpng.
