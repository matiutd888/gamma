/** @file
 * Zawiera implementację interfejsu interactive_input_util.h.
 * Kod zaczerpnięty z forum @p stackoverflow z wątku pod adresem:
 * https://stackoverflow.com/questions/33025599/move-the-cursor-in-a-c-program.
 * Dokonane zostały na nim pewne modyfikacje.
 *
 * @author David Ranieri https://stackoverflow.com/users/1606345/david-ranieri
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @date 12.05.2020
 */

#include "interactive_input_util.h"
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define A 'A' ///< Reprezentuje znak @p 'A'.
#define B 'B' ///< Reprezentuje znak @p 'B'.
#define C 'C' ///< Reprezentuje znak @p 'C'.
#define D 'D' ///< Reprezentuje znak @p 'D'.
#define BRACKET '[' ///< Reprezentuje znak @p '['.
#define TERMINAL_FAILURE -1 /**< Kod zwracany przez funkcje @p tcgetattr i @p tcsetattr
                             * w razie niepowodzonia.
                             */
#define EXIT_CODE_FAILURE 1 /**< Kod, z którym jest kończony program
                             * w razie krytycznego błędu.
                             */

static struct termios term; /**< Przechowuje ustawienia terminala
                             * podczas trybu interaktywnego.
                             */
static struct termios oterm; ///< Przechowuje domyślne ustawienia terminala.

/**
 * @brief Kończy program z kodem @p 1.
 */
static void terminate() {
    exit(EXIT_CODE_FAILURE);
}

void setup_terminal() {
    if (tcgetattr(STDIN_FILENO, &oterm) == TERMINAL_FAILURE)
        terminate();
    memcpy(&term, &oterm, sizeof(term));
    term.c_lflag &= ~(ICANON | ECHO);
}

void restore_terminal() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &oterm) == TERMINAL_FAILURE)
        terminate();
}

/**
 * @brief Wczytuje jeden znak w trybie niekanonicznym.
 * Czeka dopóki znak nie pojawi się w strumieniu wejściowym.
 * @return Kod reprezentujący wczytany znak.
 */
static int getch() {
    term.c_cc[VMIN] = 1;
    term.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == TERMINAL_FAILURE)
        terminate();
    int c = getchar();
    return c;
}

/**
 * @brief Wczytuje znak i umieszcza go z powrotem w strumieniu wejściowym.
 * Wczytuje w trybie niekanonicznym znak z flagą @p VTIME ustawioną
 * na wartość @p 1 (to sprawia, że czeka tylko jedną dziesiątą sekundy na dane ze
 * strumienia wejściowego). Jeżeli uda się taki znak wczytać, umieszcza go
 * z powrotem w strumieniu wejściowym używając @p ungetc.
 * @return Wartość @p 1, jeśli udało się wczytać znak, wartość
 * @p 0 w przeciwnym przypadku.
 */
static int kbhit() {
    term.c_cc[VMIN] = 0;
    term.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == TERMINAL_FAILURE)
        terminate();
    int c = getchar();
    if (c != -1) ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

/**
 * @brief Rozpoznaje klawisz kryjący się za sekwencją znaków.
 * Funkcja powinna być wywoływana, gdy wczytany został znak o kodzie
 * @ref KEY_ESCAPE. Funkcja sprawdza, czy następny wczytany znak
 * jest wczytywany "od razu". Jeżeli nie, to znak @ref KEY_ESCAPE
 * nie jest początkiem sekwencji znaków reprezentującej wciśnięcie
 * innego klawisza, więc funkcja zwraca wartość @ref KEY_ESCAPE.
 * Następnie funkcja sprawdza, czy następny
 * (wczytany od razu) znak ma kod @ref BRACKET.
 * Jeżeli tak, funkcja sprawdza, czy kodem znaku następującym po nim
 * jest któryś z @ref A, @ref B, @ref C lub @ref D. Jeżeli tak, to
 * wciśnięty został klawisz strzałki, i zwracany jest odpowiedni kod.
 * Jeżeli znak wczytany od razu po znaku o kodzie @ref KEY_ESCAPE
 * nie ma kodu równego @ref BRACKET funkcja wczytuje wszystkie
 * 'wczytywane od razu' znaki i zwraca wartość @p 0.
 * @return Kod odpowiadający klawiszowi kryjącemu się za sekwencją znaków
 * następującą po wczytanym znaku @ref KEY_ESCAPE lub wartość @p 0,
 * jeżeli nie rozpoznano żadnego klawisza.
 */
static int kbesc() {
    if (!kbhit()) return KEY_ESCAPE;
    int c = getch();
    if (c == BRACKET) {
        switch (getch()) {
            case A:
                c = KEY_UP;
                break;
            case B:
                c = KEY_DOWN;
                break;
            case C:
                c = KEY_LEFT;
                break;
            case D:
                c = KEY_RIGHT;
                break;
            default:
                c = 0;
                break;
        }
    } else {
        c = 0;
    }
    if (c == 0) while (kbhit()) getch();
    return c;
}

int kbget() {
    int c;
    c = getch();
    return (c == KEY_ESCAPE) ? kbesc() : c;
}
