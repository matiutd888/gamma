/** @file
 * Zawiera implementację interfejsu stack.h
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */

#include "stack.h"
#include "memory_util.h"
#include "field.h"
#include <stdlib.h>

#define RESIZE_MULTIPLIER 2     /**< Współczynnik, o jaki zwiększany jest
                                 * maksymalny rozmiar stosu, gdy stos się zapełni.
                                 */
#define EXIT_CODE_MALLOC_FAIL 1 /**< Kod wyjściowy programu,
                                 * gdy nie uda się zaalokować potrzebnej pamięci.
                                 */
/** @brief Struktura reprezentująca stos przechowujący
 * dane typu @ref field. 
 * Implementacja opiera się na
 * dynamicznej tablicy. Przechowuje ilość elementow w stosie
 * i maksymalny rozmiar stosu.
 */
struct m_stack {
    uint64_t count; ///< Reprezentuje liczbę elementów na stosie.
    field_t *arr;   ///< Dynamiczna tablica przechowująca elementy na stosie.
    uint64_t size;  ///< Maksymalny rozmiar stosu.
};

stack_t *init_stack(uint64_t size) {
    stack_t *s = NULL;
    s = malloc(sizeof(stack_t));
    if (s == NULL)
        return s;
    if (!init_array_1D((void **) &(s->arr), FIELD_SIZE, size)) {
        free(s);
        return NULL;
    }
    s->size = size;
    s->count = 0;
    return s;
}

/** @brief Sprawdza, czy stos jest zapełniony.
 * Sprawdza, czy dla stosu reprezentowanego przez strukturę wskazywaną
 * przez @p s, ilość elementów jest równa jego maksymalnemu rozmiarowi.
 * @param[in] s     – wskaźnik na strukturę reprezentującą stos,
 *                    różny od @p NULL,
 * @return Wartość @p true, jeśli stos reprezentowany przez strukturę
 * wskazywaną przez @p s jest zapełniony (czyli ilość jego elementów jest
 * równa jego maksymalnemu rozmiarowi), wartość @p false w przeciwnym
 * przypadku.
 */
static bool is_full(stack_t *s) {
    return s->size == s->count;
}

bool is_stack_empty(stack_t *s) {
    return s->count == 0;
}

/** @brief Zwiększa maksymalny rozmiar stosu.
 * Zwiększa maksymalny rozmiar stosu reprezentowanego
 * przez strukturę wskazywaną przez @p s.
 * Rezerwuje pamięć dla stosu o takim
 * maksymalnym rozmiarze (dokonuje realokacji pamięci
 * przechowywanej przez dynamiczną tablicę @p s->arr).
 * Jeśli nie uda się tego dokonać (zabraknie pamięci),
 * terminuje program z kodem @p 1.
 * @param[in] s  – wskaźnik na strukturę reprezentującą stos,
 *                 różny od @p NULL.
 */
static void resize(stack_t *s) {
    s->size = RESIZE_MULTIPLIER * s->size + 1;
    s->arr = realloc(s->arr, FIELD_SIZE * s->size);
    if (s->arr == NULL)
        exit(EXIT_CODE_MALLOC_FAIL);
}

void stack_push(stack_t *s, uint32_t x, uint32_t y) {
    if (is_full(s))
        resize(s);
    s->arr[s->count].x = x;
    s->arr[s->count].y = y;
    s->count++;
}

field_t stack_pop(stack_t *s) {
    s->count--;
    return s->arr[s->count];
}

void stack_destruct(stack_t *s) {
    if (s == NULL)
        return;
    free(s->arr);
    free(s);
}
