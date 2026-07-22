
#include <stdbool.h>
#include <string.h>
#include "stm32g4xx.h"
#include "stm32g4xx_ll_dma.h"
#include "main.h"
#include "char_canvas.h"
#include "setting.h"

typedef void (*videoGenFunc)(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenEpNTSC(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenEpPAL(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenBLK(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenVSYN(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenVSYNEven(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenVSYNOdd(uint16_t *buf, uint16_t size, uint16_t line);
void videoGenHSYN(uint16_t *buf, uint16_t size, uint16_t line);
void videoGen1stData(uint16_t *buf, uint16_t size, uint16_t line);
void videoGen2ndData(uint16_t *buf, uint16_t size, uint16_t line);

int32_t canvas_v_offset_gen[]  ={CANVAS_V_OFFSET_NTSC_GEN, CANVAS_V_OFFSET_PAL_GEN};

#define VG_SYN  0
#define VG_BLK  0x0180
#define VG_GRY  0x0280
#define VG_WHI  0x0400

const videoGenFunc videoGenFuncTable[2][1251] = {
    {
        // ntsc
        videoGenEpNTSC,   videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC,     // 1-6
        videoGenVSYN,     videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYNEven,   // 7-12
        videoGenEpNTSC,   videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC,     // 13-18
        videoGenHSYN,     videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK, // 19-30
        videoGenHSYN,     videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 31-40
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 41-50
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 51-60
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 61-70
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 71-80
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 81-90
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 91-100
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 101-110
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 111-120
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 121-130
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 131-140
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 141-150
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 151-160
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 161-170
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 171-180
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 181-190
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 191-200
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 201-210
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 211-220
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 221-230
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 231-240
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 241-250
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 251-260
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 261-270
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 271-280
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 281-290
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 291-300
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 301-310
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 311-320
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 321-330
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 331-340
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 341-350
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 351-360
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 361-370
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 371-380
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 381-390
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 391-400
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 401-410
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 411-420
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 421-430
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 431-440
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 441-450
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 451-460
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 461-470
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 471-480
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 481-490
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 491-500
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 501-510
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 511-520(260)
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData, // 521-525
        videoGenEpNTSC,   videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC,     // 526-531
        videoGenVSYN,     videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYNOdd,   // 532-537
        videoGenEpNTSC,   videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC, videoGenEpNTSC,      // 538-543
        videoGenBLK,      videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,videoGenBLK,videoGenHSYN,    // 544-555
        videoGenBLK,      videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,    // 556-565
        videoGenBLK,      videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,    // 566-570
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 571-580
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 581-590
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 591-600
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 601-610
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 611-620
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 621-630
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 631-640
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 641-650
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 651-660
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 661-670
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 671-680
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 681-690
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 691-700
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 701-710
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 711-720
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 721-730
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 731-740
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 741-750
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 751-760
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 761-770
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 771-780
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 781-790
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 791-800
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 801-810
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 811-820
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 821-830
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 831-840
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 841-850
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 851-860
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 861-870
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 871-880
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 881-890
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 891-900
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 901-910
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 911-920
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 921-930
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 931-940
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 941-950
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 951-960
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 961-970
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 971-980
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 981-990
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 991-1000
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1001-1010
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1011-1020
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1021-1030
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1031-1040
        videoGen1stData,  videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1041-1050
        NULL
    },{
        // pal
        videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYNOdd,videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,   // 1-10
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 11-20
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 21-30
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 31-40
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 41-50
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 51-60
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 61-70
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 71-80
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 81-90
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 91-100
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 101-110
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 111-120
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 121-130
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 131-140
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 141-150
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 151-160
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 161-170
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 171-180
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 181-190
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 191-200
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 201-210
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 211-220
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 221-230
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 231-240
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 241-250
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 251-260
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 261-270
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 271-280
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 281-290
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 291-300
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 301-310
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 311-320
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 321-330
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 331-340
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 341-350
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 351-360
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 361-370
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 371-380
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 381-390
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 391-400
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 401-410
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 411-420
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 421-430
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 431-440
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 441-450
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 451-460
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 461-470
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 471-480
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 481-490
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 491-500
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 501-510
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 511-520
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 521-530
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 531-540
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 541-550
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 551-560
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 561-570
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 571-580
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 581-590
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 591-600
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 601-610
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 611-620
        videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYN,   videoGenVSYNEven, // 621-630
        videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 631-640
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 641-650
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 651-660
        videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,    videoGenHSYN,   videoGenBLK,     // 661-670
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 671-680
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 681-690
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 691-700
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 701-710
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 711-720
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 721-730
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 731-740
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 741-750
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 751-760
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 761-770
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 771-780
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 781-790
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 791-800
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 801-810
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 811-820
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 821-830
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 831-840
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 841-850
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 851-860
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 861-870
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 871-880
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 881-890
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 891-900
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 901-910
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 911-920
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 921-930
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 931-940
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 941-950
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 951-960
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 961-970
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 971-980
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 981-990
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 991-1000
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1001-1010
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1011-1020
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1021-1030
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1031-1040
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1041-1050
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1051-1060
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1061-1070
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1071-1080
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1081-1090
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1091-1000
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1101-1110
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1111-1120
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1121-1130
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1131-1140
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1141-1150
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1151-1160
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1161-1170
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1171-1180
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1181-1190
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1191-1200
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1201-1210
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1211-1220
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1221-1230
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData, // 1231-1240
        videoGen1stData,videoGen2ndData,videoGen1stData,videoGen2ndData,videoGen1stData,videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,  videoGenEpPAL,   // 1241-1250
        NULL
    }
};

//__attribute__((section (".sram2")))
uint32_t dataTableVG[256][2] = {
    {VG_BLK | VG_BLK<<16 , VG_BLK | VG_BLK<<16},   // 0x00
    {VG_BLK | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x01
    {VG_BLK | VG_BLK<<16 , VG_BLK | VG_WHI<<16},   // 0x02
    {VG_BLK | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x03
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x04
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x05
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x06
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x07
    {VG_BLK | VG_BLK<<16 , VG_WHI | VG_BLK<<16},   // 0x08
    {VG_BLK | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x09
    {VG_BLK | VG_BLK<<16 , VG_WHI | VG_WHI<<16},   // 0x0a
    {VG_BLK | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x0b
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x0c
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x0d
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x0e
    {VG_BLK | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x0f
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0x10
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x11
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0x12
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x13
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x14
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x15
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x16
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x17
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0x18
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x19
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0x1a
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x1b
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x1c
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x1d
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x1e
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x1f
    {VG_BLK | VG_WHI<<16 , VG_BLK | VG_BLK<<16},   // 0x20
    {VG_BLK | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0x21
    {VG_BLK | VG_WHI<<16 , VG_BLK | VG_WHI<<16},   // 0x22
    {VG_BLK | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0x23
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0x24
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x25
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0x26
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x27
    {VG_BLK | VG_WHI<<16 , VG_WHI | VG_BLK<<16},   // 0x28
    {VG_BLK | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0x29
    {VG_BLK | VG_WHI<<16 , VG_WHI | VG_WHI<<16},   // 0x2a
    {VG_BLK | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0x2b
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0x2c
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x2d
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0x2e
    {VG_BLK | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x2f
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0x30
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x31
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0x32
    {VG_BLK | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x33
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x34
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x35
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x36
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x37
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0x38
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x39
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0x3a
    {VG_BLK | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x3b
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x3c
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x3d
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x3e
    {VG_BLK | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x3f
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_BLK<<16},   // 0x40
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x41
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_WHI<<16},   // 0x42
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x43
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x44
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x45
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x46
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x47
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_BLK<<16},   // 0x48
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x49
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_WHI<<16},   // 0x4a
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x4b
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x4c
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x4d
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x4e
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x4f
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0x50
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x51
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0x52
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x53
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x54
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x55
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x56
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x57
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0x58
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x59
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0x5a
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x5b
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x5c
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x5d
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x5e
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x5f
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_BLK<<16},   // 0x60
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0x61
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_WHI<<16},   // 0x62
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0x63
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0x64
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x65
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0x66
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x67
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_BLK<<16},   // 0x68
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0x69
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_WHI<<16},   // 0x6a
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0x6b
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0x6c
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x6d
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0x6e
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0x6f
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0x70
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x71
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0x72
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x73
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x74
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x75
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x76
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x77
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0x78
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x79
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0x7a
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x7b
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x7c
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x7d
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x7e
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x7f
    {VG_WHI | VG_BLK<<16 , VG_BLK | VG_BLK<<16},   // 0x80
    {VG_WHI | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x81
    {VG_WHI | VG_BLK<<16 , VG_BLK | VG_WHI<<16},   // 0x82
    {VG_WHI | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0x83
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x84
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x85
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x86
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x87
    {VG_WHI | VG_BLK<<16 , VG_WHI | VG_BLK<<16},   // 0x88
    {VG_WHI | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x89
    {VG_WHI | VG_BLK<<16 , VG_WHI | VG_WHI<<16},   // 0x8a
    {VG_WHI | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0x8b
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0x8c
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x8d
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0x8e
    {VG_WHI | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0x8f
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0x90
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x91
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0x92
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0x93
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x94
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x95
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x96
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x97
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0x98
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x99
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0x9a
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0x9b
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0x9c
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x9d
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0x9e
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0x9f
    {VG_WHI | VG_WHI<<16 , VG_BLK | VG_BLK<<16},   // 0xa0
    {VG_WHI | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0xa1
    {VG_WHI | VG_WHI<<16 , VG_BLK | VG_WHI<<16},   // 0xa2
    {VG_WHI | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0xa3
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0xa4
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xa5
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0xa6
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xa7
    {VG_WHI | VG_WHI<<16 , VG_WHI | VG_BLK<<16},   // 0xa8
    {VG_WHI | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0xa9
    {VG_WHI | VG_WHI<<16 , VG_WHI | VG_WHI<<16},   // 0xaa
    {VG_WHI | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0xab
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0xac
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xad
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0xae
    {VG_WHI | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xaf
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0xb0
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xb1
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0xb2
    {VG_WHI | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xb3
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xb4
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xb5
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xb6
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xb7
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0xb8
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xb9
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0xba
    {VG_WHI | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xbb
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xbc
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xbd
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xbe
    {VG_WHI | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xbf
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_BLK<<16},   // 0xc0
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0xc1
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_WHI<<16},   // 0xc2
    {VG_GRY | VG_BLK<<16 , VG_BLK | VG_GRY<<16},   // 0xc3
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0xc4
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0xc5
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0xc6
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0xc7
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_BLK<<16},   // 0xc8
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0xc9
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_WHI<<16},   // 0xca
    {VG_GRY | VG_BLK<<16 , VG_WHI | VG_GRY<<16},   // 0xcb
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_BLK<<16},   // 0xcc
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0xcd
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_WHI<<16},   // 0xce
    {VG_GRY | VG_BLK<<16 , VG_GRY | VG_GRY<<16},   // 0xcf
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0xd0
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xd1
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0xd2
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xd3
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xd4
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xd5
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xd6
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xd7
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0xd8
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xd9
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0xda
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xdb
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xdc
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xdd
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xde
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xdf
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_BLK<<16},   // 0xe0
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0xe1
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_WHI<<16},   // 0xe2
    {VG_GRY | VG_WHI<<16 , VG_BLK | VG_GRY<<16},   // 0xe3
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0xe4
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xe5
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0xe6
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xe7
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_BLK<<16},   // 0xe8
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0xe9
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_WHI<<16},   // 0xea
    {VG_GRY | VG_WHI<<16 , VG_WHI | VG_GRY<<16},   // 0xeb
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_BLK<<16},   // 0xec
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xed
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_WHI<<16},   // 0xee
    {VG_GRY | VG_WHI<<16 , VG_GRY | VG_GRY<<16},   // 0xef
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_BLK<<16},   // 0xf0
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xf1
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_WHI<<16},   // 0xf2
    {VG_GRY | VG_GRY<<16 , VG_BLK | VG_GRY<<16},   // 0xf3
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xf4
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xf5
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xf6
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xf7
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_BLK<<16},   // 0xf8
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xf9
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_WHI<<16},   // 0xfa
    {VG_GRY | VG_GRY<<16 , VG_WHI | VG_GRY<<16},   // 0xfb
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_BLK<<16},   // 0xfc
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16},   // 0xfd
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_WHI<<16},   // 0xfe
    {VG_GRY | VG_GRY<<16 , VG_GRY | VG_GRY<<16}    // 0xff
};

uint8_t video_gen_format = 0;    // 0:ntsc 1:pal
uint16_t video_gen_line = 0;    // VIDEO_GEN_LINE_X
uint16_t hpos2nd = 0;
uint32_t vg_vcanvas_count = 0;
uint16_t videoGenCount = 0;

__attribute__((section (".sram2")))
uint16_t videoBuff[VIDEO_GEN_LINE_PAL] __attribute__((aligned(4)));

//void memset16(register volatile uint16_t *dst, register volatile uint16_t data, register volatile uint16_t len16)
//{
//    while(len16--){
//        *dst++ = data;
//    }
//}
__attribute__((section (".ccmram_code")))
void memset16(volatile uint16_t *dst, uint16_t data, uint16_t len16)
{
    __asm__ volatile (
        "1:\n"
        "strh %[data], [%[dst]], #2\n"   // *dst = data; dst += 2;
        "subs %[len], %[len], #1\n"      // len--
        "bne 1b\n"                        // if (len != 0) goto 1;
        : [dst] "+r" (dst), [len] "+r" (len16)
        : [data] "r" (data)
        : "memory"
    );
}

__attribute__((section (".ccmram_code"), optimize("O3")))
void setVideoGenLine(register volatile uint16_t *ldata, uint16_t *buf, int line, register volatile uint16_t hpos, register volatile uint16_t len)
{
    register volatile  uint8_t* fontp;

//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
    int linex = line*3;

    while( len > 0 ){
        fontp = (uint8_t*)&font[buf[hpos/12]][linex];
        if ((len >= 12) && ((hpos % 12) == 0)){
            register volatile uint32_t *ldata32 = (uint32_t*)ldata;
            register volatile uint32_t *fp;
            fp = (void*)dataTableVG[ *fontp++ ];
            *ldata32++ = *fp++;
            *ldata32++ = *fp++;
            fp = (void*)dataTableVG[ *fontp++ ];
            *ldata32++ = *fp++;
            *ldata32++ = *fp++;
            fp = (void*)dataTableVG[ *fontp ];
            *ldata32++ = *fp++;
            *ldata32++ = *fp++;
            hpos += 12;
            len -= 12;
            ldata += 12;
        }else{
            while( len > 0 ){
                *(uint32_t*)ldata = dataTableVG[ *(uint8_t*)(fontp + ((hpos>>2) % 3) ) ][(hpos>>1) & 0x1];
                hpos +=2;
                len -= 2;
                ldata +=2;
                if ((hpos % 12) == 0){
                    break;
                }
            }
        }
    }
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
    return;
}

// equalization pulse
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenEpNTSC(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);

    uint16_t cnt = 0;
    memset16(&buf[cnt], VG_SYN, VIDEO_TIM_NS(2300) );
    cnt += VIDEO_TIM_NS(2300);
    memset16(&buf[cnt], VG_BLK, size - cnt);
}

__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenEpPAL(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);

    uint16_t cnt = 0;
    memset16(&buf[cnt], VG_SYN, VIDEO_TIM_NS(2350) );
    cnt += VIDEO_TIM_NS(2350);
    memset16(&buf[cnt], VG_BLK, size - cnt);
}

// blank
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenBLK(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);
    memset16(buf, VG_BLK, size);
}

// serration pulse
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenVSYN(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);
    uint16_t cnt = 0;
    memset16(&buf[cnt], VG_SYN, size - VIDEO_TIM_NS(4700) );
    cnt += size - VIDEO_TIM_NS(4700);
    memset16(&buf[cnt], VG_BLK, size - cnt);
}
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenVSYNEven(uint16_t *buf, uint16_t size, uint16_t line)
{
    vg_vcanvas_count = 0;
    videoGenVSYN(buf, size, line);
}
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenVSYNOdd(uint16_t *buf, uint16_t size, uint16_t line)
{
    vg_vcanvas_count = 1;
    videoGenVSYN(buf, size, line);
}

// hsync
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGenHSYN(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);
    uint16_t cnt = 0;
    memset16(&buf[cnt], VG_SYN, VIDEO_TIM_NS(4700) );
    cnt += VIDEO_TIM_NS(4700);
    memset16(&buf[cnt], VG_BLK, size - cnt);
}

// hsync + 1st data
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGen1stData(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);
    uint16_t cnt = 0;
    memset16(&buf[cnt], VG_SYN, VIDEO_TIM_NS(4700) );
    cnt += VIDEO_TIM_NS(4700);
    memset16(&buf[cnt], VG_BLK, VIDEO_TIM_NS(4700) );
    cnt += VIDEO_TIM_NS(4700);
    uint16_t offset = ( (cnt & 0x1) + (CANVAS_H_OFFSET_GEN>>1) ) & 0xfffe; // 32bit aligned for setVideoGenLine(). uint16_t *buf    @@TODO: adjust offset NTSC/PAL
    memset16(&buf[cnt], VG_GRY, offset);
    cnt += offset;

    hpos2nd = size - cnt;

    int32_t canvas_line = (int32_t)vg_vcanvas_count - canvas_v_offset_gen[setting()->videoFormat];
    if ( canvas_line >= 0 && canvas_line < canvas_v[setting()->videoFormat] ){
#ifdef RESOLUTION_HD
            setVideoGenLine(&buf[cnt], charCanvasGet(canvas_line/18), canvas_line % 18, 0, hpos2nd);
#else
            UNUSED(line);
            setVideoGenLine(&buf[cnt], charCanvasGet((canvas_line>>1)/18), (canvas_line>>1) % 18, 0, hpos2nd);
#endif
    }else{
        memset16(&buf[cnt], VG_GRY, hpos2nd);
    }
}

// 2nd data + blank
__attribute__((section (".ccmram_code"), optimize("O2")))
void videoGen2ndData(uint16_t *buf, uint16_t size, uint16_t line)
{
    UNUSED(line);
    uint16_t cnt = 0;
    int32_t canvas_line = (int32_t)vg_vcanvas_count - canvas_v_offset_gen[setting()->videoFormat];
    if ( canvas_line >= 0 && canvas_line < canvas_v[setting()->videoFormat] ){
#ifdef RESOLUTION_HD
        setVideoGenLine(&buf[cnt], charCanvasGet(canvas_line/18), canvas_line % 18, hpos2nd, CANVAS_H_R - hpos2nd);
#else
        (void)line;
        setVideoGenLine(&buf[cnt], charCanvasGet((canvas_line>>1)/18), (canvas_line>>1) % 18, hpos2nd, CANVAS_H_R - hpos2nd);
#endif
        cnt += CANVAS_H_R - hpos2nd;
        memset16(&buf[cnt], VG_GRY, size - cnt);
    }else{
        memset16(&buf[cnt], VG_GRY, size - cnt);
    }
    vg_vcanvas_count += 2;

}
// video gen dac interrupt comp
__attribute__((section (".ccmram_code"), optimize("O2")))
void dmaComp2cp(DMA_HandleTypeDef *_hdma)
{
    UNUSED(_hdma);

//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);

    (videoGenFuncTable[setting()->videoFormat][videoGenCount])(&videoBuff[video_gen_line/2], video_gen_line/2, vg_vcanvas_count);
    videoGenCount++;
    if (videoGenFuncTable[setting()->videoFormat][videoGenCount] == NULL){
        videoGenCount = 0;
        charCanvasNext();
    }

//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);
}

// video gen dac interrupt half
__attribute__((section (".ccmram_code"), optimize("O2")))
void dmaComp2ht(DMA_HandleTypeDef *_hdma)
{
    UNUSED(_hdma);
//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_SET);
    (videoGenFuncTable[setting()->videoFormat][videoGenCount])(&videoBuff[0], video_gen_line/2, vg_vcanvas_count);
    videoGenCount++;
    if (videoGenFuncTable[setting()->videoFormat][videoGenCount] == NULL){
        videoGenCount = 0;
    }
//    HAL_GPIO_WritePin(DEBUG_GPIO_Port, DEBUG_Pin, GPIO_PIN_RESET);
}


void videosignal_gen_start(void)
{
    video_gen_line = (setting()->videoFormat == VIDEO_PAL) ?  VIDEO_GEN_LINE_PAL : VIDEO_GEN_LINE_NTSC;;

    videoGenBLK(&videoBuff[0], video_gen_line/2, 0);
    videoGenBLK(&videoBuff[video_gen_line/2],video_gen_line/2, 0);
    videoGenCount = 0;

    HAL_TIM_Base_MspInit(&htim4);
    LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH2);

    OPAMP1->CSR = BLK;
    LL_DAC_Disable(DAC3, LL_DAC_CHANNEL_1);

    HAL_DMA_RegisterCallback(&hdma_dac3_ch1, HAL_DMA_XFER_HALFCPLT_CB_ID, dmaComp2ht);
    HAL_DMA_RegisterCallback(&hdma_dac3_ch1, HAL_DMA_XFER_CPLT_CB_ID, dmaComp2cp);
    LL_DMA_ConfigTransfer(DMA2,
                                          LL_DMA_CHANNEL_1,
                                          LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_PRIORITY_HIGH | LL_DMA_MODE_CIRCULAR |
                                          LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
                                          LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_HALFWORD);
    LL_DMA_ConfigAddresses(DMA2,
                                           LL_DMA_CHANNEL_1,
                                           (uint32_t)&videoBuff[0], (uint32_t)&(DAC3->DHR12R1),
                                           LL_DMA_GetDataTransferDirection(DMA2, LL_DMA_CHANNEL_1));
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, video_gen_line); 

    LL_DMA_EnableIT_HT(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DAC_EnableDMAReq(DAC3, LL_DAC_CHANNEL_1);
    LL_DAC_EnableTrigger(DAC3, LL_DAC_CHANNEL_1);
    LL_DAC_SetHighFrequencyMode(DAC3, LL_DAC_HIGH_FREQ_MODE_ABOVE_160MHZ);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);

    LL_DAC_SetTriggerSource(DAC3, LL_DAC_CHANNEL_1, LL_DAC_TRIG_EXT_TIM4_TRGO);
    LL_DAC_Enable(DAC3, LL_DAC_CHANNEL_1);
    LL_TIM_EnableCounter(TIM4);
}

void videosignal_gen_stop(void)
{
    LL_TIM_DisableCounter(TIM4);
    LL_DAC_SetTriggerSource(DAC3, LL_DAC_CHANNEL_1, LL_DAC_TRIG_SOFTWARE);

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
    LL_DAC_DisableTrigger(DAC3, LL_DAC_CHANNEL_1);
    LL_DAC_DisableDMAReq(DAC3, LL_DAC_CHANNEL_1);
    LL_DMA_DisableIT_TC(DMA2, LL_DMA_CHANNEL_1);
    LL_DMA_DisableIT_HT(DMA2, LL_DMA_CHANNEL_1);
}

