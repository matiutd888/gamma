/** @file
 * Zawiera implementację interfejsu gamma.h
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "memory_util.h"
#include "gamma.h"
#include "field.h"
#include "stack.h"
#include "int_to_string_util.h"

#define NO_PLAYER 0  ///< Reprezentuje brak gracza.
#define DIRECTIONS 4 ///< Maksymalna liczba sąsiadów, jaką może posiadać pole.

#define PLAYER_SIZE sizeof(struct m_player) ///< Rozmiar struktury @ref m_player.
#define UINT_64_SIZE sizeof(uint64_t) ///< Rozmiar zmiennej typu @p uint64_t
#define UINT_32_SIZE sizeof(uint32_t) ///< Rozmiar zmiennej typu @p uint32_t

#define NO_PLAYER_CHAR '.' ///< Znak reprezentujący niezajęte pole w napisowej reprezentacji planszy.
#define SPACE ' ' ///< Znak spacji.

static const int x_dir[DIRECTIONS] = {0, 1, 0, -1}; /**< Pomocnicza tablica
                                                     * pomagająca szukać numerów
                                                     * kolumn sąsiadów pola w
                                                     * pętli @p for.
                                                     */

static const int y_dir[DIRECTIONS] = {1, 0, -1, 0}; /**< Pomocnicza tablica
                                                     * pomagająca szukać numerów
                                                     * wierszy sąsiadów pola w
                                                     * pętli @p for.
                                                     */

/** @brief Struktura przechowująca dane gracza.
 * Przechowuje informacje o liczbie obszarów, liczbie zajętych pól
 * i o tym, czy już zagrał złoty ruch.
 */
typedef struct m_player {
    bool has_played_golden_move; ///< Równe @p true, jeśli gracz zagrał złoty ruch.
    uint64_t num_of_areas;       /**< Liczba obszarów, które tworzą pola
                                  * zajmowane przez gracza.
                                  */
    uint64_t busy_fields;        ///< Liczba pól zajętych przez gracza.
} player_t;

/** @struct gamma gamma.h
 * @brief Struktura przechowująca stan gry.
 * Przechowuje informacje o wymiarach planszy, liczbie graczy,
 * maksymalnej liczbie obszarów, liczbie pustych pól, a także trzyma tablicę
 * przechowującą informację o graczach i dwuwymiarowe tablice
 * przechowujące dla każdego pola informacje (kolejno):
 * o tym, jaki gracz je zajmuje, o reprezentancie w
 * strukturze find union obszaru,
 * do jakiego pole należy, o rozmiarze obszaru,
 * do jakiego pole należy, o tym, czy zostało już odwiedzone
 * podczas przeszukiwania planszy.
 * (używane w @ref gamma_golden_move).
 * Struktura przechowuje również wskaźnik na stos przechowujący
 * struktury reprezentujące pola.
 */
struct gamma {
    uint32_t width;            ///< Liczba kolumn planszy.
    uint32_t height;           ///< Liczba wierszy planszy.
    uint32_t num_of_players;   /**< @brief Liczba graczy w grze.
                             * Wartość jest równa argumentowi @p players w
                             * funkcji @ref gamma_new.
                             */
    uint32_t max_num_of_areas; /**< @brief Maksymalna liczba obszarów.
                             * Wartość jest równa argumentowi @p areas w
                             * funkcji @ref gamma_new.
                             */
    uint64_t empty_fields;  ///< Liczba niezajętych pól na planszy.
    player_t *players;      /**< @brief Tablica graczy.
                             * Wskaźnik na pierwszy element tablicy w której
                             * element o indeksie @p i reprezentuje gracza
                             * o @p i @p + @p 1-szym numerze.
                             */
    uint32_t **player_arr;     /**< @brief Pamięta który gracz zajmuje dane pole.
                             * Dynamiczna tablica dwuwymiarowa o wymiarach
                             * [@p width][@p height], w której komórka
                             * [@p x][@p y] jest równa numerowi gracza,
                             * którego pionek zajmuje pole (@p x, @p y), lub
                             * @ref NO_PLAYER, jeśli pole
                             * nie jest zajęte przez żadnego gracza.
                             */
    field_t **parents;      /**< @brief Tablica reprezentantów obszarów.
                             * Dynamiczna tablica dwuwymiarowa o wymiarach
                             * [@p width][@p height], w której komórka
                             * [@p x][@p y] zawiera strukturę, reprezentującą
                             * pole będące reprezentantem
                             * (w strukturze find union) obszaru pionków
                             * zajętych  przez pewnego gracza, do którego
                             * należy pole (@p x, @p y). Jeśli pole nie jest
                             * zajęte przez żadnego gracza, a co za tym idzie,
                             * nie należy do żadnego obszaru, wartość
                             * komórki odpowiadającej polu nie ma sensownej
                             * interpretacji i jest równa strukturze
                             * reprezentującej pole o współrzędnych (@p 0, @p 0).
                             */
    uint64_t **areas_size;  /**< @brief Tablica trzymająca informacje
                             * o rozmiarach obszarów.
                             * Dynamiczna tablica dwuwymiarowa o wymiarach
                             * [@p width][@p height]. Jeśli pole (@p x, @p y)
                             * jest reprezentantem pewnego obszaru, wartość
                             * @p areas_size[x][y] jest równa ilości pól w
                             * w obszarze, które dane pole reprezentuje.
                             * Jeśli pole (@p x, @p y) nie jest
                             * reprezentantem żadnego obszaru, wartość
                             * @p areas_size[x][y] jest równa 0.
                             */
    int **visited;          /**< @brief Tablica pomocnicza używana przy
						     * przeszukiwaniu planszy. Używana w @ref delete_util.
                             */
    stack_t *stack;         /**< @brief Wskaźnik na strukturę reprezentującą stos
                             * struktur reprezentujących pola na planszy.
                             * Używany przy przeszukiwaniu planszy algorytmem DFS.
                             */
};

/** @brief Sprawdza, czy współrzędne pola i
 * wskaźnik na strukturę przechowującą stan gry są poprawne.
 * Sprawdza, wskaźnik @p g nie jest równy @p NULL i czy współrzędne
 * @p x i @p y są zgodne z założeniami dla planszy @p g, to znaczy @p x
 * jest mniejszy od wartości @p width z funkcji @ref gamma_new
 * i @p y jest mniejszy od wartości @p height z funkcji @ref gamma_new.
 * @param[in] x   – numer kolumny, liczba nieujemna,
 * @param[in] y   – numer wiersza, liczba nieujemna,
 * @param[in] g   – wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość @p true, jeśli wskaźnik na strukturę przechowującą stan gry
 * @p g jest równy @p NULL lub @p x jest niemniejszy od
 * wartości @p width z funkcji @ref gamma_new lub @p y jest niemniejszy
 * od wartości @p height z funkcji @ref gamma_new,
 * a @p false w przeciwnym przypadku.
 */
static bool wrong_field(uint32_t x, uint32_t y, gamma_t *g) {
    return g == NULL || !(x < g->width && y < g->height);
}

/** @brief Sprawdza, czy numer gracza i wskaźnik
 * na strukturę przechowującą stan gry są poprawne.
 * Sprawdza czy wskaźnik @p g nie jest równy NULL oraz
 * czy numer gracza @p player
 * jest liczbą dodatnią i niewiększą od wartości
 * @p players z funkcji @ref gamma_new.
 * @param[in] player – numer gracza,
 * @param[in] g      – wskaźnik na strukturę przechowującą stan gry.
 * @return Wartość @p true, jeśli wskaźnik @p g jest równy @p NULL,
 * lub @p player jest większy niż wartość @p g->num_of_player lub niedodatni.
 */
static bool wrong_player(uint32_t player, gamma_t *g) {
    return g == NULL || g->num_of_players < player || player < 1;
}

/** @brief Sprawdza, czy pole nie należy do żadnego gracza.
* Sprawdza wartość @p g->player_arr dla pola o współrzędnych
* (@p x, @p y).
* @param[in] x 	– numer kolumny, liczba nieujemna, mniejsza od
* 			      @p g->width,
* @param[in] y 	– numer wiersza, liczba nieujemna, mniejsza od
* 				  @p g->height,
* @param[in] g  – wskaźnik na strukturę przechowującą stan gry,
* 				  różny od @p NULL.
* @return Wartość @p true, jeśli wartość @p g->player_arr dla pola o współrzędnych
* (@p x, @p y) jest równa @p NO_PLAYER, @p false w przeciwnym przypadku.
*/
static bool no_field(uint32_t x, uint32_t y, gamma_t *g) {
    return g->player_arr[x][y] == NO_PLAYER;
}

/** @brief Tworzy stos.
 * Rezerwuje pamięć dla stosu o maksymalnym rozmiarze
 * równym ilości pól na planszy w grze, której stan reprezentowany
 * jest przez strukturę wskazywaną
 * przez @p g. Ustawia wskaźnik @p g->stack na ten stos.
 * @param[in] g – wskaźnik na strukturę reprezentujacą stan gry,
 *                różny od @p NULL.
 * @return Wartość @p true, jeśli udało się zaalokować pamięć,
 * @p false w przeciwnym przypadku.
 */
static bool init_m_stack(gamma_t *g) {
    uint64_t stack_max_size = g->width;
    stack_max_size *= g->height;
    g->stack = init_stack(stack_max_size);
    if (g->stack == NULL)
        return false;
    return true;
}

/** @brief Ustawia wszystkie wskaźniki na @p NULL.
 * W strukturze wskazywanej przez @p g ustawia wszystkie
 * atrybuty będące wskaźnikami na @p NULL.
 * @param[in] g – wskaźnik na strukturę reprezentujacą stan gry,
 *                różny od @p NULL.
 */
static void set_pointers_to_NULL(gamma_t *g) {
    g->players = NULL;
    g->parents = NULL;
    g->player_arr = NULL;
    g->visited = NULL;
    g->stack = NULL;
    g->areas_size = NULL;
}

gamma_t *gamma_new(uint32_t width, uint32_t height, uint32_t players, uint32_t areas) {
    if (width < 1 || height < 1 || players < 1 || areas < 1)
        return NULL;

    gamma_t *g = NULL;
    g = malloc(sizeof(struct gamma));
    if (g == NULL)
        return NULL;
    g->max_num_of_areas = areas;
    g->width = width;
    g->height = height;
    g->num_of_players = players;
    g->empty_fields = width;
    g->empty_fields *= height;

    set_pointers_to_NULL(g);
    bool is_ok = true;
    if (!init_arr_2D((void ***) &g->parents, FIELD_SIZE, width, height))
        is_ok = false;
    if (!init_array_1D((void **) &g->players, PLAYER_SIZE, players))
        is_ok = false;
    if (!init_arr_2D((void ***) &g->player_arr, UINT_32_SIZE, width, height))
        is_ok = false;
    if (!init_arr_2D((void ***) &g->areas_size, UINT_64_SIZE, width, height))
        is_ok = false;
    if (!init_arr_2D((void ***) &g->visited, sizeof(int), width, height))
        is_ok = false;
    if (!init_m_stack(g))
        is_ok = false;
    if (!is_ok) {
        gamma_delete(g);
        return NULL;
    }
    return g;
}

/** @brief Liczy ilość sąsiednich pól zajętych przez gracza @p player.
 * Sprawdza liczbę pól sąsiadujących z polem o współrzędnych (@p x, @p y)
 * dla których wartość odpowiadającej im komórki w @p g->player_arr jest równa
 * @p player, co jest równe ilości pól należących do gracza @p player
 * sąsiadujących z polem (@p x, @p y) na planszy w grze,
 * której stan reprezentuje struktura wskazywana przez @p g.
 * @param[in] g      – wskaźnik na strukturę przechowującą stan gry,
 * 					   różny od @p NULL,
 * @param[in] x      – numer kolumny, liczba nieujemna, mniejsza od
 * 					   @p g->width,
 * @param[in] y      – numer wiersza, liczba nieujemna, mniejsza od
 * 					   @p g->height,
 * @param[in] player – numer gracza, liczba niewiększa od @p g->num_of_players.
 * @return Liczba takich pól.
 */
static int count_neighbours(gamma_t *g, uint32_t x, uint32_t y, uint32_t player) {
    int l = 0;
    for (int i = 0; i < DIRECTIONS; ++i) {
        uint32_t x_i = x + x_dir[i];
        uint32_t y_i = y + y_dir[i];
        if (!wrong_field(x_i, y_i, g) && g->player_arr[x_i][y_i] == player)
            l++;
    }
    return l;
}

/** @brief Zwraca reprezentanta obszaru do którego należy pole.
 * Wykorzystuje algorytm operacji @p find ze struktury @p Find @p Union.
 * Rekurencyjnie szuka komórki w tablicy @p g->parents reprezentującej
 * pole, które jest swoim własnym reprezentantem.
 * @param[in] g – wskaźnik na strukturę reprezentującą stan gry,
 * 				  różny od @p NULL,
 * @param[in] f – struktura reprezentująca pole; założenie jest takie,
 * 				  że pole to jest zajęte przez jakiegoś gracza,
 * 				  i (co z tego wynika) należy do pewnego obszaru.
 * @return Struktura reprezentująca pole będące reprezentantem obszaru,
 * do którego należy pole @p f.
 */
static field_t find_parent(gamma_t *g, field_t f) {
    if (!field_equals(f, (g->parents[f.x][f.y])))
        g->parents[f.x][f.y] = find_parent(g, g->parents[f.x][f.y]);
    return g->parents[f.x][f.y];
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
    if (wrong_player(player, g))
        return 0;
    return g->players[player - 1].busy_fields;
}

/** @brief Porównuje reprezentantów obszarów ze względu na wielkość obszaru,
 * który reprezentują.
 * Porównuje wartości @p g->areas_size dla komórek reprezentujących pola
 * @p f1 i @p f1.
 * @param[in] g  – wskaźnik na strukturę reprezentującą stan gry, różny
 * 				   od @p NULL,
 * @param[in] f1 – struktura reprezentująca pole będące reprezentantem
 * 				   pewnego obszaru,
 * @param[in] f2 – struktura reprezentująca pole będące reprezentantem
 *                 pewnego obszaru.
 * @return Wartość @p true, jeśli rozmiar (ilość pól) obszaru
 * reprezentowanego przez @p f1 jest mniejsza od rozmiaru obszaru
 * reprezentowanego przez @p f2.
 */
static bool parents_compare(gamma_t *g, field_t f1, field_t f2) {
    return g->areas_size[f1.x][f1.y] < g->areas_size[f2.x][f2.y];
}

/** @brief Sortuje tablicę reprezentantów obszarów malejąco po rozmiarze
 * obszarów, które reprezentują.
 * Sortuje tablicę struktur długości @p l, której zerowy element wskazywany jest
 * przez @p n_parents, reprezentujących pola będace reprezentantami
 * pewnych obszarów na planszy w grze reprezentowanej przez strukturę
 * wskazywanej przez @p g, używając funkcji @ref parents_compare.
 * @param[in] g               – wskaźnik na strukturę reprezentującą stan gry,
 * 							    różny od @p NULL,
 * @param[in,out] n_parents   – wskaźnik na zerowy element tablicy struktur
 * 					            reprezentujących pola będące reprezentantami
 * 						        pewnych obszarów na planszy w grze
 * 								reprezentowanej przez strukturę
 * 								wskazywaną przez @p g,
 * @param[in] l               – długość tablicy, której zerowy element wskazywany
 *                              jest przez @p n_parents.
 */
static void sort_n_parents(gamma_t *g, field_t *n_parents, int l) {
    for (int i = 0; i < l; ++i) {
        for (int j = 0; j < l - i - 1; ++j) {
            if (parents_compare(g, n_parents[j], n_parents[j + 1])) {
                field_t tmp = n_parents[j + 1];
                n_parents[j + 1] = n_parents[j];
                n_parents[j] = tmp;

            }
        }
    }
}

/** @brief Dokonuje scalenia dwóch obszarów.
 * Dołącza na planszy w grze reprezentowanej przez strukturę wskazywaną
 * przez @p g,  obszar reprezentowany przez pole reprezentowane przez
 * strukturę @p smaller do obszaru reprezentowanego przez pole reprezentowane
 * przez strukturę @p bigger. Dokonuje tego poprzez ustawienie reprezentanta
 * obszaru dotychczas reprezentowanego przez @p smaller na @p bigger i
 * zaktualizowanie komórki w tablicy @p g->areas_size reprezentującej pole
 * reprezentowane przez @p bigger.
 * @param[in,out] g        – wskaźnik na strukturę reprezentującą stan gry,
 *                           różny od @p NULL,
 * @param[in] bigger       – struktura reprezentująca pole będące reprezentantem
 * 						     pewnego obszaru na planszy w grze reprezentowanej
 *						     przez strukturę wskazywaną przez @p g,
 * @param[in] smaller      – struktura reprezentująca pole będące reprezentantem
 * 					         obszaru, który należy dołączyć do obszaru
 *                           reprezentowanego przez pole reprezentowane
 *                           przez @p bigger.
 */
static void union_areas(gamma_t *g, field_t bigger, field_t smaller) {
    uint64_t s2 = g->areas_size[smaller.x][smaller.y];
    g->parents[smaller.x][smaller.y] = bigger;
    g->areas_size[bigger.x][bigger.y] += s2;
    g->areas_size[smaller.x][smaller.y] = 0;
}

/** @brief Łączy wszystkie obszary, do których należą pola
 * w tablicy @p neighbours.
 * Funkcja dokonuje scalenia wszystkich obszarów na planszy w grze której stan
 * reprezentowany jest przez strukturę wskazywaną przez @p g, do których
 * należą pola reprezentowane przez tablicę @p neighbours (długosci @p l),
 * dołączając obszary do tego o największym rozmiarze. Pola w tablicy
 * @p neighbours z założenia należą do gracza @p player.
 * Scalając funkcja aktualizuje atrybut @p num_of_areas gracza @p player.
 * @param[in,out] g       – wskaźnik na strukturę reprezentującą stan gry,
 * 						    różny od @p NULL,
 * @param[in] neighbours  – tablica struktur reprezentujących pola należące
 * 						    do obszarów, które należy ze sobą scalić.
 * 						    Pola te powinny być zajęte przez gracza @p player.
 * @param[in] l           – długość tablicy @p neighbours, liczba mniejsza od
 * 						    @ref DIRECTIONS, większa od 0.
 * @param[in] player      – numer gracza, do którego należą pola w tablicy
 * 						    @p neighbours, liczba dodatnia niewiększa od
 * 						    @p g->num_of_players.
 */
static void remove_same_areas(gamma_t *g, field_t neighbours[], uint32_t l, uint32_t player) {
    field_t n_parents[DIRECTIONS];
    for (uint32_t i = 0; i < l; ++i)
        n_parents[i] = find_parent(g, neighbours[i]);

    sort_n_parents(g, n_parents, l);

    uint32_t it = 1;
    field_t accumulator = n_parents[0];

    while (it < l) {
        n_parents[it] = find_parent(g, n_parents[it]);
        if (!field_equals(n_parents[it], accumulator)) {
            union_areas(g, accumulator, n_parents[it]);
            g->players[player - 1].num_of_areas--;
        }
        it++;
    }
}

/** @brief Wykonuje ruch zajmujący pole (@p x, @p y).
 * Zajmuje graczem @p player pole o o współrzędnych (@p x, @p y) na
 * planszy w grze, której stan reprezentowany jest przez strukturę
 * wskazywaną przez @p g. Dokonuje zmiany odpowiednich atrybutów struktury
 * wskazywanej przez @p g: Zmniejsza o jeden liczbę niezajętych pól na planszy,
 * zwiększa o jeden liczbę pól zajętych przez gracza @p player i
 * liczbę obszarów, które tworzą pola gracza @p player na planszy.
 * @param[in,out] g        – wskaźnik na strukturę reprezentującą stan gry,
 *                           różny od @p NULL,
 * @param[in] x            – numer kolumny, liczbe nieujemna mniejsza od
 * 							 @p g->width,
 * @param[in] y            – numer wiersza, liczba nieujemna mniejsza od
 * 							 @p g->height,
 * @param[in] player       – numer gracza, liczba dodatnia niewiększa od
 * 							 @p g->num_of_players
 */
static void take_field(gamma_t *g, uint32_t x, uint32_t y, uint32_t player) {
    g->player_arr[x][y] = player;
    g->players[player - 1].busy_fields++;
    g->players[player - 1].num_of_areas++;
    g->empty_fields--;
}

/** @brief Zajmuje pole i aktualizuje liczbę obszarów.
 * Wykonuje ruch graczem @p player zajmujący pole (@p x, @p y)
 * na planszy w grze, której stan reprezentowany jest przez
 * strukturę wskazywaną przez @p g. Po wykonaniu ruchu aktualizuje
 * liczbę obszarów zajmowanych przez gracza @p player.
 * @param[in,out] g        – wskaźnik na strukturę reprezentującą stan gry,
 *                           różny od @p NULL,
 * @param[in] x            – numer kolumny, liczbe nieujemna mniejsza od
 * 							 @p g->width,
 * @param[in] y            – numer wiersza, liczba nieujemna mniejsza od
 * 							 @p g->height,
 * @param[in] player       – numer gracza, liczba dodatnia niewiększa od
 * 							 @p g->num_of_players
 */
static void move_util(gamma_t *g, uint32_t x, uint32_t y, uint32_t player) {
    take_field(g, x, y, player);

    field_t neigbours[DIRECTIONS];
    int l = 0;
    for (int i = 0; i < DIRECTIONS; ++i) {

        field_t f_i;
        f_i.x = x + x_dir[i];
        f_i.y = y + y_dir[i];
        if (!wrong_field(f_i.x, f_i.y, g)
            && g->player_arr[f_i.x][f_i.y] == player) {

            neigbours[l] = f_i;
            if (l == 0) {
                g->players[player - 1].num_of_areas--;
                field_t parent = find_parent(g, f_i);
                g->parents[x][y] = parent;
                g->areas_size[parent.x][parent.y]++;
            }
            l++;
        }
    }
    if (l == 0) {
        g->parents[x][y].x = x;
        g->parents[x][y].y = y;
        g->areas_size[x][y] = 1;
        return;
    }
    remove_same_areas(g, neigbours, l, player);
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (wrong_player(player, g) || wrong_field(x, y, g))
        return false;

    uint32_t p = g->player_arr[x][y];
    if (p != NO_PLAYER)
        return false;

    if (g->players[player - 1].num_of_areas >= g->max_num_of_areas
        && count_neighbours(g, x, y, player) == 0) {
        return false;
    }

    move_util(g, x, y, player);

    return true;
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
    if (wrong_player(player, g))
        return 0;
    if (g->players[player - 1].num_of_areas > g->max_num_of_areas)
        return 0;

    uint64_t count = 0;
    if (g->players[player - 1].num_of_areas == g->max_num_of_areas) {
        for (uint32_t i = 0; i < g->width; ++i) {
            for (uint32_t j = 0; j < g->height; ++j) {
                if (no_field(i, j, g) && count_neighbours(g, i, j, player) > 0)
                    count++;
            }
        }
        return count;
    }
    return g->empty_fields;
}

/**
 * @brief Wykonuje złoty ruch bez sprawdzania ani zmieniania tego,
 * czy gracz wykonał już złoty ruch.
 * Tymczasowo zmienia wartość pola @p has_played_golden_move w strukturze
 * reprezentującej gracza o numerze @p player (przechowywanej w strukturze
 * reprezentującej stan gry, wskazywanej przez @p g) na wartość @p false.
 * Następnie wykonuje @ref gamma_golden_move na parametrach podanych
 * przy wywołaniu funkcji i zapamiętuje wartość zwróconą przez funkcję.
 * Przywraca początkową wartość pola @p has_played_golden_move w strukturze
 * reprezentującej gracza o numerze @p player.
 * @param[in,out] g        – wskaźnik na strukturę reprezentującą stan gry,
 *                           różny od @p NULL,
 * @param[in] player       – numer gracza, liczba dodatnia niewiększa od
 * 							 @p g->num_of_players,
 * @param[in] x            – numer kolumny, liczbe nieujemna mniejsza od
 * 							 @p g->width,
 * @param[in] y            – numer wiersza, liczba nieujemna mniejsza od
 * 							 @p g->height.
 * @return Wartość @p false w przypadku niepoprawnych parametrów. Wartość
 * zwrócona przy wywołaniu @ref gamma_golden_move na odpowiednio zmienionej
 * strukturze wskazywanej przez @p g w przeciwnym przypadku.
 */
static bool golden_no_checking_nor_changing(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (wrong_player(player, g) || wrong_field(x, y, g))
        return false;

    bool has_played_save = g->players[player - 1].has_played_golden_move;
    g->players[player - 1].has_played_golden_move = false;
    bool golden = gamma_golden_move(g, player, x, y);
    g->players[player - 1].has_played_golden_move = has_played_save;
    return golden;
}

bool gamma_golden_possible(gamma_t *g, uint32_t player) {
    if (wrong_player(player, g))
        return false;
    if (g->players[player - 1].has_played_golden_move)
        return false;

    bool player_heurisic = false;
    for (uint32_t i = 0; i < g->num_of_players; i++) {
        if (i != player - 1 && g->players[i].busy_fields > 0)
            player_heurisic = true;
    }
    if (!player_heurisic)
        return false;
    if (g->players[player - 1].num_of_areas < g->max_num_of_areas)
        return true;

    for (uint32_t i = 0; i < g->width; i++) {
        for (uint32_t j = 0; j < g->height; j++) {
            uint32_t player_it = g->player_arr[i][j];
            if (golden_no_checking_nor_changing(g, player, i, j)) {
                golden_no_checking_nor_changing(g, player_it, i, j);
                return true;
            }
        }
    }
    return false;
}

/** @brief Znajduje szerekość kolumn w reprezentacji planszy
 * w postaci napisu.
 * Znajduje maksymalną liczbę cyfr w zapisie dziesiętnym
 * wśród numerów graczy, którzy zajmują przynajmniej jedno pole,
 * na planszy w grze, której stan reprezentowany jest przez
 * strukturę wskazywaną przez @p g.
 * Jeśli ta liczba cyfr jest równa @p 1, funkcja zwraca @p 1,
 * w przeciwnym wypadku zwraca maksymalną liczbę cyfr
 * plus jeden, (ponieważ w przypadku numerów o liczbie cyfr
 * większej od 1 należy oddzielać je spacją). Liczba zwracana
 * przez funkcję jest równa ilości znaków przeznaczonych na
 * reprezentację pola na planszy w napisie
 * reprezentującym stan planszy, zwracanym przez @ref gamma_board.
 * @param g – wskaźnik na strukturę reprezentującą stan gry,
 *            różny od @p NULL.
 * @return Maksymalna liczba cyfr w zapisie dziesiętnym
 * wśród numerów graczy, które zajmują przynajmniej jedno pole,
 * na planszy w grze, której stan reprezentowany jest przez strukturę
 * wskazywaną przez @p g, jeśli numery te są jednocyfrowe.
 * W przeciwnym wypadku maksymalna liczba cyfr w zapisie dziesiętnym
 * wśród numerów graczy, które zajmują przynajmniej jedno pole,
 * jeśli numery te są jednocyfrowe plus @p 1.
 */
static uint32_t find_max_number_width(gamma_t *g) {
    uint32_t max_width = 1;
    for (uint32_t i = 0; i < g->num_of_players; ++i) {
        if (g->players[i].busy_fields > 0) {
            uint32_t d = digit_count(i + 1);
            if (d > max_width)
                max_width = d;
        }
    }
    if (max_width > 1)
        max_width++;
    return max_width;
}

uint32_t gamma_field_width_interactive(gamma_t *g) {
    uint32_t width = digit_count(g->num_of_players);
    if (width > 1)
        width++;
    return width;
}

/**
 * @brief Zapisuje napisową reprezentację planszy do bufora.
 * @param[in] g          – wskaźnik na strukturę reprezentującą stan gry,
 *                         różny od @p NULL,
 * @param[in] max_width  – szerokosć pola w napisowej reprezentacji planszy,
 * @param[out] buffer    – bufor rozmiaru zdolnego pomieścić napisową
 *                         reprezentację planszy (niemniejszego niż
 *                         @p g->num_of_fields * @p max_width +
 *                         @p g->height + @p 1.
 */
static void gamma_board_helper(gamma_t *g, uint32_t max_width, char *buffer) {
    uint64_t it = 0;
    for (uint32_t i = 0; i < g->height; ++i) {
        for (uint32_t j = 0; j < g->width; ++j) {
            // TODO czy użyć tutaj gamma_write_field
            // trzeba by chyba zmodyfikować write_field tak, by przyjmowało szerokość pola
            uint32_t old_it = it;
            if (g->player_arr[j][g->height - i - 1] == NO_PLAYER) {
                buffer[it] = NO_PLAYER_CHAR;
                it++;
            } else {
                it = write_int(buffer, it, g->player_arr[j][g->height - i - 1]);
            }
            while (it - old_it < max_width) {
                buffer[it] = SPACE;
                it++;
            }
        }
        buffer[it] = '\n';
        it++;
    }
    buffer[it] = '\0';
}

/**
 * @brief Alokuje pamięć dla napisowej reprezentacji gry.
 * Alokuje pamięć dla bufora przechowującego napisową
 * reprezentację planszy w grze, której stan reprezentowany jest przez
 * strukturę wskazywaną przez @p g, w której to napisowej reprezentacji
 * szerokość pola jest równa @p max_width.
 * @param[in] g         – wskaźnik na strukturę reprezentującą stan gry,
 *                        różny od @p NULL,
 * @param max_width     – szerokość pola w napisie reprezentującym stan planszy.
 * @return Wskaźnik na zaalokowany bufor lub wartość @p NULL, jeśli nie
 * udało się go zaalokować.
 */
static char *alloc_memory_for_board(gamma_t *g, uint32_t max_width) {
    char *buffer = NULL;
    uint64_t num_of_fields = g->width;
    num_of_fields *= g->height;
    buffer = malloc((num_of_fields * max_width + g->height + 1) * sizeof(char));
    return buffer;
}

char *gamma_board(gamma_t *g) {
    if (g == NULL)
        return NULL;
    uint32_t max_width = find_max_number_width(g);
    char *buffer = alloc_memory_for_board(g, max_width);
    if (buffer == NULL)
        return NULL;
    gamma_board_helper(g, max_width, buffer);
    return buffer;
}

void gamma_delete(gamma_t *g) {
    if (g == NULL)
        return;

    free_array((void **) g->player_arr, g->width);
    free_array((void **) g->parents, g->width);
    free_array((void **) g->areas_size, g->width);
    free_array((void **) g->visited, g->width);
    if (g->players != NULL)
        free(g->players);
    stack_destruct(g->stack);
    free(g);
}

/** @brief Usuwa pionek z pola.
 * Usuwa pionek gracza @p player z pola (@p x, @p y) na planszy
 * w grze, której stan reprezentuje struktura wskazywana przez @p g.
 * Zwiększa atrybut odpowiadający liczbie niezajętych pól na planszy,
 * ustawia rozmiar obszaru, do którego należało dotychczas
 * pole (@p x, @p y) na @p 0, zmniejsza od jeden liczbę obszarów i
 * liczbę zajętych pól gracza @p player.
 * @param[in,out] g  – wskaźnik na strukture reprezentującą stan gry, różny od
 *                     @p NULL,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od @p
 * 					   g->num_of_players
 * @param[in] x      – numer kolumny, liczbe nieujemna mniejsza od
 * 					   @p g->width,
 * @param[in] y      – numer wiersza, liczba nieujemna mniejsza od
 * 					   @p g->height.
 */
static void delete_field(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    field_t f;
    f.x = x;
    f.y = y;
    field_t parent = find_parent(g, f);
    g->areas_size[parent.x][parent.y] = 0;
    g->player_arr[x][y] = NO_PLAYER;
    g->empty_fields++;
    g->players[player - 1].busy_fields--;
    g->players[player - 1].num_of_areas--;
}

/** @brief Sprawdza, czy pole należy do gracza i nie zostało odwiedzone.
 * Sprawdza czy na planszy, w grze, której stan reprezentuje struktura
 * wskazywana przez @p g, istnieje pole o współrzędnych (@p x, @p y)
 * należące do gracza @p p, i wartość komórki w tablicy @p g->visited
 * odpowiadającej polu (@p x, @p y) jest równa @p 0.
 * @param[in,out] g  – wskaźnik na strukture reprezentującą stan gry, różny od
 *                     @p NULL,
 * @param[in] x      – numer kolumny,
 * @param[in] y      – numer wiersza,
 * @param[in] p      – numer gracza, liczba dodatnia niewiększa od
 *                     @p g->num_of_players.
 * @return Wartość @p true, jeśli na planszy, w grze, której stan
 * reprezentuje struktura wskazywana przez @p g, istnieje pole o
 * współrzędnych (@p x, @p y) należące do gracza @p p
 * (tzn takie, że g->player_arr[x][y] == player) i wartość komórki
 * w tablicy @p g->visited odpowiadającej polu (@p x, @p y) jest
 * równa @p 0. Wartość @p false w przeciwnym przypadku.
 */
static bool dfs_condition(gamma_t *g, uint32_t x, uint32_t y, uint32_t p) {
    return !wrong_field(x, y, g)
           && !g->visited[x][y]
           && g->player_arr[x][y] == p;
}

/** @brief Ustawia pola w tablicy @p visited na @p 0.
 * Dla planszy, w grze, której stan reprezentuje struktura wskazwana
 * przez @p g, @p Odwiedzonym @p Obszarem nazwiemy zbiór pól
 * należących do tego samego obszaru na planszy, których wartość
 * w odpowiadającej im komórce tablicy @p g->visited
 * jest równa 1. Funkcja @p init_visited dla wszystkich
 * pól w @p Odwiedzonym @p Obszarze, do którego należy pole
 * (@p x, @p y) ustawia wartość odpowiadającej im komórki w
 * tablicy @p g->visited na @p 0 (używając do tego algorytmu
 * DFS).
 * @param[in,out] g  – wskaźnik na strukture reprezentującą stan gry,
 *                     różny od @p NULL,
 * @param[in] x      – numer kolumny, liczbe nieujemna mniejsza od
 * 					   @p g->width,
 * @param[in] y      – numer wiersza, liczba nieujemna mniejsza od
 * 					   @p g->height.
 */
static void init_visited(gamma_t *g, uint32_t x, uint32_t y) {
    stack_push(g->stack, x, y);
    g->visited[x][y] = 0;
    while (!is_stack_empty(g->stack)) {
        field_t f = stack_pop(g->stack);
        for (int i = 0; i < DIRECTIONS; ++i) {
            uint32_t x_i = f.x + x_dir[i];
            uint32_t y_i = f.y + y_dir[i];
            if (!wrong_field(x_i, y_i, g) && g->visited[x_i][y_i]) {
                g->visited[x_i][y_i] = 0;
                stack_push(g->stack, x_i, y_i);
            }
        }
    }
}

/** @brief Ustawia reprezentanta obszaru.
 * Funkcja ustawia pole @p parent jako reprezentanta obszaru
 * do którego należy pole (@p x, @p y), składającego się z pól zajętych
 * przez gracza @p p, w grze, której stan reprezentuje struktura wskazywana
 * przez @p g. Zwraca ilość pól należących do tego obszaru.
 * @param[in,out] g – wskaźnik na strukture reprezentującą stan gry,
 *                     różny od @p NULL,
 * @param[in] parent – struktura reprezentująca pole, które zostanie
 *                     ustawione jako reprezentant obszaru,
 * @param[in] x      – numer kolumny, liczbe nieujemna mniejsza od
 * 				       @p g->width,
 * @param[in] y      – numer wiersza, liczba nieujemna mniejsza od
 * 				       @p g->height,
 * @param[in] p      – numer gracza, liczba dodatnia niewiększa od
 *                     @p g->num_of_players.
 * @return Wielkość obszaru pól zajętych przez gracza @p p, do którego
 * należu pole (@p x, @p y), w grze, której stan reprezentuje struktura
 * wskazywana przez @p g.
 */
static uint64_t set_parent_dfs(gamma_t *g, field_t parent, uint32_t x, uint32_t y, uint32_t p) {
    uint64_t count = 0;
    stack_push(g->stack, x, y);
    g->visited[x][y] = 1;
    while (!is_stack_empty(g->stack)) {
        count++;
        field_t f = stack_pop(g->stack);
        g->parents[f.x][f.y] = parent;
        for (int i = 0; i < DIRECTIONS; ++i) {
            uint32_t x_i = f.x + x_dir[i];
            uint32_t y_i = f.y + y_dir[i];
            if (dfs_condition(g, x_i, y_i, p)) {
                g->visited[x_i][y_i] = 1;
                stack_push(g->stack, x_i, y_i);
            }
        }
    }
    return count;
}

/** @brief Usuwa pole i aktualizuje informacje o obszarach.
 * Usuwa z planszy w grze reprezentowanej przez strukturę wskazywaną
 * przez @p g pionek gracza @p player, stojący dotychczas na polu
 * (@p x, @p y). Po usunięciu aktualizuje informacje o obszarach, które
 * tworzą pionki gracza @p player na planszy.
 * @param[in,out] g  – wskaźnik na strukture reprezentującą stan gry, różny od
 *                     @p NULL,
 * @param[in] x      – numer kolumny, liczbe nieujemna mniejsza od
 * 					   @p g->width,
 * @param[in] y      – numer wiersza, liczba nieujemna mniejsza od
 * 					   @p g->height,
 * @param[in] player – numer gracza, liczba dodatnia niewiększa od
 *                     @p g->num_of_players. Funkcja zakłada, że pionek
 *                     gracza @p player leży na polu (@p x, @p y).
 */
static void delete_util(gamma_t *g, uint32_t x, uint32_t y, uint32_t player) {
    delete_field(g, player, x, y);

    for (int i = 0; i < DIRECTIONS; ++i) {
        field_t f_i;
        f_i.x = x + x_dir[i];
        f_i.y = y + y_dir[i];
        if (dfs_condition(g, f_i.x, f_i.y, player)) {
            uint64_t a_size = set_parent_dfs(g, f_i, f_i.x, f_i.y, player);
            g->players[player - 1].num_of_areas++;
            g->areas_size[f_i.x][f_i.y] = a_size;
        }
    }
    init_visited(g, x, y);
}

/** @brief Próbuje usunąć pionek i wstawić nowy na jego miejsce.
 * Sprawdza, czy z planszy w grze, której stan reprezentuje struktura
 * wskazywana przez @p g, można usunąć pionek stojący na polu (@p x, @p y),
 * i dodać na to pole pionek gracza @p n_player, nie naruszając
 * przy tym zasady o maksymalnej liczbie obszarów, które może zająć jeden gracz.
 * Jeśli można, wykonuje taką operację (aktualizując przy tym wszystkie
 * informacje o obszarach i ilościach wolnych/zajętych pól)
 * i zwraca wartość @p true. Jeśli z jakiegoś powodu nie można tego zrobić
 * (na polu (@p x, @p y) nie stoi żaden pionek, lub zasada o liczbie obszarów
 * byłaby naruszona), funkcja zwraca wartość @p false.
 * @param[in,out] g    – wskaźnik na strukture reprezentującą stan gry, różny od
 *                       @p NULL,
 * @param[in] x        – numer kolumny, liczbe nieujemna mniejsza od
 * 					     @p g->width,
 * @param[in] y        – numer wiersza, liczba nieujemna mniejsza od
 * 					     @p g->height,
 * @param[in] n_player – numer gracza, liczba dodatnia niewiększa od
 *                       @p g->num_of_players. Funkcja zakłada, że na
 *                       polu (@p x, @p y) nie leży pionek gracza
 *                       @p n_player.
 * @return Wartość @p true, jeśli nie naruszając zasad gry da się
 * usunąć z planszy pionek leżący na polu (@p x, @p y) należący do pewnego
 * gracza i ustawić na nim pionka gracza @p n_player (na planszy w grze,
 * której stan reprezentuje struktura wskazywana przez @p g), wartość
 * @p false w przeciwnym przypadku.
 */
static bool delete_and_move(gamma_t *g, uint32_t x, uint32_t y, uint32_t n_player) {
    if (g->players[n_player - 1].num_of_areas >= g->max_num_of_areas
        && count_neighbours(g, x, y, n_player) == 0) {
        return false;
    }

    uint32_t player = g->player_arr[x][y];
    delete_util(g, x, y, player);


    if (g->players[player - 1].num_of_areas > g->max_num_of_areas) {
        gamma_move(g, player, x, y);
        return false;
    }

    if (!gamma_move(g, n_player, x, y)) {
        gamma_move(g, player, x, y);
        return false;
    }
    return true;
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (wrong_player(player, g) || wrong_field(x, y, g))
        return false;

    if (g->players[player - 1].has_played_golden_move)
        return false;

    if (g->player_arr[x][y] == player)
        return false;

    if (no_field(x, y, g))
        return false;

    if (delete_and_move(g, x, y, player)) {
        g->players[player - 1].has_played_golden_move = true;
        return true;
    }

    return false;
}

char *gamma_board_interactive(gamma_t *g) {
    if (g == NULL)
        return NULL;
    uint32_t max_width = gamma_field_width_interactive(g);
    char *buffer = alloc_memory_for_board(g, max_width);
    if (buffer == NULL)
        return NULL;
    gamma_board_helper(g, max_width, buffer);
    return buffer;
}

bool gamma_write_field(gamma_t *g, char *s, uint32_t x, uint32_t y) {
    if (wrong_field(x, y, g))
        return false;
    uint32_t it = 0;
    uint32_t player = g->player_arr[x][y];
    if (player == NO_PLAYER) {
        s[it++] = NO_PLAYER_CHAR;
    } else {
        it = write_int(s, it, player);
    }
    uint32_t width = gamma_field_width_interactive(g);
    while (it < width)
        s[it++] = SPACE;
    return true;
}

bool gamma_move_possible(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (wrong_field(x, y, g) || wrong_player(player, g))
        return false;
    if (!gamma_move(g, player, x, y))
        return false;
    delete_util(g, x, y, player);
    return true;
}

uint32_t gamma_player_areas(gamma_t *g, uint32_t player) {
    if (wrong_player(player, g))
        return 0;
    return g->players[player - 1].num_of_areas;
}

uint32_t gamma_max_areas(gamma_t *g) {
    if (g == NULL)
        return 0;
    return g->max_num_of_areas;
}
