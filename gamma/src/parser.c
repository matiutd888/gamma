/** @file
 * Zawiera implementację interfejsu parser.h
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.05.2020
 */

#define _GNU_SOURCE ///< Makro potrzebne do prawidłowej kompilacji.

#include "parser.h"
#include "gamma.h"
#include "interactive.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define GETLINE_FAILURE -1 /**< Stała zwracana przez @p getline,
                            * gdy nie uda się wczytać linii.
                            */
#define IGNORE_LINE '#' /**< Znak oznaczający ignorowanie linii,
                         * w której jest pierwszym znakiem.
                         */
#define M 'm' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_move.
#define G 'g' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_golden_move.
#define P 'p' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_board.
#define F 'f' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_free_fields.
#define Q 'q' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_golden_possible.
#define SMALL_B 'b' ///< Pierwszy znak w poleceniu wywołującym @ref gamma_busy_fields.
#define B 'B' ///< Pierwszy znak w poleceniu rozpoczynającym tryb wsadowy.
#define I 'I' ///< Pierwszy znak w poleceniu rozpoczynającym tryb interaktywny.
#define ULL unsigned long long ///< Makro definiujące typ @p unsigned @p long @p long.
#define DEAFULT_LINE_SIZE 2 ///< Początkowy rozmiar bufora.
#define MAX_PARAMETER_COUNT 4 ///< Maksymalna liczba parametrów w poleceniu
#define END_LINE '\n' ///< Znak nowej linii.
#define CHAR_INCORRECT -1 /**< Zwracana przez @ref get_parameters_count_from_char,
                           * gdy znak jest niepoprawnym pierwszym znakiem polecenia.
                           */

/**
 * @brief Wyświetla komunikat @p ERROR @p line.
 * Komunikat jest zakończony znakiem nowej linii.
 * Wypisywany jest na @p stderr.
 * @param[in] line – numer wiersza.
 */
static void show_error_message(ULL line) {
    fprintf(stderr, "ERROR %llu\n", line);
}

/**
 * @brief Wyświetla komunikat @p OK @p line.
 * Komunikat jest zakończony znakiem nowej linii.
 * @param[in] line – numer wiersza.
 */
static void show_ok_message(ULL line) {
    printf("OK %llu\n", line);
}

/**
 * @brief Sprawdza obecność niepoprawnych znaków.
 * Iteruje po znakach łańcucha znaków @p line zakończnonego
 * znakiem @ref END_LINE. Szuka, czy w łańcuchu występują znaki niebędące
 * białymi znakami lub cyframi.
 * @param line – łańcuch znaków zakończony znakiem @ref END_LINE.
 * @return Wartość @p true, jeśli wszystkie znaki w łańcuchu są poprawne
 * (tzn wszystkie znaki są białymi znakami lub cyframi), wartość
 * @p false w przeciwnym przypadku.
 */
static bool check_for_broken_chars(char *line) {
    int it = 0;
    while (line[it] != END_LINE) {
        if (!isdigit(line[it]) && !isspace(line[it]))
            return false;
        it++;
    }
    return true;
}

/**
 * @brief Wczytuje liczby, których zapis dziesiętny występuje w łańcuchu.
 * Odczytane liczby zapisuje w tablicy @p numbers.
 * Zakłada, że występuje dokładnie @p parameters_count liczb w łańcuchu i że
 * oddzielone są one białymi znakami. Pierwszy znak w łańcuchu musi być biały.
 * Liczby występujące w łańcuchu powinny być nieujemne i mieścić się w
 * zakresie zmiennej typu @p uint32_t.
 * @param[in] line              – łańcuch znaków zawierający zapis
 *                                dziesiętny liczb,
 * @param[in] parameters_count  – spodziewana liczba liczb w łańcuchu,
 * @param[out] numbers          – dynamiczna tablica, do której zapisywane będą
 *                                wczytane liczby, o rozmiarze niemniejszym niż
 *                                @p parameters_count.
 * @return Wartość @p true jeśli konwersja się udała i wszystkie założenia były
 * spełnione, wartość @p false w przeciwnym przypadku.
 */
static bool read_numbers(char *line, int parameters_count, uint32_t *numbers) {
    if (!isspace(line[0]))
        return false;
    if (!check_for_broken_chars(line))
        return false;
    char delimit[] = " \t\r\n\v\f";
    char *token = NULL;
    token = strtok(line, delimit);
    int it = 0;
    while (token != NULL) {
        if (it > parameters_count - 1) {
            return false;
        } else {
            errno = 0;
            uint64_t wynik = strtoul(token, NULL, 10);
            if (errno == ERANGE || wynik > UINT32_MAX)
                return false;
            numbers[it] = wynik;
            it++;
            token = strtok(NULL, delimit);
        }
    }
    if (it != parameters_count)
        return false;
    return true;
}

/**
 * @brief Tworzy grę dla danych parametrów.
 * Wywołuje polecenie @ref gamma_new z parametrami przechowywanymi
 * w pierwszy czterych komórkach dynamicznej tablicy @p numbers.
 * @param[in] numbers – dynamiczna tablica przechowująca rozmiaru
 *                      niemniejszego niż 4.
 * @return Wynik polecenia @ref gamma_new dla podanych parametrów.
 */
static gamma_t *create_gamma_from_numbers(uint32_t *numbers) {
    gamma_t *g = gamma_new(numbers[0], numbers[1], numbers[2], numbers[3]);
    return g;
}

/**
 * @brief Znajduje liczbę parametrów dla danego pierwszego znaku polecenia.
 * Rozważa poprawne pierwsze znaki poleceń w trybie wsadowym (czyli znak 'B'
 * nie jest poprawnym pierwszym znakiem).
 * @param[in] c – pierwszy znak polecenia.
 * @return Liczba parametrów w poleceniu, którego pierwszym znakiem jest @p c,
 * jeżeli @p c jest pierwszym znakiem polecenia akceptowanego po przejściu w 
 * tryb wsadowy, wartość @ref CHAR_INCORRECT w przeciwnym przypadku.
 */
static int get_parameters_count_from_char(char c) {
    switch (c) {
        case M:
            return 3;
        case G:
            return 3;
        case SMALL_B:
        case F:
        case Q:
            return 1;
        case P:
            return 0;
        default:
            return CHAR_INCORRECT;
    }
}

/**
 * @brief Wypisuje zmienną typu @p bool.
 * Jeżeli wartość zmiennej to @p true, wypisuje @p 1, w przeciwnym wypadku
 * wypisuje @p 0. Napis reprezentujący zmienną zakończony jest znakiem
 * nowej linii.
 * @param[in] b – zmienna typu @p bool.
 */
static void print_bool(bool b) {
    printf("%d\n", b);
}

/**
 * @brief Wypisuje liczbę typu @p uint64_t i znak nowej linii.
 * @param[in] u – liczba typu @p uint64_t.
 */
static void print_uint64_t(uint64_t u) {
    printf("%lu\n", u);
}

/**
 * @brief Wypisuje planszę.
 * Wywołuje @ref gamma_board na parametrze @p g i zapisuje wynik do zmiennej.
 * Jeśli wywołanie @p gamma_board zwróciło wartość @p NULL, wypisuje
 * komunikat o błędzie (@p ERROR @p line), w przeciwnym wypadku wypisuje
 * wynik @p gamma_board i zwalnia zaalokowaną pamięć.
 * @param[in] g          – wskaźnik na strukturę reprezentującą stan gry,
 * @param[in] line_count – numer linijki, w której wystąpiło polecenie,
 *                         wywołujące @p gamma_board (polecenie @p p w trybie
 *                         wsadowym).
 */
static void print_board(gamma_t *g, ULL line_count) {
    char *s = gamma_board(g);
    if (s == NULL) {
        show_error_message(line_count);
        return;
    }
    printf("%s", s);
    free(s);
}

/**
 * @brief Wykonuje polecenie w trybie wsadowym.
 * Wykonuje polecenie, którego pierwszym znakiem jest @p c, po którym
 * to znaku wystąpiły liczby opisane w tablicy @p num. Polecenie ma być
 * wykonane na grze, której stan reprezentuje struktura wskazywana przez @p g.
 * Wypisuje odpowiedni napis na ekranie.
 * @param[in] c           – pierwszy znak polecenia,
 * @param[in] num         – liczby występujące po znaku,
 * @param[in,out] g       – wskaźnik na strukturę reprezentującą stan gry, różny
 *                          od @p NULL.
 * @param[in] line_count  – numer linii, w której wystąpiło polecenie.
 */
static void deal_with_query(char c, uint32_t *num, gamma_t *g, ULL line_count) {
    switch (c) {
        case M:
            print_bool(gamma_move(g, num[0], num[1], num[2]));
            break;
        case G:
            print_bool(gamma_golden_move(g, num[0], num[1], num[2]));
            break;
        case SMALL_B:
            print_uint64_t(gamma_busy_fields(g, num[0]));
            break;
        case F:
            print_uint64_t(gamma_free_fields(g, num[0]));
            break;
        case Q:
            print_bool(gamma_golden_possible(g, num[0]));
            break;
        case P:
            print_board(g, line_count);
    }
}

/**
 * @brief Tworzy grę na podstawie parametrów w występujących łańcuchu znaków.
 * Najpierw wyodrębnia @ref MAX_PARAMETER_COUNT parametrów i zapisuje je
 * do @p numbers z użyciem funkcji @ref read_numbers,
 * następnie dla wyodrębnionych parametrów wywołuje funkcję @p gamma_new
 * i zwraca jej wynik.
 * @param[in] line     – łańcuch znaków zawierający parametry do wywołania
 *                       funkcji @p gamma_new, o pierwszym znaku
 *                       będącym białym znakiem.
 * @param[out] numbers – tablica, do której będą zapisywane odczytywane
 *                       parametry, o rozmiarze niemniejszym niż
 *                       @ref MAX_PARAMETER_COUNT.
 * @return Wartość zwrócona przez @p gamma_new dla odczytanych parametrów, lub
 * wartość @p NULL jeśli nie udało odczytać sie dokładnie
 * @ref MAX_PARAMETER_COUNT  parametrów z łańcucha @p line.
 */
static gamma_t *create_gamma_from_line(char *line, uint32_t *numbers) {
    if (!read_numbers(line, MAX_PARAMETER_COUNT, numbers))
        return NULL;
    gamma_t *g = create_gamma_from_numbers(numbers);
    return g;
}

/**
 * @brief Przeprowadza tryb wsadowy.
 * Zakłada, że bufor wskazywany przez @p line_ptr ma pierwszy
 * znak równy @ref B i jest zakończony znakiem nowej linii.
 * Tworzy grę opisaną poleceniem wskazywanym przez @p line_ptr
 * występującym w liniii numer @p line_count.
 * Jeżeli nie udało się tego zrobić, zwraca wartość @p false.
 * W przeciwnym wypadku przeprowadza tryb wsadowy, tzn wczytuje polecenia w
 * kolejnych wierszach, wykonuje funkcje, które te polecenia wywołują
 * na strukturze gry, która została stworzona poleceniem powodującym
 * przejście do trybu wsadowego i w zależności od wyniku tych funkcji
 * wypisuje odpowiednie napisy na wyjściu.
 * Gdy skończy się strumień wejściowy funkcja wywołuje
 * funkcję @ref gamma_delete na zmiennej wskazującej na strukturę gry i zwraca
 * @p true.
 * @param[in,out] line_ptr   – wskaźnik na bufor przechowujący wczytane polecenie,
 *                             mające powodować przejście do trybu wsadowego;
 *                             bufor powinien mieć pierwszy znak równy @ref B
 *                             i być zakończony znakiem nowej linii.
 * @param[in] line_count     – numer wiersza, w którym wystąpiło polecenie
 *                             @p line.
 * @return Wartość @p false, jeśli nie udało się przejść do trybu wsadowego,
 * wartość @p true w przeciwnym przypadku.
 */
static bool tryb_wsadowy(char **line_ptr, ULL line_count) {
    uint32_t numbers[MAX_PARAMETER_COUNT];
    gamma_t *g = create_gamma_from_line(&(*line_ptr)[1], numbers);
    if (g == NULL)
        return false;
    show_ok_message(line_count);
    bool czy_koniec_programu = false;
    line_count++;
    size_t rozmiar = DEAFULT_LINE_SIZE;
    while (!czy_koniec_programu) {
        ssize_t len = getline(line_ptr, &rozmiar, stdin);
        if (len == GETLINE_FAILURE) {
            czy_koniec_programu = true;
        } else {
            bool good_line = true, ignore = false;
            char c = (*line_ptr)[0];
            if (c == IGNORE_LINE || c == END_LINE) {
                good_line = false;
                ignore = true;
            }
            if (good_line && (*line_ptr)[len - 1] != END_LINE)
                good_line = false;
            int param_count = get_parameters_count_from_char(c);
            if (good_line && param_count == CHAR_INCORRECT)
                good_line = false;
            if (good_line && read_numbers(&(*line_ptr)[1], param_count, numbers))
                deal_with_query(c, numbers, g, line_count);
            else good_line = false;
            if (!good_line && !ignore)
                show_error_message(line_count);
            line_count++;
        }
    }
    gamma_delete(g);
    return true;
}

/**
 * @brief Przeprowadza tryb interaktywny.
 * Zakłada, że łańcuch wskazywany przez @p line_ptr ma 
 * pierwszy znak równy @ref I i jest zakończony znakiem nowej linii.
 * Tworzy grę na podstawie polecenia zapisanego w łańcuchu wskazywanym
 * przez @p line_ptr. Jeżeli nie uda się stworzyć gry lub przejść w tryb
 * interaktywny (np. nie da się wyświetlić planszy na ekranie z
 * powodu jej rozmiarów) zwraca @p false. W przeciwnym przypadku przechodzi
 * w tryb interaktywny, i gdy ów tryb się skończy zwraca wartość @p true.
 * @param[in,out] line_ptr – wskaźnik na zmienną zawierającą
 *                           łańcuch znaków zawierający polecenie wywołujące
 *                           tryb interaktywny. Pierwszy znak łańcucha
 *                           powinien być równy @ref I, a sam łańcuch
 *                           zakończony powinien być znakiem nowej linii.
 * @return Wartość @p false, jeśli nie udało się dla danego polecenia
 * przejść w tryb interaktywny, wartość @p true w przeciwnym przypadku.
 */
static bool tryb_interaktywny(char **line_ptr) {
    uint32_t numbers[MAX_PARAMETER_COUNT];
    gamma_t *g = create_gamma_from_line(&(*line_ptr)[1], numbers);
    if (g == NULL)
        return false;
    if (!interactive(g, numbers)) {
        gamma_delete(g);
        return false;
    }
    gamma_delete(g);
    return true;
}

void read_input() {
    bool czy_wybrany_tryb = false;
    char *line = NULL;
    size_t rozmiar = 0;
    ULL line_count = 1;
    while (!czy_wybrany_tryb) {
        bool ignore = false;
        bool good_line = true;

        ssize_t len = getline(&line, &rozmiar, stdin);
        if (len == GETLINE_FAILURE)
            break;
        char c = line[0];
        if (c == IGNORE_LINE || c == END_LINE) {
            ignore = true;
            good_line = false;
        }
        if (good_line && line[len - 1] != END_LINE)
            good_line = false;
        if (good_line && c == B)
            czy_wybrany_tryb = tryb_wsadowy(&line, line_count);
        if (good_line && c == I)
            czy_wybrany_tryb = tryb_interaktywny(&line);
        if ((!good_line || !czy_wybrany_tryb) && !ignore)
            show_error_message(line_count);
        line_count++;
    }
    free(line);
}
