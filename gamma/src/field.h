/** @file
 * Zawiera deklarację i definicję struktury reprezentującej
 * pole na planszy.
 * Udostępnia metodę porównującą pola.
 *
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */

#ifndef GAMMA_FIELD_H
#define GAMMA_FIELD_H

#include <stdint.h>
#include <stdbool.h>

#define FIELD_SIZE sizeof(struct field)  ///< Rozmiar struktury @ref field.


/** @brief Struktura reprezentująca współrzędne pola na dwuwymiarowej planszy.
 * Zakładamy, że współrzędne mieszczą się w zakresie typu @p uint32_t.
 */
typedef struct field {
    uint32_t x; ///< Numer kolumny.
    uint32_t y; ///< Numer wiersza.
} field_t;

/** @brief Porównuje dwa pola.
 * @return Wartość @p true, jeśli pola @p f1 i @p f2
 * mają takie same współrzędne,
 * @p false w przeciwnym przypadku.
 */
static inline bool field_equals(field_t f1, field_t f2) {
    return f1.x == f2.x && f1.y == f2.y;
}

#endif //GAMMA_FIELD_H
