#include "serial.hpp"

Serial::Serial()
{
   _serial_params.DCBlength = sizeof(_serial_params);

   set_timeouts(50, 50, 0, 50, 0);
}

Serial::~Serial()
{
   close();
}

/*
Open a COM Port
@param port Port Name, e.g. "COM11"
@param baud Baud Rate
@return Returns 0 when successful
*/
int Serial::open(const char *port, uint32_t baud)
{
   if (_is_open)
   {
      close();
   }

   _port = port;

   std::string p = "\\\\.\\";
   p += _port;

   _handle = CreateFile(p.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   if (_handle == INVALID_HANDLE_VALUE)
   {
      if (GetLastError() == ERROR_FILE_NOT_FOUND)
      {
         return SERIAL_ERR::PORT_NOT_FOUND;
      }

      return SERIAL_ERR::OPEN_ERR;
   }

   _is_open = true;

   int err = update_com_state();

   if (err != SERIAL_ERR::OK)
   {
      return err;
   }

   err = update_timeouts();

   if (err != SERIAL_ERR::OK)
   {
      return err;
   }

   return SERIAL_ERR::OK;
}

/*
Close the COM Port if open
@return Returns 0 when successful
*/
int Serial::close()
{
   _is_open = false;

   return (CloseHandle(_handle) ? SERIAL_ERR::OK : SERIAL_ERR::CLOSE_ERR);
}

/*
Send bytes of vector via the open COM Port
@param data Data vector that contains the bytes for sending
@return Returns number of bytes written or error code
*/
int Serial::write(const std::vector<uint8_t> &data)
{
   return write(data.data(), data.size());
}

/*
Send bytes of array via the open COM Port
@param data Data byte array
@param length Number of bytes to send
@return Returns number of bytes sent or error code
*/
int Serial::write(const uint8_t *data, int length)
{
   if (!_is_open)
   {
      return SERIAL_ERR::NOT_OPEN;
   }

   DWORD bytes_written = 0;

   if (!WriteFile(_handle, (char *)data, length, &bytes_written, NULL))
   {
      return SERIAL_ERR::WRITE_ERR;
   }

   return bytes_written;
}

/*
Send a string message
@param message String to send
@return Returns number of bytes sent or error code
*/
int Serial::print(std::string message)
{
   return write((uint8_t *)message.c_str(), message.size());
}

/*
Send float as string
@param value Float value to print
@return Returns number of bytes sent or error code
*/
int Serial::print(float value)
{
   std::stringstream ss;
   ss << value;

   return print(ss.str());
}

/*
Send float as string with variable precision
@param value Float value to print
@param precision Number of decimal places
@return Returns number of bytes sent or error code
*/
int Serial::print(float value, uint8_t precision)
{
   std::stringstream ss;
   ss << std::setprecision(precision) << value;

   return print(ss.str());
}

/*
Read bytes to buffer array
@param data Data byte array for buffering the data
@param length Number of bytes to read
@return Returns number of bytes read or error code
*/
int Serial::read(uint8_t *buffer, int length)
{
   if (!_is_open)
   {
      return SERIAL_ERR::NOT_OPEN;
   }

   DWORD bytes_read = 0;

   if (!ReadFile(_handle, (char *)buffer, length, &bytes_read, NULL))
   {
      return SERIAL_ERR::READ_ERR;
   }

   return bytes_read;
}

/*
Applies the saved communication settings to the open port
@return Returns 0 when successful
*/
int Serial::update_com_state()
{
   if (!_is_open)
   {
      return SERIAL_ERR::NOT_OPEN;
   }

   if (!GetCommState(_handle, &_serial_params))
   {
      return SERIAL_ERR::GET_COM_STATE;
   }

   _serial_params.BaudRate = _baud;
   _serial_params.ByteSize = _byte_size;
   _serial_params.StopBits = _stop_bits;
   _serial_params.Parity = _parity;

   if (!SetCommState(_handle, &_serial_params))
   {
      return SERIAL_ERR::SET_COM_STATE;
   }

   return SERIAL_ERR::OK;
}

/*
Applies the saved timeout settings to the open port
@return Returns 0 when successful
*/
int Serial::update_timeouts()
{
   if (_is_open)
   {
      return SERIAL_ERR::NOT_OPEN;
   }

   return (SetCommTimeouts(_handle, &_timeouts) ? SERIAL_ERR::OK : SERIAL_ERR::SET_TIMEOUTS);
}

/*
Update the communication settings. If the port is not open, settings are saved and applied on open() call
@param baud Baud Rate
@param byte_size Number of bits per byte, allowed are: 4, 5, 6, 7 or 8
@param stop_bits Stop bits, where 0 = 1 bit, 1 = 1.5 bits and 2 = 2 bits
@param parity Parity Bit, where 0 = None, 1 = Odd, 2 = Even, 3 = Mark and 4 = Space
@return Returns 0 when successful
*/
int Serial::set_comm_state(uint32_t baud, uint8_t byte_size, uint8_t stop_bits, uint8_t parity)
{
   // Ditch forbidden settings
   if (parity > 4 || stop_bits > 3 || byte_size < 4 || byte_size > 8)
   {
      return SERIAL_ERR::INVALID_PARAM;
   }

   _baud = baud;
   _byte_size = byte_size;
   _stop_bits = stop_bits;
   _parity = parity;

   return update_com_state();
}

/*
Set the read and write timeouts. If the port is not open, settings are saved and applied on open() call
@param rd_interval Maximum time between read chars
@param rd_total Constant in milliseconds
@param rd_mult Multiplier of characters
@param wr_total Constant in milliseconds
@param wr_mult Multiplier of characters
@return Returns 0 when successful
*/
int Serial::set_timeouts(uint32_t rd_interval, uint32_t rd_total, uint32_t rd_mult, uint32_t wr_total, uint32_t wr_mult)
{
   _timeouts.ReadIntervalTimeout = rd_interval;
   _timeouts.ReadTotalTimeoutConstant = rd_total;
   _timeouts.ReadTotalTimeoutMultiplier = rd_mult;

   _timeouts.WriteTotalTimeoutConstant = wr_total;
   _timeouts.WriteTotalTimeoutMultiplier = wr_mult;

   int err = update_timeouts();

   if (err == SERIAL_ERR::NOT_OPEN)
   {
      return SERIAL_ERR::OK;
   }

   return err;
}

/*
Set the baud rate. If the port is not open, baud rate is saved and applied on open() call
@return Returns 0 when successful
*/
int Serial::set_baud_rate(uint32_t baud)
{
   _baud = baud;

   int err = update_com_state();

   if (err == SERIAL_ERR::NOT_OPEN)
   {
      return SERIAL_ERR::OK;
   }

   return err;
}

/*
Get the last know state of the port (open/closed)
@return Returns true if the port was opened, false if the port is closed
*/
bool Serial::get_open()
{
   return _is_open;
}

/*
Get the ids of all available COM Ports
@return Returns a vector that contains all port ids, e.g. "COM11" -> 11
*/
std::vector<uint8_t> Serial::get_port_ids()
{
   char target_path[5000]; // buffer to store the path of the COMPORTS
   std::vector<uint8_t> port_ids;

   for (uint8_t i = 0; i < 255; i++)
   {
      std::string str = "COM" + std::to_string(i);
      DWORD port = QueryDosDevice(str.c_str(), target_path, 5000);

      if (port != 0)
      {
         port_ids.push_back(i);
      }
   }

   return port_ids;
}

/*
Get the names of all available COM Ports
@return Returns a string vector that contains all port names "COMxx"
*/
std::vector<std::string> Serial::get_port_names()
{
   return get_port_names(true);
}

/*
Get the names of all available COM Ports
@param add_prefix If true, "COM" prefix is added to the port number string
@return Returns a string vector that contains all port names with or without prefix
*/
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