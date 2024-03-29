#include "serial.hpp"

Serial::Serial()
{
   _serial_params.DCBlength = sizeof(_serial_params);
}

Serial::~Serial()
{
   close();
}

int Serial::open(const char *port, uint32_t baud)
{
   _port = port;

   std::string p = "\\\\.\\";
   p += _port;
   _handle = CreateFile(p.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   if (_handle == INVALID_HANDLE_VALUE)
   {
      if (GetLastError() == ERROR_FILE_NOT_FOUND)
      {
         // Port not found
         return -1;
      }
      // Other error
      return -2;
   }

   set_comm_state(baud, 8, ONESTOPBIT, NOPARITY);

   // Set timeouts
   COMMTIMEOUTS timeout = {0};
   timeout.ReadIntervalTimeout = 1;
   timeout.ReadTotalTimeoutConstant = 0;
   timeout.ReadTotalTimeoutMultiplier = 0;
   // timeout.WriteTotalTimeoutConstant = 50;
   // timeout.WriteTotalTimeoutMultiplier = 10;

   if (!SetCommTimeouts(_handle, &timeout))
   {
      return -4;
   }

   _is_open = true;

   return 0;
}

int Serial::close()
{
   _is_open = false;
   return CloseHandle(_handle);
}

int Serial::write(std::vector<uint8_t> data)
{
   return write(&data[0], data.size());
}

int Serial::write(uint8_t *data, int length)
{
   DWORD bytes_written = 0;

   if (!WriteFile(_handle, (char *)data, length, &bytes_written, NULL))
   {
      return -1;
   }

   return bytes_written;
}

int Serial::read(uint8_t *buffer, int n)
{
   if (!_is_open)
   {
      return SERIAL_ERR::NOT_OPEN;
   }

   DWORD bytes_read = 0;

   if (!ReadFile(_handle, (char *)buffer, n, &bytes_read, NULL))
   {
      return SERIAL_ERR::READ_ERR;
   }

   return bytes_read;
}

std::vector<uint8_t> Serial::get_port_ids()
{
   char target_path[5000]; // buffer to store the path of the COMPORTS
   std::vector<uint8_t> port_ids;

   for (uint8_t i = 0; i < 255; i++)
   {
      std::string str = "COM" + std::to_string(i); // converting to COM0, COM1, COM2
      DWORD port = QueryDosDevice(str.c_str(), target_path, 5000);

      if (port != 0)
      {
         port_ids.push_back(i);
      }
   }

   return port_ids;
}

std::vector<std::string> Serial::get_port_names()
{
   return get_port_names(true);
}

std::vector<std::string> Serial::get_port_names(bool add_prefix)
{
   std::vector<uint8_t> port_ids = get_port_ids();
   std::vector<std::string> coms;

   if (add_prefix)
   {
      for (const auto &id : port_ids)
      {
         coms.push_back(std::string("COM") + std::to_string(id));
      }
   }

   return coms;
}

bool Serial::set_comm_state(uint32_t baud, uint8_t byte_size, uint8_t stop_bits, uint8_t parity)
{
   // Ditch forbidden settings
   if (parity > 4 || stop_bits > 3 || byte_size < 4 || byte_size > 8)
   {
      return false;
   }

   if (!GetCommState(_handle, &_serial_params))
   {
      return false;
   }

   _serial_params.BaudRate = baud;
   _serial_params.ByteSize = byte_size;
   _serial_params.StopBits = stop_bits;
   _serial_params.Parity = parity;

   return SetCommState(_handle, &_serial_params);
}

bool Serial::set_comm_state(DCB state)
{
   _serial_params = state;

   return SetCommState(_handle, &_serial_params);
}

bool Serial::set_baud_rate(uint32_t baud)
{
   if (!GetCommState(_handle, &_serial_params))
   {
      return false;
   }

   _serial_params.BaudRate = baud;

   return SetCommState(_handle, &_serial_params);
}

bool Serial::get_open()
{
   return _is_open;
}