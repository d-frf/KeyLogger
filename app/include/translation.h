#ifndef TRANSLATION_H
#define TRANSLATION_H

#if defined(__linux__)

#include <linux/input.h>

struct km_trans_t;

typedef struct km_trans_t keymap_translator_t;


/*
 * Initializes a translator using the system's active XKB keymap.
 * Returns 0 on success, -1 on failure.
 */
int keymap_translator_init(keymap_translator_t **);


/*
 * Translates a single struct input_event (from evdev) into a UTF-8 string.
 *
 * - Updates internal modifier state (shift, ctrl, altgr, caps, etc.)
 * - Writes the resulting character(s) into `out` (must be at least 8 bytes)
 * - Returns the number of bytes written (0 if the key produces no character,
 *   e.g. modifier keys, key-up events, or non-key events)
 */
int keymap_translate_event(keymap_translator_t*,const struct input_event*,char*,size_t);

/*
 * Frees all resources associated with the translator.
 */
void keymap_translator_destroy(keymap_translator_t *);

#endif

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

// On Windows, the "state" is managed natively by the OS layout thread.
typedef struct {
    HKL layout; // Handle to the keyboard layout
} keymap_translator_t;

int keymap_translator_init(keymap_translator_t **kt);
int keymap_translate_event(keymap_translator_t *kt, WPARAM wParam, LPARAM lParam, char *out, size_t out_size);
void keymap_translator_destroy(keymap_translator_t *kt);

#endif

#endif