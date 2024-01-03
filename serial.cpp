#include "serial.h"
#include <cstring> // for memset
#include <fcntl.h>
#include <iostream>
#include <linux/serial.h>
#include <sstream>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define _POSIX_SOURCE 1 /* POSIX compliant source */

Serial::Serial(Board &board_, const std::string &dev_)
    : board(board_), dev(dev_), running(true), t([this] { run(); }) {}

Serial::~Serial() {
  running = false;
  t.join();
}

void Serial::setRts(bool b) {
  // std::cout<<"setting RTS to "<<b<<std::endl;
  int status;
  ioctl(fd, TIOCMGET, &status);
  if (((status & TIOCM_RTS) != 0) == b) {
    return;
  }

  status ^= TIOCM_RTS;
  ioctl(fd, TIOCMSET, &status);
}

void Serial::run() {
  static const int BAUDRATE = B4800;
  struct termios tty, oldtio;
  char buf[255];
  const char *device = dev.c_str();

  fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);

  if (fd < 0) {
    std::stringstream ss;
    ss << "Cannot open " << device << std::endl;
    throw std::runtime_error(ss.str());
  }

  tcgetattr(fd, &oldtio); /* save current port settings */

  serial_struct serial;
  ioctl(fd, TIOCGSERIAL, &serial);
  serial.flags |= ASYNC_LOW_LATENCY;
  ioctl(fd, TIOCSSERIAL, &serial);

  if (tcgetattr(fd, &tty) < 0) {
    std::stringstream ss;
    ss << "Error from tcgetattr: " << strerror(errno) << std::endl;
    throw std::runtime_error(ss.str());
  }

  cfsetospeed(&tty, (speed_t)BAUDRATE);
  cfsetispeed(&tty, (speed_t)BAUDRATE);

  tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;      /* 8-bit characters */
  tty.c_cflag &= ~PARENB;  /* no parity bit */
  tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;

  tcflush(fd, TCIFLUSH);

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    std::stringstream ss;
    ss << "Error from tcsetattr: " << strerror(errno);
    throw std::runtime_error(ss.str());
  }

  // setRts(false);

  while (running) { /* loop for input */
    // std::cout<<"starting read"<<std::endl;
    int res = read(fd, buf, 255);
    setRts(board.isBusy());
    if (res) {
      // std::cout<<"received"<<res<<" bytes"<<std::endl;
      board.pushData(reinterpret_cast<const uint8_t *>(buf), res);
    } else {
      // std::cout<<"no data"<<std::endl;
    }
  }
  tcsetattr(fd, TCSANOW, &oldtio);
}
