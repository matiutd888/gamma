/** @file
 * Interfejs udostępniający funkcje
 * pomagające wpisywać reprezentacje napisowe
 * liczb do buforów
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.05.2020
 */

#ifndef GAMMA_INT_TO_STRING_UTIL_H
#define GAMMA_INT_TO_STRING_UTIL_H

#include <stdint.h>

/** @brief Wpisuje liczbę do napisu.
 * Zapisuje zapis dziesiętny liczby @p n w napisie @p s zaczynając od pozycji
 * @p l.
 * @param[out] s – łańcuch znaków, do którego wpisana ma zostać liczba.
 *                 Funkcja zakłada, że jego długość jest niemniejsza niż
 *                 suma @p l i liczby cyfr @p n minus 1.
 * @param[in] l  – pozycja na której ma zostać wpisana pierwsza cyfra zapisu
 *                 dziesiętnego @p n w napisie @p s, nieujemna liczba naturalna
 * 				   mniejsza od długosci @p s,
 * @param[in] n  – liczba naturalna.
 * @return Suma @p l i liczby cyfr w zapisie dziesiętnym @p n.
 */
uint64_t write_int(char *s, uint64_t l, uint32_t n);

/** @brief Zwraca liczbę cyfr w zapisie dziesiętnym.
 * Oblicza i zwraca liczbę cyfr w zapisie dziesiętnym nieujemnej liczby
 * całkowitej @p n.
 * @param[in] n – nieujemna liczba całkowita.
 * @return Liczba cyfr @p n.
 */
uint32_t digit_count(uint64_t n);

#endif //GAMMA_INT_TO_STRING_UTIL_H
