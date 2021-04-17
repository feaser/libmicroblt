/*------------------------------------------------------------------------/
/  MMCv3/SDv1/SDv2 (in SPI mode) control module
/-------------------------------------------------------------------------/
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/


/*
 * This file was modified from a sample available from the FatFs
 * web site. It was modified to work with a Olimex STM32-P405
 * evaluation board.
 *
 */
#include "diskio.h"
#include "stm32f4xx.h"                           /* STM32 registers and drivers        */
#include "stm32f4xx_ll_bus.h"                    /* STM32 LL BUS header                */
#include "stm32f4xx_ll_gpio.h"                   /* STM32 LL GPIO header               */
#include "stm32f4xx_ll_spi.h"                    /* STM32 LL SPI header                */


/*--------------------------------------------------------------------------

   Macro Definitions

---------------------------------------------------------------------------*/
/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC    0x01    /* MMC ver 3 */
#define CT_SD1    0x02    /* SD ver 1 */
#define CT_SD2    0x04    /* SD ver 2 */
#define CT_SDC    (CT_SD1|CT_SD2) /* SD */
#define CT_BLOCK  0x08    /* Block addressing */


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0   (0)      /* GO_IDLE_STATE */
#define CMD1   (1)      /* SEND_OP_COND */
#define ACMD41 (41|0x80)  /* SEND_OP_COND (SDC) */
#define CMD8   (8)      /* SEND_IF_COND */
#define CMD9   (9)      /* SEND_CSD */
#define CMD10  (10)     /* SEND_CID */
#define CMD12  (12)     /* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)  /* SD_STATUS (SDC) */
#define CMD16  (16)     /* SET_BLOCKLEN */
#define CMD17  (17)     /* READ_SINGLE_BLOCK */
#define CMD18  (18)     /* READ_MULTIPLE_BLOCK */
#define CMD23  (23)     /* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)  /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)     /* WRITE_BLOCK */
#define CMD25  (25)     /* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)     /* SEND_OP_COND (ACMD) */
#define CMD55  (55)     /* APP_CMD */
#define CMD58  (58)     /* READ_OCR */


/* Control signals (Platform dependent) */
#define CS_LOW()    LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12)  /* MMC CS = L */
#define CS_HIGH()   LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12)    /* MMC CS = H */


#define FCLK_SLOW()     /* Set slow clock (100k-400k) */
#define FCLK_FAST() set_max_speed() /* Set fast clock (depends on the CSD) */

static volatile
DSTATUS Stat = STA_NOINIT;  /* Disk status */

static
UINT CardType;


/*-----------------------------------------------------------------------*/
/* Send 80 or so clock transitions with CS and DI held high. This is     */
/* required after card power up to get it into SPI mode                  */
/*-----------------------------------------------------------------------*/
static
void send_initial_clock_train(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct;
  unsigned int i;
  DWORD timeoutTime;

  /* Ensure CS is held high. */
  CS_HIGH();

  /* Switch the SSI TX line to a GPIO and drive it high too. */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_15);

  /* Send 10 bytes over the SSI. This causes the clock to wiggle the */
  /* required number of times. */
  for(i = 0 ; i < 10 ; i++)
  {
    /* Set timeout time to wait for DR register empty */
    timeoutTime = HAL_GetTick() + 100;
    /* Loop while DR register in not empty */
    while (LL_SPI_IsActiveFlag_TXE(SPI2) == 0)
    {
      /* Break wait loop upon timeout */
      if (HAL_GetTick() > timeoutTime)
      {
        break;
      }
    }

    /* Send byte through the SPI peripheral */
    LL_SPI_TransmitData8(SPI2, 0xff);

    /* Set timeout time to wait for byte reception */
    timeoutTime = HAL_GetTick() + 100;
    /* Wait to receive a byte */
    while (LL_SPI_IsActiveFlag_RXNE(SPI2) == 0)
    {
      /* Break wait loop upon timeout */
      if (HAL_GetTick() > timeoutTime)
      {
        break;
      }
    }
  }

  /* Revert to hardware control of the SSI TX line. */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions.                                  */
static
void power_on (void)
{
  LL_SPI_InitTypeDef  SPI_InitStruct;
  LL_GPIO_InitTypeDef GPIO_InitStruct;

  /*
   * This doesn't really turn the power on, but initializes the
   * SSI port and pins needed to talk to the card.
   */
  /* Enable SPI and GPIO peripheral clocks. */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

  /* Configure I/O for Chip select (PB12) */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* De-select the Card: Chip Select high */
  LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);

  /* Configure SPI pins: SCK (PB13), MOSI (PB15) and MISO (PB14) */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_13|LL_GPIO_PIN_14|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* SPI2 parameter configuration */
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV128; // 168MHz/4/128=328kHz < 400kHz
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI2, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_Enable(SPI2);

  /* Set DI and CS high and apply more than 74 pulses to SCLK for the card */
  /* to be able to accept a native command. */
  send_initial_clock_train();
}

// set the SSI speed to the max setting
static
void set_max_speed(void)
{
  LL_SPI_InitTypeDef  SPI_InitStruct;

  /* Disable the SPI system */
  LL_SPI_Disable(SPI2);

  /* MMC/SDC can work at the clock frequency up to 20/25MHz so pick a speed close to
   * this but not higher
   */
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV4; // 168MHz/4/4=10.5MHz < 25MHz
  SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
  SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
  SPI_InitStruct.CRCPoly = 7;
  LL_SPI_Init(SPI2, &SPI_InitStruct);
  LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_Enable(SPI2);
}

static
void power_off (void)
{
  Stat |= STA_NOINIT; /* Force uninitialized */
}


/*-----------------------------------------------------------------------*/
/* Transmit/Receive data to/from MMC via SPI  (Platform dependent)       */
/*-----------------------------------------------------------------------*/

static
BYTE xchg_spi (BYTE dat)
{
  BYTE result = 0;
  DWORD timeOutTime;
  BYTE timeoutDetected = 0;

  /* Send byte through the SPI peripheral */
  LL_SPI_TransmitData8(SPI2, dat);

  /* Set timeout for 50 ms from now */
  timeOutTime = HAL_GetTick() + 50;

  /* Wait to receive a byte with timeout */
  while (LL_SPI_IsActiveFlag_RXNE(SPI2) == 0)
  {
    /* Check for timeout */
    if (HAL_GetTick() > timeOutTime)
    {
      /* Set flag to remember that a timeout occurred and nothing was received */
      timeoutDetected = 1;
      /* Stop waiting */
      break;
    }
  }

  /* Read the value of the received byte */
  if (timeoutDetected == 0)
  {
    result = LL_SPI_ReceiveData8(SPI2);
  }

  /* Give the result back to the caller */
  return result;
}

static
void rcvr_spi_m (BYTE *dst)
{
    *dst = xchg_spi(0xFF);
}


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

static
int wait_ready (void)
{
  BYTE d;
  DWORD timeOutTime;

  /* set timeout for 500 ms from now */
  timeOutTime = HAL_GetTick() + 500;

  do {
    d = xchg_spi(0xFF);
  } while ((d != 0xFF) && (HAL_GetTick() < timeOutTime));

  return (d == 0xFF) ? 1 : 0;
}


/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void deselect_card (void)
{
  CS_HIGH();
  xchg_spi(0xFF);   /* Dummy clock (force DO hi-z for multiple slave SPI) */
}



/*-----------------------------------------------------------------------*/
/* Select the card and wait ready                                        */
/*-----------------------------------------------------------------------*/

static
int select_card (void)  /* 1:Successful, 0:Timeout */
{
  CS_LOW();
  xchg_spi(0xFF);   /* Dummy clock (force DO enabled) */

  if (wait_ready()) return 1; /* OK */
  deselect_card();
  return 0; /* Timeout */
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (  /* 1:OK, 0:Failed */
  BYTE *buff,     /* Data buffer to store received data */
  UINT btr      /* Byte count (must be multiple of 4) */
)
{
  BYTE token;
  DWORD timeOutTime;

  /* set timeout for 100 ms from now */
  timeOutTime = HAL_GetTick() + 100;

  do {              /* Wait for data packet in timeout of 100ms */
    token = xchg_spi(0xFF);

  } while ((token == 0xFF) && (HAL_GetTick() < timeOutTime));

  if(token != 0xFE) return 0;   /* If not valid data token, retutn with error */

    do {                            /* Receive the data block into buffer */
        rcvr_spi_m(buff++);
        rcvr_spi_m(buff++);
    } while (btr -= 2);
  xchg_spi(0xFF);         /* Discard CRC */
  xchg_spi(0xFF);

  return 1;           /* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

static
int xmit_datablock (  /* 1:OK, 0:Failed */
  const BYTE *buff, /* 512 byte data block to be transmitted */
  BYTE token      /* Data token */
)
{
    BYTE resp;
    UINT wc;


  if (!wait_ready()) return 0;

  xchg_spi(token);    /* Xmit a token */
  if (token != 0xFD) {  /* Not StopTran token */
        wc = 512;
        do {                            /* Xmit the 512 byte data block to MMC */
            xchg_spi(*buff++);
            xchg_spi(*buff++);
        } while (wc -= 2);
    xchg_spi(0xFF);       /* CRC (Dummy) */
    xchg_spi(0xFF);
    resp = xchg_spi(0xFF);    /* Receive a data response */
    if ((resp & 0x1F) != 0x05)  /* If not accepted, return with error */
      return 0;
  }

  return 1;
}



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
  BYTE cmd,   /* Command byte */
  DWORD arg   /* Argument */
)
{
  BYTE n, res;


  if (cmd & 0x80) { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;
    res = send_cmd(CMD55, 0);
    if (res > 1) return res;
  }

  /* Select the card and wait for ready */
  deselect_card();
  if (!select_card()) return 0xFF;

  /* Send command packet */
  xchg_spi(0x40 | cmd);     /* Start + Command index */
  xchg_spi((BYTE)(arg >> 24));  /* Argument[31..24] */
  xchg_spi((BYTE)(arg >> 16));  /* Argument[23..16] */
  xchg_spi((BYTE)(arg >> 8));   /* Argument[15..8] */
  xchg_spi((BYTE)arg);      /* Argument[7..0] */
  n = 0x01;           /* Dummy CRC + Stop */
  if (cmd == CMD0) n = 0x95;    /* Valid CRC for CMD0(0) + Stop */
  if (cmd == CMD8) n = 0x87;    /* Valid CRC for CMD8(0x1AA) + Stop */
  xchg_spi(n);

  /* Receive command response */
  if (cmd == CMD12) xchg_spi(0xFF); /* Skip a stuff byte on stop to read */
  n = 10;             /* Wait for a valid response in timeout of 10 attempts */
  do {
    res = xchg_spi(0xFF);
  } while ((res & 0x80) && --n);

  return res;     /* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
  BYTE pdrv   /* Physical drive nmuber (0) */
)
{
  BYTE n, cmd, ty, ocr[4];
  DWORD timeOutTime;


  if (pdrv) return STA_NOINIT;    /* Supports only single drive */
  if (Stat & STA_NODISK) return Stat; /* No card in the socket */

    power_on();                            /* Force socket power on */

     CS_LOW();                /* CS = L */

  ty = 0;
  if (send_cmd(CMD0, 0) == 1) {     /* Enter Idle state */
    timeOutTime = HAL_GetTick() + 1000; /* Initialization timeout of 1000 msec */

    if (send_cmd(CMD8, 0x1AA) == 1) { /* SDv2? */
      for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);      /* Get trailing return value of R7 resp */
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {       /* The card can work at vdd range of 2.7-3.6V */
        while ((HAL_GetTick() < timeOutTime) && send_cmd(ACMD41, 0x40000000)); /* Wait for leaving idle state (ACMD41 with HCS bit) */
        if ((HAL_GetTick() < timeOutTime) && send_cmd(CMD58, 0) == 0) {      /* Check CCS bit in the OCR */
          for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
          ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;  /* SDv2 */
        }
      }
    } else {              /* SDv1 or MMCv3 */
      if (send_cmd(ACMD41, 0) <= 1)   {
        ty = CT_SD1; cmd = ACMD41;  /* SDv1 */
      } else {
        ty = CT_MMC; cmd = CMD1;  /* MMCv3 */
      }
      while ((HAL_GetTick() < timeOutTime) && send_cmd(cmd, 0)) {  /* Wait for leaving idle state */
        ;
      }
      if (!(HAL_GetTick() < timeOutTime) || send_cmd(CMD16, 512) != 0) /* Set read/write block length to 512 */
        ty = 0;
    }
  }
  CardType = ty;
  deselect_card();

  if (ty) {     /* Initialization succeded */
    Stat &= ~STA_NOINIT;  /* Clear STA_NOINIT */
    FCLK_FAST();
  } else {      /* Initialization failed */
    power_off();
  }

  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
  BYTE pdrv   /* Physical drive nmuber (0) */
)
{
  if (pdrv) return STA_NOINIT;  /* Supports only single drive */
  return Stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
  BYTE pdrv,    /* Physical drive nmuber to identify the drive */
  BYTE *buff,   /* Data buffer to store read data */
  DWORD sector, /* Sector address in LBA */
  UINT count    /* Number of sectors to read */
)
{
  if (pdrv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert to byte address if needed */

  if (count == 1) {   /* Single block read */
    if ((send_cmd(CMD17, sector) == 0)  /* READ_SINGLE_BLOCK */
      && rcvr_datablock(buff, 512))
      count = 0;
  }
  else {        /* Multiple block read */
    if (send_cmd(CMD18, sector) == 0) { /* READ_MULTIPLE_BLOCK */
      do {
        if (!rcvr_datablock(buff, 512)) break;
        buff += 512;
      } while (--count);
      send_cmd(CMD12, 0);       /* STOP_TRANSMISSION */
    }
  }
  deselect_card();

  return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
  BYTE pdrv,      /* Physical drive nmuber to identify the drive */
  const BYTE *buff, /* Data to be written */
  DWORD sector,   /* Sector address in LBA */
  UINT count      /* Number of sectors to write */
)
{
  if (pdrv || !count) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;
  if (Stat & STA_PROTECT) return RES_WRPRT;

  if (!(CardType & CT_BLOCK)) sector *= 512;  /* Convert to byte address if needed */

  if (count == 1) {   /* Single block write */
    if ((send_cmd(CMD24, sector) == 0)  /* WRITE_BLOCK */
      && xmit_datablock(buff, 0xFE))
      count = 0;
  }
  else {        /* Multiple block write */
    if (CardType & CT_SDC) send_cmd(ACMD23, count);
    if (send_cmd(CMD25, sector) == 0) { /* WRITE_MULTIPLE_BLOCK */
      do {
        if (!xmit_datablock(buff, 0xFC)) break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(0, 0xFD)) /* STOP_TRAN token */
        count = 1;
    }
  }
  deselect_card();

  return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE pdrv,    /* Physical drive nmuber (0) */
  BYTE cmd,   /* Control code */
  void *buff    /* Buffer to send/receive data block */
)
{
  DRESULT res;
  BYTE n, csd[16], *ptr = buff;
  DWORD csz;


  if (pdrv) return RES_PARERR;
  if (Stat & STA_NOINIT) return RES_NOTRDY;

  res = RES_ERROR;
  switch (cmd) {
  case CTRL_SYNC :  /* Flush write-back cache, Wait for end of internal process */
    if (select_card()) res = RES_OK;
    break;

  case GET_SECTOR_COUNT : /* Get number of sectors on the disk (WORD) */
    if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
      if ((csd[0] >> 6) == 1) { /* SDv2? */
        csz = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
        *(DWORD*)buff = csz << 10;
      } else {          /* SDv1 or MMCv3 */
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csz = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
        *(DWORD*)buff = csz << (n - 9);
      }
      res = RES_OK;
    }
    break;

  case GET_BLOCK_SIZE : /* Get erase block size in unit of sectors (DWORD) */
    if (CardType & CT_SD2) {  /* SDv2? */
      if (send_cmd(ACMD13, 0) == 0) {   /* Read SD status */
        xchg_spi(0xFF);
        if (rcvr_datablock(csd, 16)) {        /* Read partial block */
          for (n = 64 - 16; n; n--) xchg_spi(0xFF); /* Purge trailing data */
          *(DWORD*)buff = 16UL << (csd[10] >> 4);
          res = RES_OK;
        }
      }
    } else {          /* SDv1 or MMCv3 */
      if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {  /* Read CSD */
        if (CardType & CT_SD1) {  /* SDv1 */
          *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
        } else {          /* MMCv3 */
          *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
        }
        res = RES_OK;
      }
    }
    break;

  case MMC_GET_TYPE :   /* Get card type flags (1 byte) */
    *ptr = CardType;
    res = RES_OK;
    break;

  case MMC_GET_CSD :  /* Receive CSD as a data block (16 bytes) */
    if ((send_cmd(CMD9, 0) == 0)  /* READ_CSD */
      && rcvr_datablock(buff, 16))
      res = RES_OK;
    break;

  case MMC_GET_CID :  /* Receive CID as a data block (16 bytes) */
    if ((send_cmd(CMD10, 0) == 0) /* READ_CID */
      && rcvr_datablock(buff, 16))
      res = RES_OK;
    break;

  case MMC_GET_OCR :  /* Receive OCR as an R3 resp (4 bytes) */
    if (send_cmd(CMD58, 0) == 0) {  /* READ_OCR */
      for (n = 0; n < 4; n++) {
        *((BYTE*)buff+n) = xchg_spi(0xFF);
      }
      res = RES_OK;
    }
    break;

  case MMC_GET_SDSTAT : /* Receive SD status as a data block (64 bytes) */
    if ((CardType & CT_SD2) && send_cmd(ACMD13, 0) == 0) {  /* SD_STATUS */
      xchg_spi(0xFF);
      if (rcvr_datablock(buff, 64))
        res = RES_OK;
    }
    break;

  default:
    res = RES_PARERR;
  }

  deselect_card();

  return res;
}

