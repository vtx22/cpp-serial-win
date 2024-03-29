#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <windows.h>
#include <vector>
#include <stdint.h>
#include <string>
#include <iostream>

enum SERIAL_ERR
{
   NOT_OPEN = -1,
   READ_ERR = -2,
};

class Serial
{
public:
   Serial();

   ~Serial();

   int open(const char *port, uint32_t baud);
   int close();

   static std::vector<uint8_t> get_port_ids();
   static std::vector<std::string> get_port_names();
   static std::vector<std::string> get_port_names(bool add_prefix);

   int read(uint8_t *buffer, int n);

   int write(std::vector<uint8_t> data);
   int write(uint8_t *data, int length);

   bool set_comm_state(uint32_t baud, uint8_t byte_size, uint8_t stop_bits, uint8_t parity);
   bool set_comm_state(DCB state);

   bool set_baud_rate(uint32_t baud);

   bool get_open();

private:
   const char *_port;

   bool _is_open = false;

   HANDLE _handle;
   DCB _serial_params = {0};
};

#endif // SERIAL_HPP