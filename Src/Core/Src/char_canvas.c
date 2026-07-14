
#include <stdbool.h>
#include <string.h>
#include "main.h"
#include "char_canvas.h"

static uint16_t canvas_draw[ROW_SIZE_MAX * COLUMN_SIZE_MAX] = {0};
static uint16_t canvas[ROW_SIZE_MAX * COLUMN_SIZE_MAX] = {0};

static bool canvas_write = false;
static bool canvas_next = false;


void charCanvasInit(void)
{
#if 0
    uint8_t ch=0;
    for(int x=0; x<ROW_SIZE_PAL*COLUMN_SIZE; x++){
        canvas[0][x] = ch;
        canvas[1][x] = ch++;
    }
#else
    memset(canvas, 0, sizeof(canvas));
    memset(canvas_draw, 0, sizeof(canvas_draw));
#endif
}

void charCanvasClear(void)
{
    for (int x = 0; x < ROW_SIZE_MAX * COLUMN_SIZE_MAX; x++) {
        canvas[x] = ' ';
    }
}

void charCanvasDraw(void)
{
    if ( canvas_write ){
        memcpy(canvas_draw, canvas, sizeof(canvas_draw));
        canvas_next = true;
        canvas_write = false;
    };
}

void charCanvasWrite(uint8_t row, uint8_t column, uint8_t attr, uint8_t* data, uint8_t len)
{
    if (row < ROW_SIZE_PAL && column < COLUMN_SIZE){
        uint8_t font_page = attr & 0x03;
        uint8_t writable = COLUMN_SIZE - column;
        uint16_t *dst = &canvas[row*COLUMN_SIZE + column];

        if (len > writable) {
            len = writable;
        }

        for (uint8_t x = 0; x < len; x++) {
            uint16_t ch = ((uint16_t)font_page << 8) | data[x];
            dst[x] = (ch < FONT_CHARS) ? ch : data[x];
        }

        canvas_write = true;
    }
}

void charCanvasNext(void)
{
    if (canvas_next){
        canvas_next = false;
    }
}

uint16_t* charCanvasGet(uint16_t row)
{
    return &canvas_draw[row * COLUMN_SIZE];
}


