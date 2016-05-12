//
//  BitString.h
//

#ifndef Trie_Data_Structure_BitString_h
#define Trie_Data_Structure_BitString_h

#include <vector>
#include <string>
#include <iostream>
#include <numeric>

class BitString {
public:
    
    // Constructor
    BitString();
    BitString(const BitString& copy);
    BitString(const std::string& arg);
    BitString(const char* arg);
    BitString(const uint8_t* prefix, const int width);
    BitString(std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last);
    
    // Operator Overloading
    BitString& operator=(const std::string& arg);
    BitString& operator=(const char* arg);
    
    BitString operator+(const BitString& arg);
    BitString operator+=(const BitString& arg);
    BitString operator+=(const char& arg);
    
    bool operator==(const std::string& arg);
    bool operator==(const char* arg);
    bool operator==(BitString& arg);
    bool operator==(const BitString& arg);
    
    bool operator!=(const std::string& arg);
    bool operator!=(const char* arg);
    bool operator!=(BitString& arg);

    std::vector<bool>::reference operator[](std::size_t idx);
    std::vector<bool>::const_reference operator[](std::size_t idx) const;

    // Convert Bit String to Int
    unsigned int toInt() const;
    
    // Convert Bit String to String
    std::string toString() const;
    
    // Convert Bit String to uint8_t ip address
    // Returns an array with 4 elements. Each element represents an 8-bit segment of the ipv4 address
    uint8_t* toUInt8(int *width) const;  // TODO: Make Generic and return width
    
    // Size
    int size() const;
    int length() const;
    
    // Push Back
    void push_back(bool arg);
    
    // Pop Back
    void pop_back();
    
    // Substring
    BitString substr(int startPos, int length = -1) const;
    
    // Append
    BitString append(const BitString& arg);
    
    // Compare
    int compare(const BitString& arg) const;
    
    // Find
    int find(const BitString& arg) const;
    
    // Getter
    std::vector<bool> getVector() const;
    
    // Convert string to Bit String
    static BitString stringToBitString(std::string arg);
    
    // Convert int to Bit String
    static BitString intToBitString(unsigned int arg, int numOfBits);
    
private:
    
    std::vector<bool> data; // Vector Container
    
};

// Stream Operator Overloading
std::ostream& operator<<(std::ostream& os, const BitString& obj);

// Defining std::hash<BitString>
namespace std {
    template <> struct hash<BitString>
    {
        size_t operator()(const BitString & x) const
        {
            std::hash<string> stringHash;
            return stringHash(x.toString());
        }
    };
}


#endif
