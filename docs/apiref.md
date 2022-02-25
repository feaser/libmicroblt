# API reference

This section provides a full reference of all the functions, macros and types that LibMicroBLT offers.

## Macros

| Macro                             | Description |
| :-------------------------------- | :---------- |
| `BLT_VERSION_MAIN`             | Main version number of LibMicroBLT. |
| `BLT_VERSION_MINOR`            | Minor version number of LibMicroBLT. |
| `BLT_VERSION_PATCH`            | Patch number of LibMicroBLT. |
| `BLT_SESSION_XCP_V10`     | Session type identifier for XCP version 1.0. |
| `BLT_FIRMWARE_READER_SRECORD` | Firmware type identifier for S-record firmware files. |

## Types

### tBltSessionSettingsXcpV10

```c
typedef struct tBltSessionSettingsXcpV10
```

Structure layout of the XCP version 1.0 session settings.

| Element       | Description                                       |
| ------------- | ------------------------------------------------- |
| `timeoutT1`   | Command response timeout in milliseconds.         |
| `timeoutT3`   | Start programming timeout in milliseconds.        |
| `timeoutT4`   | Erase memory timeout in milliseconds.             |
| `timeoutT5`   | Program memory and reset timeout in milliseconds. |
| `timeoutT6`   | Connect response timeout in milliseconds.         |
| `timeoutT7`   | Busy wait timer timeout in milliseonds.           |
| `connectMode` | Connection mode parameter in XCP connect command. |

### tPortXcpPacket

```c
typedef struct tPortXcpPacket
```

XCP packet type.

| Element | Description                                |
| ------- | ------------------------------------------ |
| `data`  | Byte array with packet data.               |
| `len`   | Number of data bytes stored in the packet. |

### tPort

```c
typedef struct tPort
```

Port interface. The port interface links the hardware specific implementation to the hardware independent library.

| Element                 | Description                                                  |
| ----------------------- | ------------------------------------------------------------ |
| `SystemGetTime`         | Function pointer to obtain the current system time in milliseconds. |
| `XcpTransmitPacket`     | Function pointer to transmit an XCP packet using the transport layer<br>implemented by the port.  The transmission itself can be blocking.<br>The function should return `TBX_OK`  if the packet could be transmitted,<br>`TBX_ERROR` otherwise. |
| `XcpReceivePacket`      | Function pointer to receive an XCP packet using the transport layer<br>implemented by  the port. The reception should be non-blocking. The<br>function should return `TBX_TRUE` if a packet was received, `TBX_FALSE`<br>otherwise. A newly received packet should be stored in the rxPacket<br>parameter. |
| `XcpComputeKeyFromSeed` | Function pointer to calculates the key to unlock the programming<br>resource, based on the given seed. This function should return `TBX_OK`<br>if the key could be calculated, `TBX_ERROR` otherwise. Note that it's okay<br>to set this element to `NULL`, if you do not use the [seed/key security<br>feature](https://www.feaser.com/openblt/doku.php?id=manual:security) of the OpenBLT bootloader. |

## Functions

### Port module

The port module makes it possible to connect your hardware specifics to the hardware independent LibMicroBLT library. In your own application you implement the port specific functions. For example for transmitting and receiving XCP communication packet on the transport layer of your choice. You then specify your port specific functions in a variable of type [tPort](#tport). You pass this variable on as a parameter when calling function [BltPortInit()](#bltportinit), which links your hardware specifics to LibMicroBLT. For this reason, you should first call function [BltPortInit()](#bltportinit), before calling any other functions of LibMicroBLT.

#### BltPortInit

```c
void BltPortInit(tPort const * port)
```

Initializes the port module by linking the port specified by the parameter. Typically called once during firmware initialization. Must be called before calling any other functions of LibMicroBLT.

| Parameter | Description                                         |
| --------- | --------------------------------------------------- |
| `port`    | Pointer to the port to link (type [tPort](#tport)). |

**Example**

This example is taken from the LibMicroBLT demo application, where the application implements the hardware specific functions in:

* `AppPortSystemGetTime()`
* `AppPortXcpTransmitPacket()`
* `AppPortXcpReceivePacket()`
* `AppPortXcpComputeKeyFromSeed()`

Refer to the LibMicroBLT demo application for an example on how to implement or port these functions for your own hardware. Once these port functions are implemented, you link them to LibMicroBLT like this:

```c
tPort const portInterface =
{
  .SystemGetTime = AppPortSystemGetTime,
  .XcpTransmitPacket = AppPortXcpTransmitPacket,
  .XcpReceivePacket = AppPortXcpReceivePacket,
  .XcpComputeKeyFromSeed = AppPortXcpComputeKeyFromSeed
};

BltPortInit(&portInterface);
```

#### BltPortTerminate

```c
void BltPortTerminate(void)
```

Terminates the port module. Typically called once during firmware exit. Embedded firmware tends to be driven by an infinite program loop and never exits. In such cases, you can omit calling this function.

### Session module

The session module implements all the functionality for communicating with the OpenBLT bootloader running the other microcontroller(s); The ones on which you want to perform a firmware update from the microcontroller that runs this LibMicroBLT library. The functionality of the session module encompasses:

* Connect and disconnect.
* Erase, write and read memory.

#### BltSessionInit

```c
void BltSessionInit(uint32_t type, void const * settings)
```

Initializes the firmware update session for a specific communication  protocol. This function is typically called at the start of the firmware update.

| Parameter  | Description                                                  |
| ---------- | ------------------------------------------------------------ |
| `type`     | The communication protocol to use for this session. It should be a [`BLT_SESSION_xxx`](#macros)<br>value. |
| `settings` | Pointer to a structure with communication protocol specific settings. |

**Example**

Initialize the session module for communication using the XCP version 1.0 protocol:

```c
tBltSessionSettingsXcpV10 const    sessionSettings =
{
  .timeoutT1   = 1000U,
  .timeoutT3   = 2000U,
  .timeoutT4   = 10000U,
  .timeoutT5   = 1000U,
  .timeoutT6   = 50U,
  .timeoutT7   = 2000U,
  .connectMode = 0
  };

BltSessionInit(BLT_SESSION_XCP_V10, &sessionSettings);
```

#### BltSessionTerminate

```c
void BltSessionTerminate(void)
```

Terminates the firmware update session. This function is typically called at the end of the firmware update.

#### BltSessionStart

```c
uint8_t BltSessionStart(void)
```

Starts the firmware update session. This is were the library attempts to activate and connect with the bootloader running on the target, through the transport layer that was specified during the session's initialization.

| Return value                                   |
| ---------------------------------------------- |
| `TBX_OK` if successful, `TBX_ERROR` otherwise. |

**Example**

When attempting to connect to the OpenBLT bootloader on the target microcontroller, the bootloader itself is often not yet active. Instead, the user's firmware is happily running. The firmware typically includes functionality to receive the connection request, after which it activates the bootloader by performing a software reset. If this functionality is not present, the user needs to manually reset the microcontroller. 

It's therefore recommended to try connecting to the OpenBLT bootloader, by calling `BltSessionStart()` repeatedly for a finite amount of time. Note that the following example also demonstrates how your own application can access the port specific functions:

```c
uint32_t const connectTimeout = 5000U;
uint32_t       connectStartTime;
uint32_t       connectDeltaTime;
uint8_t        connected = TBX_FALSE;

/* Store start time. */
connectStartTime = PortGet()->SystemGetTime();
/* Attempt to connect to the target. */
connected = BltSessionStart();
/* Repeat connection attempt for a finite amount of time. */
while (connected == TBX_FALSE)
{
  /* Attempt to connect to the target. */
  connected = BltSessionStart();
  /* Handle timeout detection if not yet connected. */
  if (connected == TBX_FALSE)
  {
    /* Calculate elapsed time while waiting for the connection to establish. Note
     * that this calculation is 32-bit time overflow safe.
     */
    connectDeltaTime = PortGet()->SystemGetTime() - connectStartTime;
    /* Did a timeout occur? */
    if (connectDeltaTime > connectTimeout)
    {
      /* Could not connect to the target. Stop connection attempts. */
      break;
    }
  }
}
```

#### BltSessionStop

```c
void BltSessionStop(void)
```

Stops the firmware update session. This is there the library disconnects the transport layer as well. Note that if you are currently connected to the OpenBLT bootloader on the target microcontroller, this requests the bootloader to start the user's firmware again, if a valid one is detected by the bootloader.

#### BltSessionClearMemory

```c
uint8_t BltSessionClearMemory(uint32_t address, uint32_t len)
```

Requests the target to erase the specified range of memory on the target. Note that the target automatically aligns this to the erasable memory block sizes. This typically results in more memory being erased than the range that was specified here. Refer to the target implementation for details.

| Parameter | Description                                          |
| --------- | ---------------------------------------------------- |
| `address` | The starting memory address for the erase operation. |
| `len`     | The total number of bytes to erase from memory.      |

| Return value                                  |
| --------------------------------------------- |
| `TBX_OK` if successful, `TBX_ERROR`otherwise. |

**Example**

The flash memory on the ST STM32F0 microcontroller starts at address `0x08000000` and consists of 2k sectors. The following code erases the <u>first two</u> 2k sectors, because the OpenBLT bootloader automatically aligns to the sectors of the microcontroller's flash.

```c
BltSessionClearMemory(0x08000010, 2048);
```

#### BltSessionWriteData

```c
uint8_t BltSessionWriteData(uint32_t address, uint32_t len, uint8_t const * data)
```

Requests the target to program the specified data to memory. Note that it is the responsibility of the application to make sure the memory range was erased beforehand.

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| `address` | The starting memory address for the write operation.         |
| `len`     | The number of bytes in the data buffer that should be written. |
| `data`    | Pointer to the byte array with data to write.                |

| Return value                                  |
| --------------------------------------------- |
| `TBX_OK` if successful, `TBX_ERROR`otherwise. |

**Example**

This code snippet writes 16 bytes to the start of flash on an ST STM32F0 microcontroller:

```c
uint8_t writeData[16] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

BltSessionWriteData(0x08000000, 16, writeData);
```

#### BltSessionReadData

```c
uint8_t BltSessionReadData(uint32_t address, uint32_t len, uint8_t * data)
```

Requests the target to upload the specified range from memory and store its contents in the specified data buffer.

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| `address` | The starting memory address for the read operation.          |
| `len`     | The number of bytes to upload from the target and store in the data buffer. |
| `data`    | Pointer to the byte array where the uploaded data should be stored. |

| Return value                                  |
| --------------------------------------------- |
| `TBX_OK` if successful, `TBX_ERROR`otherwise. |

**Example**

This code snippet reads 16 bytes from the start of flash on an ST STM32F0 microcontroller:

```c
uint8_t readData[16];

BltSessionReadData(0x08000000, 16, readData);
```

### Firmware module

The firmware module embeds all the functionality for reading firmware data from a firmware file. It handles all the file parsing of for example the [S-record](https://en.wikipedia.org/wiki/SREC_(file_format)) firmware file format. The current implementation of LibMicroBLT assumes that file is present on a locally attached FAT32 file system, which the library accesses with the help of [FatFs](http://elm-chan.org/fsw/ff/00index_e.html).

#### BltFirmwareInit

```c
void BltFirmwareInit(uint8_t readerType)
```

Initializes the firmware reader module for a specified firmware file reader. Typically called once upon application initialization.

| Parameter    | Description                                                  |
| ------------ | ------------------------------------------------------------ |
| `readerType` | The firmware file reader to use in this module. It should be a<br>[BLT_FIRMWARE_READER_xxx](#macros) value. |

**Example**

Initialize the firmware module to parse and read firmware data from S-record firmware files:

```c
BltFirmwareInit(BLT_FIRMWARE_READER_SRECORD);
```

#### BltFirmwareTerminate

```c
void BltPortTerminate(void)
```

Terminates the firmware reader module. Typically called at the end of the application when the firmware reader module is no longer needed.

#### BltFirmwareFileOpen

```c
uint8_t BltFirmwareFileOpen(char const * firmwareFile)
```

Opens the firmware file and browses through its contents to collect information about the firmware data segments it contains.

| Parameter      | Description                                |
| -------------- | ------------------------------------------ |
| `firmwareFile` | Firmware filename including its full path. |

| Return value                                   |
| ---------------------------------------------- |
| `TBX_OK` if successful, `TBX_ERROR` otherwise. |

**Example**

Code snippet that opens the firmware file `demoprog_stm32f091.srec`, stored in the `firmwares` directory on the SD cards' first FAT32 partition:

```c
uint8_t fileOpened = TBX_FALSE;

if (BltFirmwareFileOpen("/firmwares/demoprog_stm32f091.srec") == TBX_OK)
{
  fileOpened = TBX_TRUE;
}
```

#### BltFirmwareFileClose

```c
void BltFirmwareFileClose(void)
```

Closes the previously opened firmware file.

#### BltFirmwareGetTotalSize

```c
uint32_t BltFirmwareGetTotalSize(void)
```

Obtains the total number of data bytes present in all segments of the firmware file. You could for example use this number to track the progress of a firmware update, taking into account how many bytes you already programmed with [`BltSessionWriteData()`](#bltsessionwritedata).

| Return value                                                 |
| ------------------------------------------------------------ |
| Total number of firmware data bytes present in all segments of the firmware file. |

#### BltFirmwareSegmentGetCount

```c
uint8_t BltFirmwareSegmentGetCount(void)
```

Obtains the total number of firmware data segments encountered in the firmware file. A firmware data segment consists of a consecutive block of firmware data. A firmware file always has at least one segment. However, it can have more as well. For example if there is a gap between the vector table and the other program data.

| Return value                                                 |
| ------------------------------------------------------------ |
| Total number of firmware data segments present in the firmware file. |

#### BltFirmwareSegmentGetInfo

```c
uint32_t BltFirmwareSegmentGetInfo(uint8_t idx, uint32_t * address)
```

Obtains information about the specified segment, such as the base memory address that its data belongs to and the total number of data bytes in the segment.

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| `idx`     | Zero-based segment index. Valid values are between `0` and<br>`(BltFirmwareSegmentGetCount() - 1)`. |
| `address` | The base memory address of the segment's data is written to this pointer. |

| Return value                                        |
| --------------------------------------------------- |
| The total number of data bytes inside this segment. |

**Example**

Code snippet that loops through all the firmware segments, obtains each segments' info and erases the related sectors in flash memory, on the connected microcontroller. This is what you need to do, before you write the data of the new firmware to flash memory. Note that error checking was left out for clarity.

```c
uint8_t  segmentIdx;
uint32_t segmentBase = 0U;
uint32_t segmentLen;

/* Erase the memory segments on the target that the firmware data covers. */
for (segmentIdx = 0U; segmentIdx < BltFirmwareSegmentGetCount(); segmentIdx++)
{
  /* Obtain segment information such as its base memory adddress and length. */
  segmentLen = BltFirmwareSegmentGetInfo(segmentIdx, &segmentBase);
  /* Erase the segment. */
  BltSessionClearMemory(segmentBase, segmentLen);
}
```

#### BltFirmwareSegmentOpen

```c
void BltFirmwareSegmentOpen(uint8_t idx)
```

Opens the firmware data segment for reading. This should always be called before calling the [`BltFirmwareSegmentGetNextData()`](#bltfirmwaresegmentgetnextdata) function.

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| `idx`     | Zero-based segment index. Valid values are between `0` and<br/>`(BltFirmwareSegmentGetCount() - 1)`. |

#### BltFirmwareSegmentGetNextData

```c
uint8_t const * BltFirmwareSegmentGetNextData(uint32_t * address, uint16_t * len)
```

Obtains a data pointer to the next chunk of firmware data in the segment that was opened with function [`BltFirmwareSegmentOpen()`](#bltfirmwaresegmentopen). The idea is that you first open the segment and afterwards you can keep calling this function to read out the segment's firmware data. When all data is read, `len` will be set to zero and a non-`NULL` pointer is returned.

Note that there are three possible outcomes when calling this function:

1. `len` > `0` and a non-`NULL` pointer is returned. This means valid data was read.
2. `len` = `0` and a non-`NULL` pointer is returned. This means the end of the segment is reached and therefore no new data was actually read.
3. A `NULL` pointer is returned. This happens only when an error occurred.

| Parameter | Description                                                  |
| --------- | ------------------------------------------------------------ |
| `address` | The starting memory address of this chunk of firmware data is written to this pointer. |
| `len`     | The length of the firmware data chunk is written to this pointer. |

| Return value                                                 |
| ------------------------------------------------------------ |
| Data pointer to the read firmware if successful, `NULL` otherwise. |

**Example**

Code snippet that presents a function to write the firmware data from a specific segment of the firmware file, to the flash memory on the connected microcontroller. Note that error checking was left out for clarity.

```c
void writeFirmwareSegmentToFlash(uint8_t segmentIdx)
{
  uint8_t  const * chunkData;
  uint16_t         chunkLen = 0U;
  uint32_t         chunkBase = 0U;

  /* Open the segment for reading. */
  BltFirmwareSegmentOpen(segmentIdx);
    
  /* Attempt to read the first chunk of data in this segment. */
  chunkData = BltFirmwareSegmentGetNextData(&chunkBase, &chunkLen);
  /* Enter loop to write all data chunks to flash. */
  while ( (chunkData != NULL) && (chunkLen > 0U) )
  {
    /* Program the newly read data chunk. */
    BltSessionWriteData(chunkBase, chunkLen, chunkData)
    /* Attempt to read the next chunk of data in this segment. */
    chunkData = BltFirmwareSegmentGetNextData(&chunkBase, &chunkLen);
  }
}
```

