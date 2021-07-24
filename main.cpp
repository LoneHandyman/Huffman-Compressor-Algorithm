#include "huffman_compress.hpp"
#include <fstream>

int main(){
  std::ifstream file("cpp.ico", std::ios::binary);
  if(file.is_open()){
    std::string result = pac::compress(file);
    file.close();
    std::ofstream out("cpp.dat", std::ios::binary);
    if(out.is_open()){
      out.write(result.c_str(), result.length());
      out.close();
    }
  }
  file.open("cpp.dat", std::ios::binary);
  if(file.is_open()){
    std::string decoded = pac::decompress(file);
    file.close();
    std::ofstream out("cpp_copy.ico", std::ios::binary);
    if(out.is_open()){
      out.write(decoded.c_str(), decoded.length());
      out.close();
    }
  }
  return 0;
}