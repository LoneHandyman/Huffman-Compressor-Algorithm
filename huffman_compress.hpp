#ifndef HUFFMAN_COMPRESS_HPP_
#define HUFFMAN_COMPRESS_HPP_

#include <string>
#include <iostream>
#include <vector>

namespace pac{

  const std::string compress(std::istream& rawContent);
  const std::string decompress(std::istream& eContent);

}

#endif//HUFFMAN_COMPRESS_HPP_