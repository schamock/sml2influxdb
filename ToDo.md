# ToDos
## main.c
- nextChar instead of nextChar[1]?
- memset smlString at reset really necessary??
## serial.c
- var tty only inside initSerial()
- serialPort shouldn't be a global var in serial.c -> move to main()
- readCharacter() -> check return values of read. ret=0?
- readCharacter() -> only char instead of char[1]?
## sml.c
- add result struct
- add all values
- length can constist of more bytes
- function to extract data