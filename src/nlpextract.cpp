#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>

// From
// http://stackoverflow.com/questions/9943187/colour-output-of-program-run-under-bash

enum Color { NONE = 0, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

std::string set_color(Color foreground = Color::NONE,
                      Color background = Color::NONE) {
  std::stringstream s;
  s << "\033[";

  if (!foreground && !background) s << "0";  // reset colors if no params

  if (foreground) {
    s << 29 + foreground;
  }

  if (background) {
    s << ";";
    s << 39 + background;
  }

  s << "m";
  return s.str();
}

//

const uint BlockAlign = 0x800;
const uint BlockAlignMask = BlockAlign - 1;

inline bool file_exists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

void unpack_image(const std::string& filename) {
  std::fstream fs(filename.c_str(),
                  std::fstream::in | std::fstream::out | std::ios_base::binary);

  int index = 0;
}

int main(int argc, char** argv) {
  std::cout << set_color(Color::CYAN)
            << "[NLPUnpacker (C++ Edition)] New Love Plus for 3DS \"img.bin\" "
               "unpacker"
            << std::endl
            << "Original code by gdkchan" << std::endl
            << "C++ port and further modifications by Katherine Whitlock aka "
               "toroidal-code"
            << std::endl
            << "Version 0.1.0" << std::endl
            << std::endl
            << set_color();

  if (argc > 1) {
    auto filename = std::string(argv[1]);
    if (file_exists(filename)) {
      unpack_image(filename);
      std::cout << set_color(Color::GREEN) << "Done!" << std::endl;
    }
  } else {
    std::cout << set_color(Color::RED) << "Error! No file given" << set_color()
              << std::endl
              << "Usage: nlpextract path/to/img.bin" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}
