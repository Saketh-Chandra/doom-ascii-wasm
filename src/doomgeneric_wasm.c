#include "doomgeneric.h"
#include "g_game.h"
#include "d_player.h"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ASCII character palette for brightness levels (darkest to brightest)
// Simplified palette for clarity - fewer chars with more distinct visual weight
// Each character has clear visual difference for better readability
static const char ASCII_CHARS[] = " .:;+=xX$@";

// Frame buffer dimensions - optimized for Raycast display
// With -scaling 3: DOOMGENERIC becomes 106x66 (320/3 x 200/3)
// We render the top portion (excluding status bar) to fill our 106x20 output
#define FRAME_WIDTH 106
#define FRAME_HEIGHT 20
// Status bar takes ~32 pixels of 200, which is ~10 rows in 66-row buffer
#define STATUS_BAR_ROWS 10

// JavaScript callback declarations
extern void JS_RenderASCIIFrame(const char* frame, int width, int height);
extern int JS_GetNextKey(void);
extern void JS_Log(const char* message);

// Static buffers
static char ascii_frame_buffer[FRAME_WIDTH * FRAME_HEIGHT + 1];
static int input_queue[256];
static int queue_head = 0;
static int queue_tail = 0;
static uint32_t start_time_ms = 0;

// Initialize
void DG_Init() {
    memset(ascii_frame_buffer, ' ', FRAME_WIDTH * FRAME_HEIGHT);
    ascii_frame_buffer[FRAME_WIDTH * FRAME_HEIGHT] = '\0';
    start_time_ms = (uint32_t)emscripten_get_now();
    JS_Log("Doom WASM initialized");
}

// Convert pixel buffer to ASCII and send to JavaScript
void DG_DrawFrame() {
    // DG_ScreenBuffer is the framebuffer from doom-ascii (DOOMGENERIC_RESX x DOOMGENERIC_RESY)
    // With -scaling 3: 106x66 buffer (320/3 x 200/3)
    // Pixels are stored as uint32_t in BGRA format (little-endian: 0xAARRGGBB in memory = BGRA bytes)
    
    // Calculate viewport height (exclude status bar at bottom)
    unsigned viewport_height = DOOMGENERIC_RESY - STATUS_BAR_ROWS;
    
    // Direct mapping: 106-width buffer maps 1:1 to 106-width output
    // Viewport height (56 rows) maps to 20 output rows
    float x_scale = (float)DOOMGENERIC_RESX / (float)FRAME_WIDTH;
    float y_scale = (float)viewport_height / (float)FRAME_HEIGHT;
    
    for (int y = 0; y < FRAME_HEIGHT; y++) {
        for (int x = 0; x < FRAME_WIDTH; x++) {
            // Calculate the pixel block to average
            int x_start = (int)(x * x_scale);
            int y_start = (int)(y * y_scale);
            int x_end = (int)((x + 1) * x_scale);
            int y_end = (int)((y + 1) * y_scale);
            
            // Clamp to bounds
            if (x_end > (int)DOOMGENERIC_RESX) x_end = DOOMGENERIC_RESX;
            if (y_end > (int)viewport_height) y_end = viewport_height;
            
            // Average the pixels in this block
            int sum_r = 0, sum_g = 0, sum_b = 0;
            int count = 0;
            
            for (int py = y_start; py < y_end; py++) {
                for (int px = x_start; px < x_end; px++) {
                    uint32_t pixel = DG_ScreenBuffer[py * DOOMGENERIC_RESX + px];
                    // Extract RGB from BGRA format (0xAARRGGBB)
                    sum_b += pixel & 0xFF;
                    sum_g += (pixel >> 8) & 0xFF;
                    sum_r += (pixel >> 16) & 0xFF;
                    count++;
                }
            }
            
            // Calculate average RGB
            unsigned char r = count > 0 ? sum_r / count : 0;
            unsigned char g = count > 0 ? sum_g / count : 0;
            unsigned char b = count > 0 ? sum_b / count : 0;
            
            // Calculate brightness (0-255) using standard luminosity formula
            int brightness = (r * 299 + g * 587 + b * 114) / 1000;
            
            // Map to ASCII character
            int char_index = (brightness * (strlen(ASCII_CHARS) - 1)) / 255;
            ascii_frame_buffer[y * FRAME_WIDTH + x] = ASCII_CHARS[char_index];
        }
    }
    
    // Send to JavaScript
    JS_RenderASCIIFrame(ascii_frame_buffer, FRAME_WIDTH, FRAME_HEIGHT);
}

// Sleep implementation
void DG_SleepMs(uint32_t ms) {
    // Use emscripten_sleep for async sleep
    emscripten_sleep(ms);
}

// Get millisecond timestamp
uint32_t DG_GetTicksMs() {
    return (uint32_t)(emscripten_get_now() - start_time_ms);
}

// Get keyboard input
// Key codes: positive = key press, negative = key release
int DG_GetKey(int* pressed, unsigned char* key) {
    int k = JS_GetNextKey();
    if (k == 0) {
        return 0;
    }
    // Negative values indicate key release
    if (k < 0) {
        *pressed = 0;
        *key = (unsigned char)(-k);
    } else {
        *pressed = 1;
        *key = (unsigned char)k;
    }
    return 1;
}

// Read input (called by doom game loop)
void DG_ReadInput() {
    // Input is handled through WASM_QueueKey and DG_GetKey
    // This function can be empty for our implementation
}

// Window title (no-op for Raycast)
void DG_SetWindowTitle(const char* title) {
    // Not needed for Raycast - could log for debugging
    char msg[256];
    snprintf(msg, sizeof(msg), "Window title: %s", title);
    JS_Log(msg);
}

// Exported function for JavaScript to queue key presses
EMSCRIPTEN_KEEPALIVE
void WASM_QueueKey(int keycode) {
    input_queue[queue_tail] = keycode;
    queue_tail = (queue_tail + 1) % 256;
    
    // Log for debugging
    char msg[64];
    snprintf(msg, sizeof(msg), "Queued key: %d", keycode);
    JS_Log(msg);
}

// ========== Player Status Exports for MenuBarExtra ==========

// Get player health (0-100+)
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerHealth(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    if (players[consoleplayer].mo) {
        return players[consoleplayer].mo->health;
    }
    return players[consoleplayer].health;
}

// Get player armor points (0-200)
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerArmor(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    return players[consoleplayer].armorpoints;
}

// Get current weapon (0-8, see weapontype_t enum)
// 0=fist, 1=pistol, 2=shotgun, 3=chaingun, 4=rocket, 5=plasma, 6=bfg, 7=chainsaw, 8=ssg
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerWeapon(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    return (int)players[consoleplayer].readyweapon;
}

// Get ammo for specific type (0=bullets, 1=shells, 2=cells, 3=rockets)
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerAmmo(int ammo_type) {
    extern player_t players[];
    extern int consoleplayer;
    
    if (ammo_type >= 0 && ammo_type < 4) { // NUMAMMO = 4
        return players[consoleplayer].ammo[ammo_type];
    }
    return 0;
}

// Get max ammo for specific type
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerMaxAmmo(int ammo_type) {
    extern player_t players[];
    extern int consoleplayer;
    
    if (ammo_type >= 0 && ammo_type < 4) {
        return players[consoleplayer].maxammo[ammo_type];
    }
    return 0;
}

// Get current ammo for equipped weapon
EMSCRIPTEN_KEEPALIVE
int WASM_GetCurrentWeaponAmmo(void) {
    extern player_t players[];
    extern int consoleplayer;
    extern weaponinfo_t weaponinfo[];
    
    weapontype_t weapon = players[consoleplayer].readyweapon;
    if (weapon >= 0 && weapon < 9) { // NUMWEAPONS
        ammotype_t ammo_type = weaponinfo[weapon].ammo;
        if (ammo_type == 4) { // am_noammo (chainsaw/fist)
            return -1; // Indicate unlimited
        }
        return players[consoleplayer].ammo[ammo_type];
    }
    return 0;
}

// Check if player has specific key card (0-5: blue, yellow, red, blue skull, yellow skull, red skull)
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerHasKey(int key_id) {
    extern player_t players[];
    extern int consoleplayer;
    
    if (key_id >= 0 && key_id < 6) { // NUMCARDS = 6
        return players[consoleplayer].cards[key_id] ? 1 : 0;
    }
    return 0;
}

// Get kill count
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerKills(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    return players[consoleplayer].killcount;
}

// Get item count
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerItems(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    return players[consoleplayer].itemcount;
}

// Get secret count
EMSCRIPTEN_KEEPALIVE
int WASM_GetPlayerSecrets(void) {
    extern player_t players[];
    extern int consoleplayer;
    
    return players[consoleplayer].secretcount;
}
