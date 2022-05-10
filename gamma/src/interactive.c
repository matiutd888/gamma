/** @file
 * Zawiera implementację trybu interaktywnego
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.05.2020
 */

#include "interactive.h"
#include "int_to_string_util.h"
#include "interactive_input_util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#define GOLDEN_POSSIBLE "G" /**< Zmienna napisowa wyświetlana
                             * w napisie reprezentującym gracza,
                             * który może w aktualnym ruchu wykonać
                             * @ref gamma_golden_move.
                             */
#define NO_PLAYER 0 ///< Reprezentuje brak gracza, który może wykonać ruch w turze.
#define SPACE ' ' ///< Znak spacji.
#define BIG_G 'G'///< Reprezentuje znak @p 'G'.
#define SMALL_G 'g' ///< Reprezentuje znak @p 'g'.
#define BIG_C 'C' ///< Reprezentuje znak @p 'C'.
#define SMALL_C 'c' ///< Reprezentuje znak @p 'c'.
#define CTRL_D 4 ///< Kod znaku wczytanego jako wciśnięcie @p CTRL-D.
#define IOCTL_ERROR -1 /**< Kod zwracany przez funkcję @p
                        * ioctl w razie niepowodzenia.
                        */

#define STRING_END '\0' ///< Znak końca łańcucha znaków.
#define WRONG_GOLDEN_MOVE "Golden move NOT possible!" /**< Komunikat wyświetlany
                                                       * w przypadku próby
                                                       * zrobienia nielegalnego
                                                       * złotego ruchu.
                                                       */
#define WRONG_MOVE "Move NOT possible!" /**< Komunikat wyświetlany w przypadku
                                         * próby zrobienia nielegalnego
                                         * ruchu.
                                         */

#define TOO_SMALL_TERMINAL "Terminal too small!" /**< Komunikat
                                                  * wyświetlany w przypadku
                                                  * próby przejścia w tryb
                                                  * interaktywny na terminalu
                                                  * o zbyt małym rozmiarze
                                                  * by pomieścić grę.
                                                  */

#define RED "\033[31m" ///< Kod aktywujący wypisywanie w kolorze czerwonym.
#define GREEN "\033[32m" ///< Kod aktywujący wypisywanie w kolorze zielonym.
#define YELLOW "\033[33m" ///< Kod aktywujący wypisywanie w kolorze żółtym.
#define RED_BACKGROUND "\033[41m" ///< Kod aktywujący wypisywanie na czerwonym tle.
#define GREEN_BACKGROUND "\033[42m" ///< Kod aktywujący wupisywanie na zielonym tle.

/**
 * @brief Przywraca domyślne ustawienia wyświetlanych napisów.
 */
static void reset_font() {
    printf("\033[0m");
}

/**
 * @brief Przesuwa kursor.
 * Przesuwa kursor na pole w kolumnie @p col i wierszu @p row.
 * @param[in] row – numer wiersza,
 * @param[in] col – numer kolumny.
 */
static void move_to(int row, int col) {
    printf("\033[%d;%df", row, col);
}

/**
 * @brief Chowa kursor.
 */
static void hide_cursor() {
    printf("\e[?25l"); /* hide the cursor */
}

/**
 * @brief Pokazuje kursor w terminalu
 */
static void show_cursor() {
    printf("\e[?25h");
}

/**
 * @brief Zapisuje pozycję kursora.
 */
static void save_cursor_position() {
    printf("\033[s");
}

/**
 * @brief Przywraca kursorowi pozycję zapisaną w ostatnim wywołaniu funkcji
 * @ref save_cursor_position.
 * Zakłada, że takie istnieje.
 */
static void restore_cursor_position() {
    printf("\033[u");
}

/**
 * Czyści ekran terminala i przesuwa kursor do punktu (@p 1, @p 1).
 */
static inline void clear_screen_move_to_1_1() {
    printf("\033[2J");
    move_to(1, 1);
}

/**
 * @brief Wypisuje napis reprezentujący gracza.
 * Wypisuje na aktualnej pozycji kursora napis reprezentujący
 * gracza o numerze @p player w grze reprezentowanej przez strukturę
 * wskazywaną przez @p g. Napis zawiera informację o numerze gracza,
 * liczbie pól zajętych przez gracza, liczbie pól, na które gracz może
 * aktualnie zrobić ruch i o tym, czy gracz może w zrobić w obecnej turze
 * zrobić złoty ruch. Napis jest zakończony znakiem nowej linii.
 * Napis jest koloru zielonego.
 * @param[in] g      – wskaźnik na strukturę reprezentującą stan gry,
 *                     różny od @p NULL.
 * @param[in] player – numer gracza.
 */
static void print_player(gamma_t *g, uint32_t player) {
    printf("%s", GREEN);
    uint64_t busy_fields = gamma_busy_fields(g, player);
    uint64_t free_fields = gamma_free_fields(g, player);
    uint32_t areas = gamma_player_areas(g, player);
    uint32_t max_areas = gamma_max_areas(g);
    bool golden_possible = gamma_golden_possible(g, player);
    printf("PLAYER %d, B: %lu, F: %lu, areas: %d\\%d",
           player,
           busy_fields,
           free_fields,
           areas, max_areas);
    if (golden_possible) {
        printf(" ");
        printf(YELLOW);
        printf(GOLDEN_POSSIBLE);
    }
    reset_font();
}

/**
 * @brief Wyświetla komunikat na dole ekranu.
 * Przenosi kursor na pozycję w pierwszej kolumnie i
 * @p rows @p + @p 2 wierszu. Wyświetla tam komunikat
 * @p s. Na końcu przywraca kursor do początkowej pozycji.
 * @param[in] s    – łańcuch znaków będący komunikatem do wyświetlenia,
 *                   powinien być zakończony znakiem końca łańcucha,
 * @param[in] rows – liczba wierszy w napisie reprezentującym
 *                   planszę w trybie interaktywnym.
 */
static void print_komunikat(char *s, uint32_t rows) {
    save_cursor_position();
    move_to(rows + 2, 1);
    printf("%s", s);
    restore_cursor_position();
}

/**
 * @brief Wypisuje napis reprezentujący gracza w podsumowaniu gry.
 * Wypisuje napis reprezentujący gracza o numerze @p player w grze,
 * której stan reprezentowany jest przez strukturę wskazywaną przez @p g.
 * Napis zawiera informację o numerze gracza i liczbie zajętych przez
 * gracza pól. Napis jest zakończony znakiem nowej linii.
 * @param[in] g      – wskaźnik na strukturę reprezentującą stan gry,
 *                     różny od @p NULL,
 * @param[in] player – numer gracza.
 */
static void print_player_game_end(gamma_t *g, uint32_t player) {
    uint64_t busy_fields = gamma_busy_fields(g, player);
    printf("PLAYER %d, Busy fields: %lu\n", player, busy_fields);
}

/**
 * @brief Inkrementuje numer gracza.
 * @param[in] player         – numer gracza, liczba dodatnia niewiększa od
 *                             @p num_of_players,
 * @param[in] num_of_players – liczba graczy.
 * @return Wartość @p 1, gdy liczba @p player jest rówma @p num_of_players,
 * Wartość @p player + 1 w przeciwnym przypadku.
 */
static uint32_t increment_player(uint32_t player, uint32_t num_of_players) {
    return player % num_of_players + 1;
}

/**
 * @brief Znajduje i zwraca numer następnego grającego gracza.
 * Sprawdza, który gracz powinien grać po graczu o numerze @p curr.
 * Przechodzi po kolei graczy zaczynając od tego, który powinien grać po
 * graczu o numerze @p curr i dla każdego sprawdza, czy w może wykonać ruch
 * (zwykły lub złoty) na planszy w grze, której stan reprezentuje
 * struktura wskazywana przez @p g.
 * @param[in] g        – wskaźnik na strukture reprezentującą stan gry,
 *                       różny od @p NULL,
 * @param[in] curr     – numer gracza, którego "następnika" szukamy,
 * @param[in] players  – liczba wszystkich graczy w grze (licząc tych, co
 *                       nie mogą wykonać ruchu).
 * @return Numer pierwszego gracza, który może wykonać ruch, lub
 * wartość @ref NO_PLAYER, jeśli żaden gracz nie może wykonać ruchu.
 */
static uint32_t next_playing_player(gamma_t *g, uint32_t curr, uint32_t players) {
    uint32_t next = increment_player(curr, players);
    if (gamma_free_fields(g, next) > 0 || gamma_golden_possible(g, next))
        return next;
    uint32_t it = increment_player(next, players);
    while (it != next) {
        if (gamma_free_fields(g, it) > 0 || gamma_golden_possible(g, it))
            return it;
        it = increment_player(it, players);
    }
    return NO_PLAYER;
}

/**
 * @brief Sprawdza, czy znak reprezentuję strzałkę.
 * @param[in] c – kod znaku.
 * @return Wartość @p true, jeśli znak o kodzie @p c reprezentuje
 * strzałkę, @p false w przeciwnym przypadku.
 */
static bool is_direction(int c) {
    return c == KEY_DOWN
           || c == KEY_UP
           || c == KEY_LEFT
           || c == KEY_RIGHT;
}

/**
 * @brief Zmienia zmienne reprezentujące położenie kursora.
 * W zależnosci od kodu wczytanego znaku @p c zmienia zmienne reprezentujące
 * położenie kursora. Funkcja nie sprawdza, czy po zmianie wartości zmiennych
 * są nadal poprawne (może np dojść do sytuacji, w której funkcja ustawia
 * wartość zmiennej na liczbę mniejszą od @p 0)
 * @param[in] c    – kod znaku,
 * @param[out] row – wskaźnik na zmienną reprezentują numer wiersza kursora,
 *                   różny od @p NULL,
 * @param[out] col – wskaźnik na zmienną reprezentującą numer kolumny kursora,
 *                   różny od @p NULL.
 */
static void change_coordinates(int c, int *row, int *col) {
    if (c == KEY_RIGHT) {
        (*col)--;
    } else if (c == KEY_LEFT) {
        (*col)++;
    } else if (c == KEY_DOWN) {
        (*row)++;
    } else if (c == KEY_UP) {
        (*row)--;
    }
}

/**
 * @brief Czyści linijkę, w której znajduje się kursor,
 * od początku kursora do końca linijki.
 */
static void erase_to_the_end_of_line() {
    printf("\033[K");
}

/**
 * @brief Wyświetla napisy reprezentujące graczy w podsumowaniu gry.
 * Przemieszcza kursor do wiersza pod wierszem numer @p row
 * i wyświetla napisy podsumowujące grę dla kolejnych graczy
 * (zawierające informację o numerze gracza i liczbie zajętych pól w
 * grze, której stan reprezentowany jest przez
 * strukturę wskazywaną przez @p g).
 * Napisy reprezentujące graczy wyświetlane są w kolorze zielonym.
 * @param[in] g       – wskaźnik na strukturę reprezentującą stan planszy,
 *                      różny @p NULL
 * @param[in] players – liczba wszystkich graczy w grze,
 * @param[in] row     – numer wiersza pod, którym mają zostać wyświetleni gracze.
 */
static void print_players_summary(gamma_t *g, uint32_t players, int row) {
    printf(GREEN);
    move_to(row + 1, 1);
    for (uint32_t i = 0; i < players; i++) {
        erase_to_the_end_of_line();
        print_player_game_end(g, i + 1);
    }
    reset_font();
}

/**
 * @brief Wyświetla napis reprezentujący gracza, którego jest teraz tura.
 * Czyści napis reprezentujący poprzedniego gracza i wyświetla nowy.
 * Napisy reprezentujące graczy wyświetlane są pod
 * napisem reprezentującym planszę, który to napis zajmuje @p row wierszy.
 * Są one zielonego koloru.
 * @param[in] g       – wskaźnik na strkutrę reprezentującą stan gry,
 *                      różny od @p NULL,
 * @param[in] p  – numer gracza, którego wyświetlany
 *                      napis ma reprezentować,
 * @param[in] row     – liczba wierszy.
 */
static void erase_and_print_player(gamma_t *g, uint32_t p, int row) {
    save_cursor_position();
    move_to(row + 1, 1);
    erase_to_the_end_of_line();
    print_player(g, p);
    restore_cursor_position();
}

/**
 * @brief Czyści wiersz o numerze @p rows @p + @p 2.
 * @param[in] rows – liczba wierszy w napisie reprezentującym planszę.
 */
static void erase_komunikat_line(uint32_t rows) {
    save_cursor_position();
    move_to(rows + 2, 1);
    erase_to_the_end_of_line();
    restore_cursor_position();
}

/**
 * @brief Pobiera napis reprezentujący stan planszy i alokuje pamięć dla napisu
 * reprezentującego pole.
 * Alokuje pamięć dla napisu długości @p field_width i adres
 * zaalokowanej pamięci zapisuje do zmiennej wskazywanej przez @p s.
 * Wywołuje @ref gamma_board i zapisuje wynik w zmiennej
 * wskazywanej przez @p buffer.
 * Jeżeli nie uda się wykonać operacji zwalnia zaalokowaną
 * podczas wykonywania funkcji pamięć.
 * @param[out] s           – wskaźnik na zmienną mającą przechowywać
 *                           napis długosci @p field_width, różny od
 *                           @p NULL,
 * @param[out] buffer      – wskaźnik na zmienną mającą przechowywać napis
 *                           reprezentujący planszę, różny od
 *                           @p NULL,
 * @param[in] field_width  – liczba dodatnia,
 * @param[in] g            – wskaźnik na strukturę reprezentującą stan planszy,
 *                           różny od @p NULL.
 * @return Wartość @p true, jeśli alokacje zakończyły się powodzeniem,
 * Wartość @p false w przeciwnym przypadku.
 */
static bool alloc_strings(char **s, char **buffer, uint32_t field_width, gamma_t *g) {
    *s = malloc((field_width + 1) * sizeof(char));
    (*s)[field_width] = STRING_END;
    if (*s == NULL)
        return false;
    *buffer = gamma_board_interactive(g);
    if (*buffer == NULL) {
        free(*s);
        return false;
    }
    return true;
}

/**
 * @brief Czyści ekran, wyświetla początkowy stan planszy.
 * Ustawia kursor na pole (@p 1, @p 1) i chowa go.
 * @param[in] buffer – napis reprezentujący początkowy stan planszy
 *                     (pusta plansza).
 */
static void tryb_start(char *buffer) {
    clear_screen_move_to_1_1();
    printf("%s", buffer);
    move_to(1, 1);
    hide_cursor();
}

/**
 * @brief Sprawdza, czy na ekranie da się wyświetlic prostokąt o
 * danych wymiarach.
 * Sprawdza, czy na ekranie terminala da się wyświetlić prostokąt
 * o wysokości (liczba wierszy) @p lines i szerokości @p columns.
 * @param[in] lines   – liczba wierszy w napisie reprezentującym prostokąt,
 * @param[in] columns – liczba kolumn w napisie reprezentującym prostokąt.
 * @return Wartość @p true, jeśli się da, tzn (wartość @p lines jest mniejsza
 * od wysokości ekranu i wartość @p columns jest mniejsza od szerokości ekranu),
 * wartość @p false w przeciwnym przypadku.
 */
static bool check_screen_size(uint32_t lines, uint32_t columns) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == IOCTL_ERROR)
        return false;
    return lines < w.ws_row && columns < w.ws_col;
}

/**
 * @brief Sprawdza, czy znak to @p 'g' lub @p 'G'.
 * @param[in] c – kod znaku.
 * @return Wartość @p true, jeśli znak to @p 'g' lub @p 'G',
 * @return Wartość @p true, jeśli znak to @p 'g' lub @p 'G',
 * wartość @p false w przeciwnym przypadku.
 */
static bool is_g(int c) {
    return c == SMALL_G || c == BIG_G;
}

/**
 * @brief Poprawia wartość współrzędnej.
 * Zmienia wartość zmiennej wskazywanej przez @p x_ptr.
 * Gdy jej wartość jest mniejsza od @p 0, ustawia zmienną na wartość @p 0.
 * Gdy jej wartość jest większa lub równa od @p dimension, ustawia
 * jej wartość na @p dimension @p - @p 1.
 * @param x_ptr     – Wskaźnik na zmienną przechowującą wartość współrzędnej,
 * @param dimension – Wartość, od której ta współrzędna powinna być mniejsza.
 */
static void correct_coordinate(int *x_ptr, long long dimension) {
    if (*x_ptr < 0)
        *x_ptr = 0;
    else if (*x_ptr >= dimension)
        *x_ptr = dimension - 1;
}

/**
 * @brief Zmienia kolor tła wyświetlanych napisów (podświetlenie)
 * poleceniem wywoływanym przez wyświetlenie kodu @p background.
 * a kolor wyświetlanych napisów na biały.
 * @param[in] background – kod wywołujący zmianę podświetlania napisów
 *                         w terminalu na określony kolor.
 */
static void field_background(char *background) {
    printf("%s", background);
    printf("\033[37m");
}

/**
 * @brief Wyświetla napis reprezentujący pole na planszy.
 * Zapisuje w buforze @p s napis reprezentujący pole o współrzędnych
 * (@p x, @p y) w planszy, której stan reprezentuje struktura wskazywana
 * przez @p g. Używa do tego funkcji @ref gamma_write_field.
 * Zakłada, że bufor @p s jest zdolny pomieścić napis niekrótszy niż
 * wartość zwrócona przez funkcję @ref gamma_field_width_interactive
 * wywołaną dla paramtru @p g, i że bufor ten jest zakończony znakiem
 * końca łańcucha znaków.
 * Napis zapisany w buforze wyświetla za pomocą polecenia @p printf.
 * Na końcu przywraca kursor do początkowej pozycji.
 * @param[in] g  – wskaźnik na strukturę reprezentującą stan planszy,
 * @param[out] s – bufor, do którego ma być zapisana napisowa reprezentacja
 *                 pola, z założenia rozmiaru niemniejszego niż wartość
 *                 zwracana przez funkcję @ref gamma_field_width_interactive
 *                 wywołaną dla parametru @p g.
 * @param[in] x  – numer kolumny pola na planszy.
 * @param[in] y  – numer wiersza pola na planszy.
 */
static void print_field_no_color(gamma_t *g, char *s, int x, int y) {
    gamma_write_field(g, s, x, y);
    save_cursor_position();
    printf("%s", s);
    restore_cursor_position();
}

/**
 * @brief Wyświetla napis reprezentujący "aktywne" pole na planszy.
 * Funkcja działa podobnie do funkcji @ref print_field_no_color.
 * Funkcja zapisuje w buforze @p s napis reprezentujący
 * pole o współrzędnych (@p x, @p y) w planszy, której stan
 * reprezentuje struktura wskazywana przez @p g. Używa do tego
 * funkcji @ref gamma_write_field.
 * Zakłada, że bufor @p s jest zdolny pomieścić napis niekrótszy niż
 * wartość zwrócona przez funkcję @ref gamma_field_width_interactive
 * wywołaną dla paramtru @p g, i że bufor ten jest zakończony znakiem
 * końca łańcucha znaków.
 * Tak nadpisany bufor funkcja wyświetla na kolorowym tle.
 * Kolor zależy od wartości argumentu @p is_good.
 * Jeżeli jest ona równa @p true, funkcja wyświetla bufor
 * @p s na tle koloru zielonego.
 * W przeciwnym przypadku, napis wyświetlany jest na czerwonym tle.
 * @param[in] g       – wskaźnik na strukturę reprezentującą stan planszy,
 * @param[out] s      – bufor, do którego ma być zapisana napisowa reprezentacja
 *                      pola, z założenia rozmiaru niemniejszego niż wartość
 *                      zwracana przez funkcję @ref gamma_field_width_interactive
 *                      wywołaną dla parametru @p g.
 * @param[in] x       – numer kolumny pola na planszy.
 * @param[in] y       – numer wiersza pola na planszy.
 * @param[in] is_good – zmienna typu @p bool.
 */
static void print_field_color(gamma_t *g, char *s, uint32_t x, uint32_t y, bool is_good) {
    char *col = NULL;
    if (is_good)
        col = GREEN_BACKGROUND;
    else col = RED_BACKGROUND;
    field_background(col);
    gamma_write_field(g, s, x, y);
    save_cursor_position();
    uint32_t w = gamma_field_width_interactive(g);
    int diff = w > 1;
    for (uint32_t i = 0; i < w - diff; i++) {
        putchar(s[i]);
    }
    restore_cursor_position();
    reset_font();
}

/**
 * @brief Wykonuje funkcję @ref gamma_move i wyświetla komunikat,
 * jeśli zwróci ona wartość @p false.
 * Wywołuje funkcję @ref gamma_move
 * dla pola w grze, której stan reprezentuje struktura wskazywana przez
 * @p g, odpowiadającemu polu na ekranie w wierszu
 * @p row i kolumnie @p col (czyli inaczej polu na planszy w grze gamma o
 * współrzędnych (@p col, @p rows @p - @p row @p - @p 1), gdzie @p rows
 * to liczba wierszy w napisie reprezentującym stan gry gamma) i gracza
 * o numerze @p p.
 * Jeżeli wartość zwrócona przez funkcję @ref gamma_move jest równa
 * wartości @p false, funkcja wyświetla komunikat @ref WRONG_MOVE,
 * o niepoprawnej próbie wykonania ruchu w wierszu o numerze @p rows @p + @p 2.
 * @param[in,out] g – wskaźnik na strukturę reprezentującą stan gry,
 * @param[in] rows  – liczba wierszy w napisie reprezentującym stan gry,
 * @param[in] row   – numer wiersza napisu reprezentującego pole na ekranie,
 * @param[in] col   – numer kolumny napisu reprezentującego pole na ekranie,
 * @param[in] p     – liczba odpowiadająca numerowi gracza.
 * @return Wartość zwrócona przez funkcję @ref gamma_move wywołaną
 * dla parametrów (@p g, @p p, @p col, @p rows @p - @p row @p - @p 1)
 */
static bool move_helper(gamma_t *g, uint32_t rows, int row, int col, uint32_t p) {
    bool ret = gamma_move(g, p, col, rows - row - 1);
    if (!ret) {
        printf(RED);
        print_komunikat(WRONG_MOVE, rows);
        reset_font();
    }
    return ret;
}

/**
 * @brief Wykonuje funkcję @ref gamma_golden_move i wyświetla komunikat,
 * jeśli zwróci ona wartość @p false.
 * Wywołuje funkcję @ref gamma_golden_move
 * dla pola w grze, której stan reprezentuje struktura wskazywana przez
 * @p g, odpowiadającemu polu na ekranie w wierszu
 * @p row i kolumnie @p col (czyli inaczej polu na planszy w grze gamma o
 * współrzędnych (@p col, @p rows @p - @p row @p - @p 1), gdzie @p rows
 * to liczba wierszy w napisie reprezentującym stan gry gamma) i gracza
 * o numerze @p p.
 * Jeżeli wartość zwrócona przez funkcję @ref gamma_golden_move jest równa
 * wartości @p false, funkcja wyświetla komunikat @ref WRONG_GOLDEN_MOVE,
 * o niepoprawnej próbie wykonania ruchu w wierszu o numerze @p rows @p + @p 2.
 * @param[in,out] g – wskaźnik na strukturę reprezentującą stan gry,
 * @param[in] rows  – liczba wierszy w napisie reprezentującym stan gry,
 * @param[in] row   – numer wiersza napisu reprezentującego pole na ekranie,
 * @param[in] col   – numer kolumny napisu reprezentującego pole na ekranie,
 * @param[in] p     – liczba odpowiadająca numerowi gracza.
 * @return Wartość zwrócona przez funkcję @ref gamma_golden_move wywołaną
 * dla parametrów (@p g, @p p, @p col, @p rows @p - @p row @p - @p 1)
 */
static bool golden_helper(gamma_t *g, uint32_t rows, int row, int col, uint32_t p) {
    bool ret = gamma_golden_move(g, p, col, rows - row - 1);
    if (!ret) {
        printf(RED);
        print_komunikat(WRONG_GOLDEN_MOVE, rows);
        reset_font();
    }
    return ret;
}

/**
 * @brief Wyświetla komunikat @ref TOO_SMALL_TERMINAL w kolorze czerwonym,
 * zakończony znakiem nowej linii.
 */
static void wrong_terminal_size_message() {
    printf(RED);
    printf(TOO_SMALL_TERMINAL);
    reset_font();
    printf("\n");
}

bool interactive(gamma_t *g, uint32_t *numbers) {
    uint32_t field_width = gamma_field_width_interactive(g);
    uint32_t columns = numbers[0], rows = numbers[1];
    uint32_t num_of_players = numbers[2], player = 1;
    if (!check_screen_size(rows + 2, columns * field_width)) {
        wrong_terminal_size_message();
        return false;
    }
    char *s, *buffer;
    if (!alloc_strings(&s, &buffer, field_width, g))
        return false;
    int row_it = 0, col_it = 0;
    setup_terminal();
    tryb_start(buffer);
    bool tryb_end = false;
    while (!tryb_end) {
        bool is_good = gamma_move_possible(g, player, col_it, rows - row_it - 1);
        erase_and_print_player(g, player, rows);
        print_field_color(g, s, col_it, rows - row_it - 1, is_good);
        int c = kbget();
        erase_komunikat_line(rows);
        if (c == CTRL_D) {
            tryb_end = true;
        } else if (is_direction(c)) {
            print_field_no_color(g, s, col_it, rows - row_it - 1);
            change_coordinates(c, &row_it, &col_it);
        } else if (c == SPACE && move_helper(g, rows, row_it, col_it, player)) {
            player = next_playing_player(g, player, num_of_players);
        } else if (is_g(c) && golden_helper(g, rows, row_it, col_it, player)) {
            player = next_playing_player(g, player, num_of_players);
        } else if (c == BIG_C || c == SMALL_C) {
            player = next_playing_player(g, player, num_of_players);
        }
        correct_coordinate(&row_it, rows);
        correct_coordinate(&col_it, columns);
        if (!tryb_end)
            move_to((row_it + 1), (col_it + 1) * field_width - (field_width - 1));
        if (player == NO_PLAYER)
            tryb_end = true;
    }
    erase_komunikat_line(rows);
    print_field_no_color(g, s, col_it, rows - row_it - 1);
    print_players_summary(g, num_of_players, rows);
    restore_terminal();
    show_cursor();
    free(s);
    free(buffer);
    return true;
}
