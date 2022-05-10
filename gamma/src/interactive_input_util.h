/** @file
 * Udostępnia wczytywanie w trybie interaktywnym.
 * Kod zaczerpnięty z odpowiedzi na pytanie zadane na forum @p stackoverflow
 * znajdujący się pod adresem
 * https://stackoverflow.com/questions/33025599/move-the-cursor-in-a-c-program.
 * Na potrzeby zadania dokonane zostały na nim pewne modyfikacje.
 *
 * @author David Ranieri https://stackoverflow.com/users/1606345/david-ranieri
 * @author Mateusz Nowakowski <mn418323@mimuw.edu.pl>
 * @date 12.05.2020
 */

#ifndef GAMMA_INTERACTIVE_INPUT_UTIL_H
#define GAMMA_INTERACTIVE_INPUT_UTIL_H

#define KEY_ESCAPE  0x001b ///< Kod klawisza @p ESCAPE.
#define KEY_UP      0x0105 ///< Kod reprezentujący strzałkę w górę, @p 261.
#define KEY_DOWN    0x0106 ///< Kod reprezentujący strzałkę w dół, @p 262.
#define KEY_LEFT    0x0107 ///< Kod reprezentujący strzałkę w lewo, @p 263.
#define KEY_RIGHT   0x0108 ///< Kod reprezentujący strzałkę w prawo, @p 264.

/**
 * @brief Wczytuje klawisz.
 * Rozpoznaje i zwraca kod reprezentujący wczytany klawisz.
 * @return Kod reprezentujący wczytany klawisz,
 * lub wartość @p 0, jeśli nie udało się go rozpoznać.
 */
int kbget();

/**
 * @brief Przywraca ustawienia terminala zapisane w podczas wywołania funkcji
 * @ref setup_terminal.
 */
void restore_terminal();

/**
 * @brief Przygotowuje terminal pod wczytywanie w trybie
 * interaktywnym.
 * Zapisuje w zmiennej globalnej obecne ustawienia terminala.
 * Jeżeli nie uda się tego zrobić, kończy program z kodem
 * @p 1. Przechodzi do wczytywania bez czekania na @p ENTER
 * i bez wyświetlania wczytywanych znaków.
 */
void setup_terminal();

#endif //GAMMA_INTERACTIVE_INPUT_UTIL_H
