/** @file
 * Interfejs klasy przechowującej stan gry gamma
 *
 * @author Marcin Peczarski <marpe@mimuw.edu.pl>
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 18.03.2020
 */

#ifndef GAMMA_H
#define GAMMA_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Struktura przechowująca stan gry.
 */
typedef struct gamma gamma_t;

/** @brief Tworzy strukturę przechowującą stan gry.
 * Alokuje pamięć na nową strukturę przechowującą stan gry.
 * Inicjuje tę strukturę tak, aby reprezentowała początkowy stan gry.
 * @param[in] width   – szerokość planszy, liczba dodatnia,
 * @param[in] height  – wysokość planszy, liczba dodatnia,
 * @param[in] players – liczba graczy, liczba dodatnia,
 * @param[in] areas   – maksymalna liczba obszarów,
 *                      jakie może zająć jeden gracz, liczba dodatnia.
 * @return Wskaźnik na utworzoną strukturę lub NULL, gdy nie udało się
 * zaalokować pamięci lub któryś z parametrów jest niepoprawny.
 */
gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas);

/** @brief Usuwa strukturę przechowującą stan gry.
 * Usuwa z pamięci strukturę wskazywaną przez @p g.
 * Nic nie robi, jeśli wskaźnik ten ma wartość NULL.
 * @param[in] g       – wskaźnik na usuwaną strukturę.
 */
void gamma_delete(gamma_t *g);

/** @brief Wykonuje ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y).
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/** @brief Wykonuje złoty ruch.
 * Ustawia pionek gracza @p player na polu (@p x, @p y) zajętym przez innego
 * gracza, usuwając pionek innego gracza.
 * @param[in,out] g   – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch został wykonany, a @p false,
 * gdy gracz wykorzystał już swój złoty ruch, ruch jest nielegalny
 * lub któryś z parametrów jest niepoprawny.
 */
bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/** @brief Podaje liczbę pól zajętych przez gracza.
 * Podaje liczbę pól zajętych przez gracza @p player.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól zajętych przez gracza lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_busy_fields(gamma_t *g, uint32_t player);

/** @brief Podaje liczbę pól, jakie jeszcze gracz może zająć.
 * Podaje liczbę wolnych pól, na których w danym stanie gry gracz @p player może
 * postawić swój pionek w następnym ruchu.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Liczba pól, jakie jeszcze może zająć gracz lub zero,
 * jeśli któryś z parametrów jest niepoprawny.
 */
uint64_t gamma_free_fields(gamma_t *g, uint32_t player);

/** @brief Sprawdza, czy gracz może wykonać złoty ruch.
 * Sprawdza, czy istnieje pole, na które gracz
 * @p player może wykonać złoty ruch w przy obecnym stanie gry,
 * nie naruszając przy tym zasad.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli gracz jeszcze nie wykonał w tej rozgrywce
 * złotego ruchu i istnieje pole, na które mógłby przy obecnym stanie planszy
 * wykonać złoty ruch, wartość @p false w przeciwnym przypadku.
 */
bool gamma_golden_possible(gamma_t *g, uint32_t player);

/** @brief Daje napis opisujący stan planszy.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Przykład znajduje się w pliku gamma_test.c.
 * Funkcja wywołująca musi zwolnić ten bufor.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *gamma_board(gamma_t *g);

/**
 * @brief Zwraca napis reprezentujący planszę zgodną z konwencją przyjętą
 * w wyświetlaniu planszy w trybie interaktywnym.
 * Alokuje w pamięci bufor, w którym umieszcza napis zawierający tekstowy
 * opis aktualnego stanu planszy. Owy napis jest w niektórych
 * przypadkach różny od tego, zwracanego przez @ref gamma_board.
 * Przyjmuje stałą szerokość napisowej reprezentacji pola w napisie
 * reprezentującym planszę, która nie jest zależna od tego, czy gracze o
 * maksymalnej liczbie cyfr w zapisie dziesiętnym ich numerów zrobili ruch
 * czy nie. Różni się tym od zwykłego @ref gamma_board. Dla gry na dziesięciu
 * graczy w której gracz o numerze @p 10 nie zrobił ruchu, @p gamma_board zwróci
 * planszę, w której szerokość napisowej reprezentacji pola jest równa @p 1,
 * a @p gamma_board_interactive zwróci planszę, w której szerokość napisowej
 * reprezentacji pola jest równa @p 3 (ponieważ maksymalny numer - @p 10 ma
 * dwie cyfry zapisu dziesiętnego, do tego dochodzi jeden znak na oddzielanie
 * liczb w polach spacją).
 * @param[in] g – wskaźnik na strukturę reprezentującą stan gry.
 * @return Wskaźnik na zaalokowany bufor zawierający napis opisujący stan
 * planszy lub NULL, jeśli nie udało się zaalokować pamięci.
 */
char *gamma_board_interactive(gamma_t *g);

/**
 * @brief Zwraca szerokosć pola w napisowej reprezentacji
 * planszy w trybie interaktywnym.
 * Liczy liczbę cyfr dziesiętnej reprezentacji liczby graczy w grze,
 * której stany reprezentowany jest przez @p g.
 * @param[in] g – wskaźnik na strukturę reprezentującą stan gry,
 *                różny od @p NULL.
 * @return Wartość @p 1, jeśli dziesiętna reprezentacja liczby graczy
 * jest jednocyfrowa, w przeciwnym przypadku ilość cyfr w dziesiętnej
 * reprezentacji liczby graczy @p + @p 1.
 */
uint32_t gamma_field_width_interactive(gamma_t *g);

/**
 * @brief Wpisuje do łańcucha znaków napisową reprezentację pola.
 * Do bufora @p s o rozmiarze nie mniejszm niż wartość zwracana przez
 * @ref gamma_field_width_interactive wywołanym dla parametru @p g,
 * wpisuje napisową reprezentację pola o współrzędnych (@p x, @p y)
 * na planszy w grze której stan reprezentuje struktura wskazywana przez
 * @p g. Napisowa reprezentacja jest uzupełniana spacjami tak, by do
 * bufora @p s zostało zapisane tyle znaków ile wynosi wartość zwrócona przez
 * @ref gamma_field_width_interactive wywołana dla parametru @p g.
 * @param[in] g – wskaźnik na strukturę reprezentującą stan gry,
 *                różny od @p NULL,
 * @param[in] s – bufor o rozmiarze nie mniejszym niż wartość zwracana przez
 *                @ref gamma_field_width_interactive wywołanym dla parametru
 *                @p g.
 * @param[in] x – numer kolumny, liczba nieujemna mniejsza od wartości
 *                @p width z funkcji @ref gamma_new,
 * @param[in] y – numer wiersza, liczba nieujemna mniejsza od wartości
 *                @p height z funkcji @ref gamma_new.
 */
bool gamma_write_field(gamma_t *g, char *s, uint32_t x, uint32_t y);

/**
 * @brief Sprawdza, czy gracz może wykonać ruch na danym polu.
 * Sprawdza, czy gracz o numerze @p player może wykonać ruch na polu
 * (@p x, @p y) w grze, której stan reprezentowany jest przez strukturę
 * wskazywaną przez @p g.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @param[in] x       – numer kolumny, liczba nieujemna mniejsza od wartości
 *                      @p width z funkcji @ref gamma_new,
 * @param[in] y       – numer wiersza, liczba nieujemna mniejsza od wartości
 *                      @p height z funkcji @ref gamma_new.
 * @return Wartość @p true, jeśli ruch może zostać wykonany, a @p false,
 * gdy ruch jest nielegalny lub któryś z parametrów jest niepoprawny.
 */
bool gamma_move_possible(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

/**
 * @brief Zwraca liczbę obszarów tworzonych przez pola zajmowane
 * przez gracza.
 * Zwraca liczbę obszarów tworzonych przez pola zajmowane przez gracza
 * @p player w grze, której stan reprezentowany jest przez strukturę
 * wskazywaną przez @p g.
 * @param[in] g       – wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  – numer gracza, liczba dodatnia niewiększa od wartości
 *                      @p players z funkcji @ref gamma_new,
 * @return Liczba obszarów tworzonych przez pola zajmowane przez gracza
 * @p player, lub wartość @p 0, jeśli któryś z parametrów jest niepoprawny.
 */
uint32_t gamma_player_areas(gamma_t *g, uint32_t player);

/**
 * @brief Zwraca maksymalną liczbę obszarów w grze.
 * Zwraca wartośc parametru @p areas z funkcji @ref gamma_new,
 * która zainicjowała wskaźnik na strukturę gry @p g.
 * @param[in] g – wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość parametru @p areas z funkcji @ref gamma_new,
 * czyli inaczej maksymalna liczba obszarów na planszy w grze,
 * której stan reprezentuje struktura wskazywana przez @p g,
 * lub wartość @p 0, jeśli @p g ma wartość @p NULL.
 */
uint32_t gamma_max_areas(gamma_t *g);

#endif /* GAMMA_H */
