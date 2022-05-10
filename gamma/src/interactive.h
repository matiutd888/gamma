/** @file
 * Udostępnia funkcję przeprowadzającą tryb interaktywny
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 12.05.2020
 */

#include "gamma.h"

#ifndef GAMMA_INTERACTIVE_H
#define GAMMA_INTERACTIVE_H

#endif //GAMMA_INTERACTIVE_H

/**
 * @brief Przechodzi w tryb interaktywny i go przeprowadza.
 * @param[in,out] g    – wskaźnik na strukturę reprezentującą grę,
 *                       której plansza ma być wywietlana podczas trybu
 *                       interaktywnego.
 * @param[in] numbers  – tablica numerów opisujących grę, występujących
 *                       po znaku @p I w poleceniu wywołującym tryb
 *                       interaktywny.
 * @return Wartość @p true, jeśli udało się przejść w tryb interaktywny,
 * wartość @p false w przeciwnym przypadku.
 */
bool interactive(gamma_t *g, uint32_t *numbers);
