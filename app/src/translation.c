#include <stdio.h>
#include <stdlib.h>
#include "../include/translation.h"



#if defined(__linux__)

#include <string.h>
#include <xkbcommon/xkbcommon.h>
#include <linux/input.h>


struct km_trans_t {
    struct xkb_context *ctx;
    struct xkb_keymap  *keymap;
    struct xkb_state   *state;
};

/*
* Initializes a translator using the system's active XKB keymap.
* Returns 0 on success, -1 on failure.
*/
int keymap_translator_init(keymap_translator_t **kt) {
    
    struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!ctx) return -1;
    
    struct xkb_keymap  *keymap = xkb_keymap_new_from_names(ctx,NULL,XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap){
        xkb_context_unref(ctx);
        return -1;
    }
    
    
    struct xkb_state   *state = xkb_state_new(keymap);
    if (!state) {
        xkb_keymap_unref(keymap);
        xkb_context_unref(ctx);
        return -1;
    }
    
    *kt = malloc(sizeof(struct km_trans_t));
    if (!*kt) {
        xkb_state_unref(state);
        xkb_keymap_unref(keymap);
        xkb_context_unref(ctx);
        return -1;
    }
    
    memset(*kt,0,sizeof(**kt));
    
    (*kt)->ctx = ctx;
    (*kt)->keymap = keymap;
    (*kt)->state = state;
    
    return 0;
    
}

/*
* Translates a single struct input_event (from evdev) into a UTF-8 string.
*
* - Updates internal modifier state (shift, ctrl, altgr, caps, etc.)
* - Writes the resulting character(s) into `out` (must be at least 8 bytes)
* - Returns the number of bytes written (0 if the key produces no character,
*   e.g. modifier keys, key-up events, or non-key events)
*/
int keymap_translate_event(keymap_translator_t *kt,
    const struct input_event *ev,
    char *out, size_t out_size)
    {
        out[0] = '\0';
        
        if (ev->type != EV_KEY)
        return 0;
        
        // evdev keycodes are offset by 8 relative to XKB keycodes
        xkb_keycode_t keycode = ev->code + 8;
        
        // event.value: 0 = release, 1 = press, 2 = autorepeat
        enum xkb_key_direction dir = ev->value ? XKB_KEY_DOWN : XKB_KEY_UP;
        xkb_state_update_key(kt->state, keycode, dir);
        
        // Only produce output on press/repeat, not on release
        if (ev->value == 0)
        return 0;
        
        int n = xkb_state_key_get_utf8(kt->state, keycode, out, out_size);
        return n;
    }
    
    /*
    * Frees all resources associated with the translator.
    */
   void keymap_translator_destroy(keymap_translator_t *kt) {
       if (kt->state)  xkb_state_unref(kt->state);
       if (kt->keymap) xkb_keymap_unref(kt->keymap);
       if (kt->ctx)    xkb_context_unref(kt->ctx);
       memset(kt, 0, sizeof(*kt));
    }
#endif


#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

int keymap_translator_init(keymap_translator_t **kt) {
    *kt = malloc(sizeof(keymap_translator_t));
    if (!*kt) return -1;

    // Fetch the active keyboard layout for the current thread
    (*kt)->layout = GetKeyboardLayout(0);
    return 0;
}

int keymap_translate_event(keymap_translator_t *kt, WPARAM wParam, LPARAM lParam, char *out, size_t out_size) {
    out[0] = '\0';

    // wParam contains the message type: WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, etc.
    // Filter to only process key presses (similar to ev->value != 0 on Linux)
    if (wParam != WM_KEYDOWN && wParam != WM_SYSKEYDOWN) {
        return 0;
    }

    // lParam is a pointer to a KBDLLHOOKSTRUCT when using a low-level hook
    KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;

    // Read the current state of all virtual keys (Shift, CapsLock, Ctrl, etc.)
    BYTE keyboard_state[256];
    if (!GetKeyboardState(keyboard_state)) {
        // Fallback: Manually query async states if thread context isn't fully attached
        for (int i = 0; i < 256; i++) {
            keyboard_state[i] = (GetKeyState(i) & 0x8000) ? 0x80 : 0;
        }
        // Retain toggle keys like Caps Lock
        keyboard_state[VK_CAPITAL] = GetKeyState(VK_CAPITAL) & 0x01;
    }

    WCHAR w_buffer[4] = {0};
    // Translate the virtual key code and scan code into UTF-16 wide characters
    int result = ToUnicodeEx(
        kbd->vkCode,
        kbd->scanCode,
        keyboard_state,
        w_buffer,
        4,
        0,
        kt->layout
    );

    if (result > 0) {
        // Convert the UTF-16 wide string into a UTF-8 narrow string for output
        int bytes_written = WideCharToMultiByte(
            CP_UTF8,
            0,
            w_buffer,
            result,
            out,
            (int)out_size - 1,
            NULL,
            NULL
        );
        if (bytes_written > 0) {
            out[bytes_written] = '\0';
            return bytes_written;
        }
    }

    return 0;
}

void keymap_translator_destroy(keymap_translator_t *kt) {
    if (kt) {
        free(kt);
    }
}

#endif