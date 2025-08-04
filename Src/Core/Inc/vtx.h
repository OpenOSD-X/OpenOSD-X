#ifndef __VTX_H
#define __VTX_H

void setVtx_vpd(uint16_t freq, uint16_t vpd);
void setVtx(uint16_t freq, uint8_t dB);
void initVtx(void);
void procVtx(void);
void debuglogVtx(void);
uint16_t getVtxFreq(void);
uint16_t getVpdTarget(void);
uint16_t getVpd(void);
uint16_t getVref(void);

#endif
