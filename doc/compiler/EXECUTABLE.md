# Chapman Executable Format

### General layout
Any Chapman executable contains the following sections, in this order:

- Header
- Function name table (similar to a Map<String, FunctionPointer>) 
- Blob section: This section contains all the constants used by the program (doubles, strings, etc.)
- Program section: The executable section of the program

### Header
| Field name          	| Size (bytes) 	| Type     	| Comment                                                                                                                         	|
|---------------------	|--------------	|----------	|---------------------------------------------------------------------------------------------------------------------------------	|
| Version             	| 1            	| uint8_t  	| Executable version.                                                                                                             	|
| Function table size 	| 2            	| uint16_t 	| The number of **entries** in the function table. Each entry is of fixed size, since the function name is represented as a hash. 	|
| Blob section size   	| 4            	| uint32_t 	| Size of the section, in bytes.                                                                                                  	|