# Intruduction

This repository is for `Nuts` using `STM32` chips, including:

- stm32f103c8
- stm32f407vg
- stm32l476rgt6



# Build projects

Develop tool [STM32CubeIDE 1.16.0](https://www.st.com/en/development-tools/stm32cubeide.html#st-get-software) 

1. Clone current repo `git clone`
4. Open STM32CubeIDE
   1. Navigate to repo folder，`File -> Switch WorkSpace -> other -> ../nut_st_workspace` ，
   2. Import projects，`File -> Import -> Existing Porjects into Workspace`，`root directory` choose `nut_st_workspace`

![Pasted image 20241207125232](./.README.assets/Pasted%20image%2020241207125232.png)



---



# Nuts Commands

## Protocol

**Request**

```
┌────────────────-──┬───────┐
│---Request Header--│-------│
│Command│ RFU│Length│PayLoad│
│-------------------│-------│
│  2B   │ 2B │  4B  │$Length│
└───────┴───┴───────┴───────┘
```

**Response**

```
┌───────────────────┬───────┐
│--Response Header--│-------│
│ Status │  Length  │PayLoad│
│-------------------│-------│
│   2B   │    4B    │$Length│
└────────┴──────────┴───────┘
```



## Command



### Set AES encryption key

**Request**

| Command    | RFU    | Length   | Payload |
| ---------- | ------ | -------- | ------- |
| **0x0100** | 0x0000 | len(key) | key     |

**Response**

| Status | Length     | Payload |
| ------ | ---------- | ------- |
| 0x0000 | 0x00000000 | None    |

**Example**

```
send   >> 01 00 00 00 00 00 00 10 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 00 
```

**Note: The length of AES key support 16 Bytes(128 bits), 24 Bytes(192 bits) and 32 Bytes(256 bits)**



### Set AES decryption key

**Request**

| Command    | RFU    | Length   | Payload |
| ---------- | ------ | -------- | ------- |
| **0x0101** | 0x0000 | len(key) | key     |

**Response**

| Status | Length     | Payload |
| ------ | ---------- | ------- |
| 0x0000 | 0x00000000 | None    |

**Example**

```
send   >> 01 01 00 00 00 00 00 10 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 00 
```

**Note: The length of AES key support 16 Bytes(128 bits), 24 Bytes(192 bits) and 32 Bytes(256 bits)**



### AES encryption

**Request**

| Command    | RFU    | Length     | Payload    |
| ---------- | ------ | ---------- | ---------- |
| **0x0102** | 0x0000 | 0x00000010 | plain(16B) |

**Response**

| Status | Length     | Payload     |
| ------ | ---------- | ----------- |
| 0x0000 | 0x00000010 | cipher(16B) |

**Example**

```
send   >> 01 02 00 00 00 00 00 10 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 10 62 F6 79 BE 2B F0 D9 31 64 1E 03 9C A3 40 1B B2
```

### AES decryption

**Request**

| Command    | RFU    | Length     | Payload     |
| ---------- | ------ | ---------- | ----------- |
| **0x0103** | 0x0000 | 0x00000010 | cipher(16B) |

**Response**

| Status | Length     | Payload    |
| ------ | ---------- | ---------- |
| 0x0000 | 0x00000010 | plain(16B) |

**Example**

```
send   >> 01 03 00 00 00 00 00 10 62 F6 79 BE 2B F0 D9 31 64 1E 03 9C A3 40 1B B2
return << 00 00 00 00 00 10 00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF
```

### Set DES encryption key

**Request**

| Command    | RFU    | Length     | Payload |
| ---------- | ------ | ---------- | ------- |
| **0x0200** | 0x0000 | 0x00000008 | key(8B) |

**Response**

| Status | Length     | Payload |
| ------ | ---------- | ------- |
| 0x0000 | 0x00000000 | None    |

**Example**

```
send   >> 02 00 00 00 00 00 00 08 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 00 
```



### Set DES decryption key

**Request**

| Command    | RFU    | Length     | Payload |
| ---------- | ------ | ---------- | ------- |
| **0x0201** | 0x0000 | 0x00000008 | key(8B) |

**Response**

| Status | Length     | Payload |
| ------ | ---------- | ------- |
| 0x0000 | 0x00000000 | None    |

**Example**

```
send   >> 02 00 00 00 00 00 00 08 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 00 
```

### DES encryption

**Request**

| Command    | RFU    | Length     | Payload   |
| ---------- | ------ | ---------- | --------- |
| **0x0202** | 0x0000 | 0x00000008 | plain(8B) |

**Response**

| Status | Length     | Payload    |
| ------ | ---------- | ---------- |
| 0x0000 | 0x00000008 | cipher(8B) |

**Example**

```
send   >> 02 02 00 00 00 00 00 08 88 99 AA BB CC DD EE FF
return << 00 00 00 00 00 08 97 9F FF 9B 97 0C A6 A4  
```



### DES decryption

**Request**

| Command    | RFU    | Length     | Payload    |
| ---------- | ------ | ---------- | ---------- |
| **0x0203** | 0x0000 | 0x00000008 | cipher(8B) |

**Response**

| Status | Length     | Payload   |
| ------ | ---------- | --------- |
| 0x0000 | 0x00000008 | plain(8B) |

**Example**

```
send   >> 02 03 00 00 00 00 00 08 97 9F FF 9B 97 0C A6 A4
return << 00 00 00 00 00 08 88 99 AA BB CC DD EE FF
```

















