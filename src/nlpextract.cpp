#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <unistd.h>
#include <bitset>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

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

#ifdef _WIN32
inline bool file_exists(const std::string& name) {
  DWORD dwAttrib = GetFileAttributes(name.c_str());
  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
          !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
#else
inline bool file_exists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}
#endif

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
  istream.read(buffer, 4);
  return std::string(buffer);
}

inline std::string read_null_terminated_string(std::istream& istream,
                                               uint32_t offset) {
  istream.seekg(offset, istream.beg);
  std::vector<char> buffer;

  for (;;) {
    char character;
    istream.read(&character, 1);  // read a single byte
    if (character == '\0') break;
    buffer.push_back(character);
  }

  return std::string(buffer.begin(), buffer.end());
}

inline uint32_t read_uint32(std::istream& istream) {
  uint32_t value;
  istream.read(reinterpret_cast<char*>(&value), sizeof value);
  return value;
};

int extract_package(std::fstream& file, std::string output_folder) {
  uint base_offset = static_cast<uint>(file.tellg());

  std::string pack_signature = read_string(file, 4);
  if (pack_signature != "PACK") {  // verify that we're extracting a package
    return -1;
  }

  // TODO: turn this into a packed struct that we read all at once. Seriously.
  uint32_t file_count = read_uint32(file) >> 16;
  uint32_t string_pointer_offset = read_uint32(file) + base_offset;
  uint32_t string_table_offset = read_uint32(file) + base_offset;
  uint32_t data_offset = read_uint32(file) + base_offset;
  uint32_t decompressed_section_length = read_uint32(file);
  uint32_t compressed_section_length = read_uint32(file);
  uint32_t padding = read_uint32(file);

  std::cout << std::uppercase << "File count: " << file_count << std::endl
            << "String pointer offset: "
            << "0x" << std::hex << string_pointer_offset << std::dec << " ("
            << string_pointer_offset << ")" << std::endl
            << "String table offset: "
            << "0x" << std::hex << string_table_offset << std::dec << " ("
            << string_table_offset << ")" << std::endl
            << "Data offset: "
            << "0x" << std::hex << data_offset << std::dec << " ("
            << data_offset << ")" << std::endl
            << "Decompressed section length: " << decompressed_section_length
            << std::endl
            << "Compressed section length: " << compressed_section_length
            << std::endl
            << "Padding: " << padding << std::endl
            << std::endl;

  // for (int idx = 0; idx < file_count; idx++) {
  //   // 32 was originally 0x20
  //   file.seekg(base_offset + 32 + (idx * 32), file.beg);

  //   // TODO: Same for this
  //   std::string file_signature = read_string(file, 4);
  //   uint32_t unknown_0 = read_uint32(file);
  //   uint32_t decompressed_length = read_uint32(file);
  //   uint32_t decompressed_offset = read_uint32(file) + base_offset;
  //   uint32_t unknown_1 = read_uint32(file);
  //   uint32_t flags = read_uint32(file);
  //   uint32_t compressed_length = read_uint32(file);
  //   uint32_t compressed_offset = read_uint32(file) + base_offset;
  //   bool is_compressed = (flags & 1) != 0;

  //   file.seekg(string_pointer_offset + (idx * 4), file.beg);  // idx # bytes
  //   uint32_t file_name_length = read_uint32(file);
  //   std::string file_name = read_null_terminated_string(
  //       file,
  //       string_table_offset + file_name_length);  // TODO: replace with
  //                                                 // file.get

  //   std::cout << std::hex << "  0x" << base_offset + 32 + (idx * 32) <<
  //   std::dec
  //             << std::endl
  //             << "  File signature: " << file_signature << std::endl
  //             << "  Unknown_0: " << unknown_0 << std::endl
  //             << "  Decompressed Length: " << decompressed_length <<
  //             std::endl
  //             << "  Decompressed Offset: " << decompressed_offset <<
  //             std::endl
  //             << "  Unknown_1: " << unknown_1 << std::endl
  //             << "  Flags: " << std::bitset<32>(flags) << std::endl
  //             << "  Compressed Length: " << compressed_length << std::endl
  //             << "  Compressed Offset: " << compressed_offset << std::endl
  //             << "  Is Compressed: " << std::boolalpha << is_compressed
  //             << std::endl
  //             << std::hex << "  0x" << string_pointer_offset + (idx * 4)
  //             << std::dec << std::endl
  //             << "  Filename Length: " << file_name_length << std::endl
  //             << "  Filename: " << file_name << std::endl
  //             << std::endl;

  //   //   if (is_compressed && compressed_length > 0) {
  //   //     std::cout << set_color(Color::WHITE) << "[Extracting compressed
  //   file
  //   //     \""
  //   //               << file_name << "\"..." << set_color() << std::endl;

  //   //     file.seekg(compressed_offset, file.beg);
  //   //     char buffer[compressed_length];
  //   //     file.read(buffer, compressed_length);

  //   //     // TODO: dump to file here
  //   //   } else {
  //   //     file.seekg(decompressed_offset, file.beg);

  //   //     // TODO: SERI Reading stuff here
  //   //   }
  // }

  file.seekg(base_offset + compressed_section_length, file.beg);
  return static_cast<int>(file_count);
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
              << " at offset 0x" << std::setfill('0') << std::setw(8)
              << std::uppercase << std::hex << file.tellg() << std::dec << "]"
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
            << "Version 0.1.0" << set_color() << std::endl
            << std::endl;

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
