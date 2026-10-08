# Symbole automatu biomolekularnego

Kod do rozdziału 3 pracy magisterskiej. Weryfikuje opublikowane
rozwiązanie i wyznacza własne.

## Budowanie

    make          # wszystko do bin/
    make check    # buduje i uruchamia wszystkie testy + weryfikację
    make clean    # sprząta build/ i bin/

Wymagania: `g++` z obsługą C++17 i `make`.

## Układ katalogów

    src/     biblioteka — logika, bez funkcji main
    tools/   programy użytkowe, jeden plik = jeden program w bin/
    tests/   testy, jeden plik = jeden program w bin/
    data/    dane wejściowe — opublikowane tabele symboli
    build/   pliki pośrednie .o i .d (tworzone automatycznie)
    bin/     gotowe programy (tworzone automatycznie)

Nowy program dodaje się przez wrzucenie pliku z `main` do `tools/`
albo `tests/` — `Makefile` znajduje go sam, nic nie trzeba dopisywać.

## Co jest czym

### src/ — biblioteka

| plik | zawartość |
|---|---|
| `dna.*` | słowa nad alfabetem nukleotydów: Xi, rho, orbity, rozkład symbolu na drogę w B(4,4) |
| `debruijn.*` | alfabet z dopełnieniem (wspólny też dla `dna`), graf B(k,n) dla dowolnego parzystego k, orbity, ograniczenie na niezrównoważenie, analiza podgrafu |
| `conditions.*` | warunki dopuszczalności z rozdziału 2, jeden predykat na jeden warunek |
| `exact_cover.*` | solver dokładnego pokrycia (Algorytm X, dancing links) |
| `junctions.*` | podziały styku i wynikające z nich filtry kandydatów (sekcja 3.4) |
| `cover_model.*` | budowa macierzy pokrycia: orbity jako kolumny, drogi jako wiersze (sekcja 3.3) |
| `search.*` | pierwsze rozwiązanie, zliczanie i przeszukiwanie wszystkich konfiguracji na wątkach |
| `walk_cover.*` | drogi długości 3 w B(4,n) wykorzystujące każdą orbitę dokładnie raz: losowe restarty i naprawianie częściowych pokryć |
| `symbol_io.*` | wczytywanie listy symboli z pliku |
| `report.hpp` | wypisywanie wyników, oddzielone od logiki sprawdzania |

### tools/ — programy

| program | działanie |
|---|---|
| `bin/verifier` | sprawdza `data/waldmajer_sakowski_2019.txt`; z argumentem sprawdza dowolny plik |
| `bin/candidates` | zlicza dopuszczalne drogi długości 3 (kandydatów) |
| `bin/generator` | szuka zbioru 40 symboli; `--count N` zlicza rozwiązania w jednej konfiguracji; `--all [s] [wątki]` przeszukuje wszystkie 256 konfiguracji przez zadany czas (domyślnie 300 s, do 8 wątków) |
| `bin/graph_analysis` | struktura B(4,4); z plikiem analizuje podgraf rozwiązania; `--table` pokazuje rodzinę B(k,n) |
| `bin/walk_family` | szuka takiej rodziny dróg w B(4,n) (domyślnie n = 6) i zapisuje ją w `data/`; `--verify plik [n]` sprawdza zapisaną rodzinę |

### tests/ — testy

Każdy moduł biblioteki ma swój program testowy (`bin/test_<moduł>`),
a `make check` uruchamia je wszystkie, a potem weryfikator. Testy nie
czytają plików, więc działają z dowolnego katalogu, i nie zależą od
szybkości komputera.

| program | co sprawdza |
|---|---|
| `test_dna` | operacje na słowach, na przykładach z rozdziału 2 |
| `test_conditions` | każdy warunek przyjmuje poprawny zbiór i odrzuca zepsuty |
| `test_exact_cover` | solver na przykładzie Knutha, termin, odrzucanie złych wierszy |
| `test_junctions` | podziały styku w kolejności z Tabeli 3.1, znaczenie zakazów |
| `test_cover_model` | liczby kandydatów z rozdziału 3 (3336, 3324, 3306) |
| `test_search` | 40 symboli dla wyboru 204, liczenie każdego zbioru dokładnie raz |
| `test_debruijn` | liczby z rozdziału 4, w tym ograniczenie 12 i wzór k^(n/2) − k, analiza podgrafu |
| `test_symbol_io` | wczytywanie danych i zgłaszanie błędnych linii |
| `test_walk_cover` | budowa instancji dla B(4,4) i B(4,6), restarty, naprawianie, sprawdzanie rodzin |

### data/ — dane

| plik | zawartość |
|---|---|
| `waldmajer_sakowski_2019.txt` | 40 symboli z Tabeli 4 pracy źródłowej (obie nici) |
| `generated.txt` | 40 symboli wyznaczonych przez `bin/generator` |
| `walk_cover_n6.txt` | 672 drogi w B(4,6) wykorzystujące każdą z 2016 orbit dokładnie raz |

## Format pliku z symbolami

Jeden symbol w linii. Puste linie i zaczynające się od `#` są pomijane.
Druga kolumna (nić komplementarna) jest opcjonalna; jeśli jest w każdym
wierszu, weryfikator sprawdza, czy równa się Xi(nić 1).

    # przyklad
    TCGCTA
    CGTTCG  GCAAGC

Użycie: `./bin/verifier moje_symbole.txt`
