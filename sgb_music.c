//#pragma bank 1

#include <string.h>
#include <gb/gb.h>
#include <gb/sgb.h>
#include "sgb_snd_sfx.h"
#include "sgb_music.h"

#define SGB_VRAM_TRANSFER_BASE_ADDR ((uint8_t *)0x8000)
// wait for at least 6 frames after issued command
#define SGB_VRAM_TRANSFER_WAIT_FRAMES    for (uint8_t iii = 0; iii < 7; iii++) {vsync();}

uint8_t sgb_buf[20]; // theoretically only 16 bytes needed for SGB commands; but this memory is also used to prepare SGB VRAM transfers

//const uint8_t sgb_mus_table_start[] = {0x00, 0x2B};
const uint8_t sgb_mus_transfer_end[] = {0x00, 0x00, 0x00, 0x04}; // dw $0000, $0400 ; jumps to start of N-SPC driver
const uint8_t tune_table[] = { // tuning of 0 crashes the SPC :/
        1, 1, 1, 1, 1, 1, 1, 1,
        13, 13, 13, 13, 20, 20, 20, 20,
        40, 40, 40, 40, 1, 1, 1, 3, 1,
        1, 1, 1, 80, 80, 80,
        170, 170, 165, 165,
        55, 55, 55, 55, 1, 1,
        20, 40, 40, 40, 40, 1, 1, 53,
        1, 1, 1, 1, 1, 10, 1, 1,
        1, 1, 1, 1, 1, 1
};
//inline void sgb_prepare_vram_tranfer()

void sgb_music_transfer(const uint8_t *data, uint16_t len, uint16_t spc_dest_addr) {
    // prepare data for VRAM transfer
    DISPLAY_OFF;
    BGP_REG = OBP0_REG = OBP1_REG = 0xE4U;
    SCX_REG = SCY_REG = 0U;
    LCDC_REG |= LCDCF_BG8000;
    set_data(SGB_VRAM_TRANSFER_BASE_ADDR, (uint8_t *)&len, sizeof(uint16_t)); // LE len
    set_data(SGB_VRAM_TRANSFER_BASE_ADDR + 2, (uint8_t *)&spc_dest_addr, sizeof(uint16_t));
    set_data(SGB_VRAM_TRANSFER_BASE_ADDR + 4, data, len);
    set_data(SGB_VRAM_TRANSFER_BASE_ADDR + 4 + len, sgb_mus_transfer_end, sizeof (sgb_mus_transfer_end));
    HIDE_SPRITES, HIDE_WIN, SHOW_BKG;

    memset(sgb_buf, 0, sizeof (sgb_buf));
    // prepare tilemap for VRAM tranfer (must display all 256 tiles)
    uint8_t i = 0U;
    for (uint8_t y = 0; y != 14U; ++y) {
        uint8_t * dout = sgb_buf;
        for (uint8_t x = 0U; x != 20U; ++x) {
            *dout++ = i++;
        }
        set_bkg_tiles(0, y, 20, 1, sgb_buf);
    }
    DISPLAY_ON; // display must be on for tranfer to work

    sgb_buf[0] = (SGB_SOU_TRN << 3) | 1;
    memset(sgb_buf + 1, 0, sizeof (sgb_buf) - 1);

    // trigger VRAM transfer
    sgb_transfer(sgb_buf);
    // wait for the VRAM transfer to complete
    SGB_VRAM_TRANSFER_WAIT_FRAMES
}

void sgb_music_play(uint8_t music_num) { // 0x01-0x0f
    sgb_buf[0] = (SGB_SOUND << 3) | 1;  // 1 is number of packets to transfer
    sgb_buf[1] = SGB_SND_EFFECT_STOP;  // Effect A
    sgb_buf[2] = SGB_SND_EFFECT_STOP;  // Effect B
    sgb_buf[3] = 0;
    sgb_buf[4] = music_num; // Music num
    sgb_transfer(sgb_buf);
}