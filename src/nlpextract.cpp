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

/**
 * Simple function to read a string from some istream
 *
 * @param istream the istream to read from
 * @param length the length of the string
 *
 * @return the extracted string
 */
inline std::string read_string(std::istream& istream, int length) {
  char buffer[length];

  // original program used C# "read" which is
  // read(byte[], num bytes, num chars) thus, I'm using "get" instead of the C++
  // "read", which only handles bytes, not characters
  // here
  istream.get(buffer, 4);

  return std::string(buffer);
}

int extract_package(std::fstream& file, std::string output_folder) {
  uint base_offset = static_cast<uint>(file.tellg());

  std::string pack_signature = read_string(file, 4);
  if (pack_signature != "PACK") {  // verify that we're extracting a package
    return -1;
  }
}

void unpack_image(const std::string& filename) {
  std::fstream file(filename.c_str(), std::fstream::in | std::fstream::out |
                                          std::fstream::binary);
  int index = 0;

  // Save the length of the file
  file.seekg(0, file.end);
  int length = file.tellg();
  file.seekg(0, file.beg);

  while (file.tellg() <= length) {
    std::cout << set_color(Color::YELLOW) << "[Package " << index
              << " at offset 0x" << std::hex << file.tellg() << std::dec << "]"
              << set_color() << std::endl;

    // TODO: Figure out how to do filepath/directory manipulation here.
    // Maybe use apathy library? Would require submoduling project into /include
    // folder.

    // I feel weird about doing side-effecting computations. Might alter
    // extract_package to be something that returns a true value, not just a
    // number. We could accumulate packages and then dump them all at once.
    if (extract_package(file, "") > 0) {
      index++;
    } else {
      // Delete the package_folder directory
    }

    if ((file.tellg() & BlockAlignMask) != 0) {
      long position = (file.tellg() & ~BlockAlignMask) + BlockAlign;
      if (position >= length) break;
      file.seekg(position, file.beg);
    }

    std::cout << std::endl;
  }
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
