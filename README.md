# C++ Serial for Windows
C++ Class for Serial Interface on Windows

## Minimal Usage Example

```C++
#include <iostream>
#include <stdint.h>

#include "serial.hpp"

int main()
{
    Serial sp;

    if(sp.open("COM11", 115200) != 0)
    {
        std::cout << "Can't open port\n"; 
        return 0;
    }

    uint8_t data[3] = {0x22, 0xFF, 0x11};
  
    if(sp.write(data, 3) != 3)
    {
        std::cout << "Failed to write 3 Bytes to Port!\n";
    }

}

```
