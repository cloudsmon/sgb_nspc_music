#ifndef _SGB_MUSIC_H
#define _SGB_MUSIC_H
#include <stddef.h>

#define SGB_SND_MUS_MIN 0x01
#define SGB_SND_MUS_MAX 0x0F
#define SGB_SPC_START_ADDR 0x2b00

#define SGB_SPC_MUS_PAUSE 0xF0
// TODO resume does not work?
#define SGB_SPC_MUS_RESUME 0xF1
#define SGB_SPC_MUS_STOP 0x80

#define SGB_NUM_INSTR 63
extern const uint8_t tune_table[SGB_NUM_INSTR];

void sgb_music_transfer(const uint8_t *data, uint16_t len, uint16_t spc_dest_addr);
void sgb_music_play(uint8_t music_num);

#endif