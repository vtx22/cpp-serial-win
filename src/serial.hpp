#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <windows.h>
#include <vector>
#include <stdint.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

/*
Serial Port Error Codes
*/
enum SERIAL_ERR
{
   OK = 0,
   NOT_OPEN = -1,
   READ_ERR = -2,
   WRITE_ERR = -3,
   SET_COM_STATE = -4,
   GET_COM_STATE = -5,
   SET_TIMEOUTS = -6,
   CLOSE_ERR = -7,
   OPEN_ERR = -8,
   INVALID_PARAM = -9,
   PORT_NOT_FOUND = -10,
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

   int write(const std::vector<uint8_t> &data);
   int write(const uint8_t *data, int length);

   int print(std::string message);
   int print(float value);
   int print(float value, uint8_t precision);

   int set_comm_state(uint32_t baud, uint8_t byte_size, uint8_t stop_bits, uint8_t parity);
   int set_timeouts(uint32_t rd_interval, uint32_t rd_total, uint32_t rd_mult, uint32_t wr_total, uint32_t wr_mult);

   int set_baud_rate(uint32_t baud);

   bool get_open();

private:
   int update_com_state();
   int update_timeouts();

   const char *_port;

   bool _is_open = false;

   uint32_t _baud = 115200;
   uint8_t _byte_size = 8;
   uint8_t _stop_bits = ONESTOPBIT;
   uint8_t _parity = NOPARITY;

   HANDLE _handle;

   DCB _serial_params = {0};
   COMMTIMEOUTS _timeouts = {0};
};

#endif // SERIAL_HPP