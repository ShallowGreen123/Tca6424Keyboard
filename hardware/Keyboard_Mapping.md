# Keyboard Mapping

This document captures the keyboard mapping currently used by
`Keyboard_TCA6424_Test`.

It is derived from:

- `T-KeyDriver.pdf`: electrical row/column wiring
- `cow.png`: hardware row/column to key-value relationship
- `123.png`: simulated physical keyboard layout and key values

## Row Mapping

| Scanner Row | Signal | TCA6424 Port |
| --- | --- | --- |
| R1 | ROW25 | P00 |
| R2 | ROW1 | P01 |
| R3 | ROW2 | P02 |
| R4 | ROW4 | P03 |
| R5 | ROW5 | P04 |
| R6 | ROW6 | P05 |
| R7 | ROW9 | P06 |
| R8 | ROW10 | P07 |
| R9 | ROW12 | P10 |

## Column Mapping

| Scanner Col | Signal | TCA6424 Port |
| --- | --- | --- |
| C1 | COL3 | P11 |
| C2 | COL7 | P12 |
| C3 | COL8 | P13 |
| C4 | COL11 | P14 |
| C5 | COL13 | P15 |
| C6 | COL14 | P16 |
| C7 | COL15 | P17 |
| C8 | COL16 | P20 |
| C9 | COL17 | P21 |
| C10 | COL18 | P22 |
| C11 | COL22 | P23 |
| C12 | COL23 | P24 |
| C13 | COL24 | P25 |
| C14 | COL26 | P26 |

## Notes

- `P27` is not used by the keyboard matrix.
- The scanner now reports events as `PRESS/RELEASE key=<value> RnCm [row=Pxx/ROW... col=Pyy/COL...]`.
- `RnCm` is the electrical matrix position and `key` is the logical keyboard value taken from `123.png`.

## Full Key Mapping

| Key | Matrix | Row Signal | Row Port | Col Signal | Col Port |
| --- | --- | --- | --- | --- | --- |
| 1 | R9C4 | ROW12 | P10 | COL11 | P14 |
| 2 | R8C4 | ROW10 | P07 | COL11 | P14 |
| 3 | R6C5 | ROW6 | P05 | COL13 | P15 |
| 4 | R7C5 | ROW9 | P06 | COL13 | P15 |
| 5 | R6C6 | ROW6 | P05 | COL14 | P16 |
| 6 | R7C6 | ROW9 | P06 | COL14 | P16 |
| 7 | R6C7 | ROW6 | P05 | COL15 | P17 |
| 8 | R7C7 | ROW9 | P06 | COL15 | P17 |
| 9 | R6C8 | ROW6 | P05 | COL16 | P20 |
| 10 | R7C8 | ROW9 | P06 | COL16 | P20 |
| 11 | R6C9 | ROW6 | P05 | COL17 | P21 |
| 15 | R8C12 | ROW10 | P07 | COL23 | P24 |
| 16 | R7C4 | ROW9 | P06 | COL11 | P14 |
| 17 | R6C4 | ROW6 | P05 | COL11 | P14 |
| 18 | R5C5 | ROW5 | P04 | COL13 | P15 |
| 19 | R4C5 | ROW4 | P03 | COL13 | P15 |
| 20 | R5C6 | ROW5 | P04 | COL14 | P16 |
| 21 | R4C6 | ROW4 | P03 | COL14 | P16 |
| 22 | R5C7 | ROW5 | P04 | COL15 | P17 |
| 23 | R4C7 | ROW4 | P03 | COL15 | P17 |
| 24 | R5C8 | ROW5 | P04 | COL16 | P20 |
| 25 | R5C9 | ROW5 | P04 | COL17 | P21 |
| 26 | R4C9 | ROW4 | P03 | COL17 | P21 |
| 29 | R7C12 | ROW9 | P06 | COL23 | P24 |
| 30 | R2C4 | ROW1 | P01 | COL11 | P14 |
| 31 | R4C12 | ROW4 | P03 | COL23 | P24 |
| 32 | R9C12 | ROW12 | P10 | COL23 | P24 |
| 33 | R5C4 | ROW5 | P04 | COL11 | P14 |
| 34 | R3C5 | ROW2 | P02 | COL13 | P15 |
| 35 | R3C6 | ROW2 | P02 | COL14 | P16 |
| 36 | R3C7 | ROW2 | P02 | COL15 | P17 |
| 37 | R4C8 | ROW4 | P03 | COL16 | P20 |
| 38 | R3C8 | ROW2 | P02 | COL16 | P20 |
| 39 | R3C9 | ROW2 | P02 | COL17 | P21 |
| 40 | R3C10 | ROW2 | P02 | COL18 | P22 |
| 43 | R6C12 | ROW6 | P05 | COL23 | P24 |
| 44 | R2C11 | ROW1 | P01 | COL22 | P23 |
| 46 | R2C5 | ROW1 | P01 | COL13 | P15 |
| 47 | R4C4 | ROW4 | P03 | COL11 | P14 |
| 48 | R3C4 | ROW2 | P02 | COL11 | P14 |
| 49 | R2C6 | ROW1 | P01 | COL14 | P16 |
| 50 | R2C7 | ROW1 | P01 | COL15 | P17 |
| 51 | R4C10 | ROW4 | P03 | COL18 | P22 |
| 52 | R2C10 | ROW1 | P01 | COL18 | P22 |
| 53 | R2C8 | ROW1 | P01 | COL16 | P20 |
| 54 | R5C10 | ROW5 | P04 | COL18 | P22 |
| 55 | R6C10 | ROW6 | P05 | COL18 | P22 |
| 57 | R9C11 | ROW12 | P10 | COL22 | P23 |
| 58 | R3C13 | ROW2 | P02 | COL24 | P25 |
| 59 | R8C3 | ROW10 | P07 | COL8 | P13 |
| 60 | R9C2 | ROW12 | P10 | COL7 | P12 |
| 61 | R2C9 | ROW1 | P01 | COL17 | P21 |
| 62 | R2C2 | ROW1 | P01 | COL7 | P12 |
| 79 | R7C10 | ROW9 | P06 | COL18 | P22 |
| 83 | R5C12 | ROW5 | P04 | COL23 | P24 |
| 84 | R3C12 | ROW2 | P02 | COL23 | P24 |
| 89 | R2C12 | ROW1 | P01 | COL23 | P24 |
| 110 | R9C5 | ROW12 | P10 | COL13 | P15 |
| 112 | R8C5 | ROW10 | P07 | COL13 | P15 |
| 113 | R9C6 | ROW12 | P10 | COL14 | P16 |
| 114 | R8C6 | ROW10 | P07 | COL14 | P16 |
| 115 | R9C7 | ROW12 | P10 | COL15 | P17 |
| 116 | R8C7 | ROW10 | P07 | COL15 | P17 |
| 117 | R9C8 | ROW12 | P10 | COL16 | P20 |
| 118 | R8C8 | ROW10 | P07 | COL16 | P20 |
| 119 | R9C9 | ROW12 | P10 | COL17 | P21 |
| 120 | R8C9 | ROW10 | P07 | COL17 | P21 |
| 121 | R9C10 | ROW12 | P10 | COL18 | P22 |
| 122 | R8C10 | ROW10 | P07 | COL18 | P22 |
| 126 | R1C14 | ROW25 | P00 | COL26 | P26 |
| 127 | R9C1 | ROW12 | P10 | COL3 | P11 |
