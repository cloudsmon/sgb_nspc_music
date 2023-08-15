#include <gb/gb.h>
#include <gb/sgb.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <gbdk/console.h>
#include <gbdk/font.h>

#include "sgb_snd_sfx.h"
#include "sgb_sfx_names.h"
#include "sgb_music.h"

#include "nspc_song_data.h"

// Stores button presses
uint8_t keys = 0x00, keys_last = 0x00;


// Default SFX attributes
uint8_t sgb_sfx_a_pitch = SGB_SND_PITCH_HI; // Bits 0..1
uint8_t sgb_sfx_a_vol   = SGB_SND_VOL_HI;   // Bits 2..3
uint8_t sgb_sfx_b_pitch = SGB_SND_PITCH_HI; // Bits 4..5
uint8_t sgb_sfx_b_vol   = SGB_SND_VOL_HI;   // Bits 6..7


// Default values for SFX types
uint8_t sgb_num_a = 0;
uint8_t sgb_num_b = SGB_SND_EFFECT_B_MIN;
uint8_t instr_opcode_idx = 0;
uint8_t sgb_music_paused = 0;

extern uint8_t sgb_buf[20]; // Should be sized to fit max payload bytes + 1 for command byte


// Play a SGB sound effect
//
// Setting either effect parameter to SGB_SND_EFFECT_EMPTY will skip it.
// The pitch and volume settings are not changed in this example, but could be.
void sgb_sound_effect(uint8_t sfx_a, uint8_t sfx_b) {
    sgb_buf[0] = (SGB_SOUND << 3) | 1;  // 1 is number of packets to transfer
    sgb_buf[1] = sfx_a;  // Effect A
    sgb_buf[2] = sfx_b;  // Effect B
    sgb_buf[3] = sgb_sfx_a_pitch | (sgb_sfx_a_vol << 2) | (sgb_sfx_b_pitch << 4) | (sgb_sfx_b_vol << 6);
    sgb_buf[4] = SGB_MUSIC_SCORE_CODE_NONE; // Must be zero if not used
    sgb_transfer(sgb_buf);
}




#define DISP_SFX_A_START 3
#define DISP_SFX_B_START 10

// Display basic operation info on the screen
void init_display(void) {
    gotoxy(0,1);
    printf("SGB SOU_TRN DEMO");

    gotoxy(0,DISP_SFX_A_START);
    printf("INSTR:\n");
    printf(" TYPE: UP / DOWN\n");
    printf(" OPcode addr:\n");
    printf(" NEXT: SELECT\n");
    printf(" SOU_TRN: START\n");

    gotoxy(0,DISP_SFX_B_START);
    printf("SONG NR:\n");
    printf(" TYPE: LEFT / RIGHT\n");
    printf(" PLAY: A\n");
    printf(" STOP: B\n");
}


// Update the display if either of the sfx types have changed
void update_display(void) {

    gotoxy(8u, DISP_SFX_A_START);
    printf("0x%hx", (uint8_t)sgb_num_a);
    gotoxy(13u, DISP_SFX_A_START + 2u);
    printf("0x%x", (uint16_t)nspc_instr_addr[instr_opcode_idx]);
    gotoxy(1u, DISP_SFX_A_START + 5u);
    printf("                    ");
    gotoxy(1u, DISP_SFX_A_START + 5u);
    printf("%s", (const char *)sgb_sfx_names_table_a[sgb_num_a]);


    gotoxy(8u, DISP_SFX_B_START);
    printf("0x%hx", (uint8_t)sgb_num_b);
    gotoxy(1u, DISP_SFX_B_START + 4u);
    printf("                    ");
    gotoxy(1u, DISP_SFX_B_START + 4u);
    printf("%s", (const char *)sgb_sfx_names_table_b[(sgb_num_b-1) > 2? 2 : (sgb_num_b-1)]);
}

void sgb_update_instr(void) { // overwrite instrument and tuning at pregenerated address
    sgb_music_play(0xF0);
    uint8_t tmp[2];
    tmp[0] = sgb_num_a; // instr nr
    tmp[1] = tune_table[sgb_num_a];
    sgb_music_transfer(tmp, 2, nspc_instr_addr[instr_opcode_idx]);
    font_init();
    init_display();
    update_display();
}

// Process button presses
void handle_input(void) {

    bool display_update_queued = false;

    // Filter so only buttons newly pressed have their bits set
    switch ((keys ^ keys_last) & keys) {
        case J_A:
                sgb_music_play(sgb_num_b);
                break;

        // Effect "B" playback controls
        case J_B:
                if (!sgb_music_paused) {
                    if (keys & J_A) {
                        sgb_music_play(SGB_SPC_MUS_STOP); // hard-reset?
                    } else {
                        sgb_music_play(SGB_SPC_MUS_PAUSE);
                    }
                    sgb_music_paused = true;
                } else {
                    // TODO does not work?
                    //sgb_music_play(SGB_SPC_MUS_RESUME);
                    sgb_music_paused = false;
                }
                break;


        // Effect type selectors
        case (J_UP): sgb_num_a++;
                  if (sgb_num_a >= SGB_NUM_INSTR) sgb_num_a = 0;
                  display_update_queued = true;
                  break;

        case (J_DOWN): sgb_num_a--;
                 if (sgb_num_a == 0xFF) sgb_num_a = SGB_NUM_INSTR - 1;
                  display_update_queued = true;
                 break;

        case (J_RIGHT): sgb_num_b++;
                  if (sgb_num_b > SGB_SND_MUS_MAX) sgb_num_b = SGB_SND_MUS_MIN;
                  display_update_queued = true;
                  break;

        case (J_LEFT): sgb_num_b--;
                  if (sgb_num_b < SGB_SND_MUS_MIN) sgb_num_b = SGB_SND_MUS_MAX;
                  display_update_queued = true;
                  break;
        case (J_SELECT): instr_opcode_idx++;
                if (instr_opcode_idx >= NSPC_FOUND_INSTR_OPCODES) instr_opcode_idx = 0;
                display_update_queued = true;
            break;
        case (J_START):
                sgb_update_instr();
            break;
    }

    if (display_update_queued)
        update_display();
}



void main(void) {

    // Wait 4 frames
    // For SGB on PAL SNES this delay is required on startup, otherwise borders don't show up
    for (uint8_t i = 4; i != 0; i--) vsync();

    DISPLAY_ON;

    if (sgb_check()) {
        sgb_music_transfer(nspc_song_data, sizeof(nspc_song_data), SGB_SPC_START_ADDR);
        init_display();
        update_display();

        while(1) {
            vsync();

            keys_last = keys;
            keys = joypad();

            handle_input();
       }
    } else {
        printf("NO SGB DETECTED.\nThis program plays\nSNES audio via\nSGB.\nIt needs real HW\n"
               "or an emulator that\ncan do SOU_TRN");
    }

}
