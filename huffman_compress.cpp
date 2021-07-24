#include "huffman_compress.hpp"
//=============================
#include <bitset>
#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <utility>
#include <vector>
//=============================

struct HuffmanTree{
  uint8_t value;
  std::shared_ptr<HuffmanTree> left, right;
  uint64_t frequency;

  bool leafFlag;
  HuffmanTree(uint8_t v, uint64_t f, std::shared_ptr<HuffmanTree> l, std::shared_ptr<HuffmanTree> r):
    value(v), frequency(f), left(l), right(r) {
    if(l == nullptr && r == nullptr)
      leafFlag = true;
    else
      leafFlag = false;
  }

  void traverse(std::map<uint8_t, std::pair<uint64_t, std::string>>& table, const std::string& code = ""){
    if(leafFlag){
      table.insert({value, {frequency, code}});
      return;
    }
    left->traverse(table, code + "0");
    right->traverse(table, code + "1");
  }

  bool read(std::shared_ptr<HuffmanTree>& query, const bool& way){
    if(way)
      query = right;
    else
      query = left;
    return query->leafFlag;
  }
};

bool huffmanTreeComparator(const std::shared_ptr<HuffmanTree>& a, const std::shared_ptr<HuffmanTree>& b){
  return a->frequency > b->frequency;
}

const std::string pac::compress(std::istream& rawContent){
  auto htcomparator = huffmanTreeComparator;
  std::priority_queue<std::shared_ptr<HuffmanTree>, 
                      std::vector<std::shared_ptr<HuffmanTree>>,
                      decltype(htcomparator)> huffmanTreeBuilder(htcomparator);
  std::shared_ptr<HuffmanTree> left, right, root;
  uint64_t countChar[256] = {0}, sum = 0;
  int32_t byte = 0;
  while(!rawContent.eof()){
    byte = rawContent.get();
    if(byte != -1)
      ++countChar[(uint8_t)byte];
  }
  for(uint16_t idx = 0; idx < 256; ++idx){
    if(countChar[idx] > 0){
      huffmanTreeBuilder.push(std::make_shared<HuffmanTree>((uint8_t)idx, countChar[idx], nullptr, nullptr));
      sum += countChar[idx];
    }
  }
  std::cout << sum << std::endl;
  while(huffmanTreeBuilder.size() != 1){
    left = huffmanTreeBuilder.top();
    huffmanTreeBuilder.pop();
    right = huffmanTreeBuilder.top();
    huffmanTreeBuilder.pop();
    root = std::make_shared<HuffmanTree>(0, left->frequency + right->frequency, left, right);
    huffmanTreeBuilder.push(root);
  }
  std::map<uint8_t, std::pair<uint64_t, std::string>> huffmanCoding;
  root->traverse(huffmanCoding);
  rawContent.clear();
  rawContent.seekg(rawContent.beg);
  std::string binaryCompress, asciiCompress, codingTable;
  while(!rawContent.eof()){
    byte = rawContent.get();
    if(byte != -1){
      std::string code = huffmanCoding[(uint8_t)byte].second;
      for(std::size_t idx = 0; idx < code.length(); ++idx)
        binaryCompress.push_back(code[idx]);
    }
  }
  for(std::size_t offset = 0; offset < binaryCompress.length(); offset += 8)
    asciiCompress.push_back((char)std::bitset<8>(binaryCompress.substr(offset, 8)).to_ulong());
  uint16_t padding = 8 - (binaryCompress.length() % 8);
  binaryCompress += std::string().assign(padding, '0');
  codingTable += std::to_string(padding);
  std::stringstream hexFormat;
  for(auto& row : huffmanCoding){
    hexFormat << std::hex << (uint32_t)row.first;
    if(hexFormat.str().length() != 2)
      codingTable += "0";
    codingTable += hexFormat.str();
    hexFormat.str("");
    hexFormat.clear();
    hexFormat << std::hex << row.second.first;
    codingTable += hexFormat.str() + ";";
    hexFormat.str("");
    hexFormat.clear();
  }
  codingTable.pop_back();
  codingTable += "|";
  codingTable.append(asciiCompress);
  return codingTable;
}

const std::string pac::decompress(std::istream& eContent){
  auto htcomparator = huffmanTreeComparator;
  std::priority_queue<std::shared_ptr<HuffmanTree>, 
                      std::vector<std::shared_ptr<HuffmanTree>>,
                      decltype(htcomparator)> huffmanTreeBuilder(htcomparator);
  std::shared_ptr<HuffmanTree> left, right, root, query, result;
  int32_t byte = 0, nextbyte = 0;
  std::string header, row;
  std::stringstream codingTable, hexFormat;
  while((byte = eContent.get()) != '|' && !eContent.eof()){
    header.push_back((char)byte);
  }
  codingTable << header;
  int16_t padding = codingTable.get() - '0', key;
  uint64_t frequency, sum = 0;
  while(std::getline(codingTable, row, ';')){
    hexFormat << row.substr(0, 2);
    hexFormat >> std::hex >> key;
    hexFormat.str("");
    hexFormat.clear();
    hexFormat << row.substr(2);
    hexFormat >> std::hex >> frequency;
    sum += frequency;
    hexFormat.str("");
    hexFormat.clear();
    huffmanTreeBuilder.push(std::make_shared<HuffmanTree>((uint8_t)key, frequency, nullptr, nullptr));
  }
  std::cout << sum << std::endl;
  while(huffmanTreeBuilder.size() != 1){
    left = huffmanTreeBuilder.top();
    huffmanTreeBuilder.pop();
    right = huffmanTreeBuilder.top();
    huffmanTreeBuilder.pop();
    root = std::make_shared<HuffmanTree>(0, left->frequency + right->frequency, left, right);
    huffmanTreeBuilder.push(root);
  }
  std::string decompressedData, binarySet;
  std::queue<bool> queryBand;
  query = root;
  while(!eContent.eof()){
    byte = eContent.get();
    if(byte != -1){
      binarySet = std::bitset<8>(byte).to_string();
      nextbyte = eContent.peek();
      if(nextbyte == -1)
       binarySet = binarySet.substr(0, binarySet.length() - padding);
      for(auto& bit : binarySet)
        queryBand.push(bit - '0');
      while(!queryBand.empty()){
        bool leafReached = query->read(result, queryBand.front());
        queryBand.pop();
        query = result;
        if(leafReached){
          decompressedData.push_back((char)query->value);
          query = root;
          if(nextbyte != -1)
            break;
        }
      }
    }
  }
  return decompressedData;
}