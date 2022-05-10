/** @file
 * Interfejs udostępniający metody pomagające alokować
 * i zwalniać pamięć dla dynamicznych tablic.
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @copyright Uniwersytet Warszawski
 * @date 07.04.2020
 */


#ifndef GAMMA_MEMORY_UTIL_H
#define GAMMA_MEMORY_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/** @brief Alokuje pamięć dla tablicy dwuwymiarowej.
 * Dokonuje alokacji pamięci dla dynamicznej tablicy dwuwymiarowej o wymiarach
 * @p [width][height], przechowującej dane typu o rozmiarze @p type, w miejscu
 * w pamięci wskazywanym przez @p data_ptr.
 * @param[out] data_ptr     – adres zmiennej, będącej wskaźnikiem na wskaźnik
 *                            pewnego typu,
 * @param[in] type          – rozmiar typu danych, który ma przechowywać tablica,
 * @param[in] width, height – wymiary tablicy dwuwymiarowej (@p [width][height])
 * @return Wartosć @p true, jeśli udało się zaalokować pamięć, @p false w
 * w przeciwnym przypadku.
 */
bool init_arr_2D(void ***data_ptr, size_t type, uint64_t width, uint64_t height);

/** @brief Alokuje pamięć dla tablicy jednowymiarowej.
 * Dokonuje alokacji pamięci dla dynamicznej tablicy o długosci
 * @p len, przechowującej dane typu o rozmiarze @p type, w miejscu
 * w pamięci wskazywanym przez @p data_ptr.
 * @param[out] data_ptr  – adres zmiennej, będącej wskaźnikiem pewnego typu,
 * @param[in] type       – rozmiar typu danych, który ma przechowywać tablica,
 * @param[in] len        – długość tablicy.
 * @return Wartosć @p true, jeśli udało się zaalokować pamięć, @p false w
 * w przeciwnym przypadku.
 */
bool init_array_1D(void **data_ptr, size_t type, uint64_t len);

/** @brief Zwalnia pamięć zaalokowaną dla tablicy dwuwymiarowej.
 * Dla @p i należącego od @p 0 do @p width @p - @p 1, zwalnia pamięć
 * pod adresem @p arr[i] (Jeżeli @p arr[i] funkcja nic nie zmienia). 
 * Na koniec zwalnia pamięć zaalokowaną pod adresem
 * @p arr. Jeżeli @p arr jest równe @p NULL, funkcja nic nie robi.
 * @param[in,out] arr – dynamiczna tablica dwuwymiarowa,
 * @param[in] width   – ilość kolumn (pierwsza współrzędna) w tablicy.
 */
void free_array(void **arr, uint64_t width);

#endif //GAMMA_MEMORY_UTIL_H
