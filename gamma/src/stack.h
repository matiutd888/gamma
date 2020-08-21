/** @file
 * Interfejs klasy implementującej stos struktur reprezentujących
 * pole.
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */
#ifndef GAMMA_STACK_H
#define GAMMA_STACK_H

#include <stdint.h>
#include <stdbool.h>
#include "field.h"

/** @brief Struktura reprezentująca stos przechowujący
 * dane typu @ref field. 
 * Implementacja opiera się na
 * dynamicznej tablicy.
 */
typedef struct m_stack stack_t;

/** @brief Dokonuje inicjacji stosu.
 * Alokuje pamięć dla stosu o maksymalnym rozmiarze (ilości elementów)
 * @p size (oznacza to tyle, że dokona alokacji dla dynamicznej tablicy
 * zdolnej przechować @p size elementów typu @ref field). Zwraca
 * wskaźnik na strukturę reprezentującą ten stos, lub wartość @p
 * NULL, jeśli nie udało się zaalokować potrzebnej pamieci.
 * @param[in] size – początkowy maksymalny rozmiar stosu.
 * @return Wskaźnik na strukturę reprezentującą stos, lub
 * @p NULL, jeśli nie uda się zaalokować potrzebnej pamieci.
 */
stack_t *init_stack(uint64_t size);

/** @brief Sprawdza, czy stos jest pusty.
 * Sprawdza czy stos reprezentowany przez strukturę wskazywaną przez
 * @p s jest pusty.
 * @param[in] s – wskaźnik na strukturę reprezentującą stos, różny od @p NULL.
 * @return Wartość @p true, jeśli stos reprezentowany przez strukturę
 * wskazywaną przez @p s jest pusty, wartość @p false w przeciwnym przypadku.
 */
bool is_stack_empty(stack_t *s);

/** @brief Dodaje element do stosu.
 * Dodaje strukturę @p field reprezentującą pole (@p x, @p y)
 * do stosu reprezentowanego przez strukturę wskazywaną przez
 * @p s. Jeśli dokonanie takiej operacji spowodowałoby, że
 * ilość elementów na stosie byłaby większa od jego maksymalnego
 * rozmiaru, zwiększa jego maksymalny rozmiar.
 * Jeśli nie jest to możliwe (nie uda się zaalokować pamięci),
 * terminuje program z kodem @p 1.
 * @param[in,out] s – wskaźnik na strukturę reprezentującą stos,
 *                    różny od @p NULL,
 * @param[in] x     – numer kolumny,
 * @param[in] y     – numer wiersza.
 */

void stack_push(stack_t *s, uint32_t x, uint32_t y);

/** @brief Usuwa i zwraca pierwszy element ze stosu.
 * Zdejmuje i zwraca pierwszy element na niepustym stosie
 * reprezentowanym przez strukturę wskazywaną przez @p s.
 * @param[in,out] s – wskaźnik na strukturę reprezentującą niepusty
 *                     stos, wskaźnik ten musi być różny od @p NULL.
 * @return Struktura typu @p field będąca pierwszym elementem stosu
 * reprezentowanego przez strukturę wskazywaną przez @p s w momencie
 * wywołania funkcji.
 */
field_t stack_pop(stack_t *s);

/** Zwalnia pamięć zarezerwowaną dla stosu.
 * Zwalnia pamięć zarezerwowaną dla struktury wskazywanej
 * przez @p s. Jeżeli @p s jest równe @p NULL, funkcja nic nie robi.
 * @param[in,out] s – wskaźnik na strukturę reprezentującą stos.
 */
void stack_destruct(stack_t *s);

#endif //GAMMA_STACK_H
