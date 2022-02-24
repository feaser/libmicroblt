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
| `XcpComputeKeyFromSeed` | Function pointer to calculates the key to unlock the programming<br>resource, based on the given seed. This function should return `TBX_OK`<br>if the key could be calculated, `TBX_ERROR` otherwise. |

## Functions

### Port module

#### BltPortInit

```c
void BltPortInit(tPort const * port)
```

Initializes the port module by linking the port specified by the parameter. Typically called once during firmware initialization.

| Parameter | Description                                         |
| --------- | --------------------------------------------------- |
| `port`    | Pointer to the port to link (type [tPort](#tport)). |

#### BltPortTerminate

```c
void BltPortTerminate(void)
```

Terminates the port module. Typically called once during firmware exit. Embedded firmware tends to be driven by an infinite program loop and never exits. In such cases, you can omit calling this function.

### Session module

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

#### BltSessionStop

```c
void BltSessionStop(void)
```

Stops the firmware update session. This is there the library disconnects the transport layer as well.

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

#### BltFirmwareInit



#### BltFirmwareTerminate



#### BltFirmwareFileOpen



#### BltFirmwareFileClose



#### BltFirmwareGetTotalSize



#### BltFirmwareSegmentGetCount



#### BltFirmwareSegmentGetInfo



#### BltFirmwareSegmentOpen



#### BltFirmwareSegmentGetNextData
