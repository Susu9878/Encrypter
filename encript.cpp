#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

void Transposition(const vector<vector<uint8_t>>& matriz, vector<vector<uint8_t>>& temp){
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            temp[j][i] = matriz[i][j];
}

int main(){
    int width, height, channels;
    unsigned char* img = stbi_load("key.png", &width, &height, &channels, 3);
    if (img == nullptr) { cerr << "Failed to load image!\n"; return -1; }

    const int region_size = 16;
    vector<vector<uint8_t>> pixel_keys(region_size, vector<uint8_t>(region_size, 0));

    for (int i = 0; i < region_size; ++i){
        for (int j = 0; j < region_size; ++j){
            if (i < height && j < width){
                int idx = (i * width + j) * 3;
                uint8_t r = img[idx], g = img[idx+1], b = img[idx+2];
                pixel_keys[i][j] = (uint8_t)(r ^ g ^ b);
            }
        }
    }
    stbi_image_free(img);

    ifstream inputFile("plaintext.txt", ios::binary);
    if(!inputFile.is_open()){ cerr << "Error opening plaintext.txt\n"; return -1; }
    string plain((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());
    inputFile.close();
    uint32_t plain_len = (uint32_t)plain.size();

    const int matrixSize = 4;
    const int blockSize  = 16;
    int numBlocks = (plain.size() + blockSize - 1) / blockSize;

    vector<vector<vector<uint8_t>>> matrices(numBlocks, vector<vector<uint8_t>>(matrixSize, vector<uint8_t>(matrixSize, 0)));

    size_t index = 0;
    for (int b = 0; b < numBlocks; b++){
        for (int i = 0; i < matrixSize; i++){
            for (int j = 0; j < matrixSize; j++){
                matrices[b][i][j] = (index < plain.size()) ? (uint8_t)plain[index++] : 0u;
            }
        }
    }


    for(int i = 0; i < region_size; i++){

        for (int b = 0; b < numBlocks; b++){
            for (int r = 0; r < matrixSize; r++){
                for (int c = 0; c < matrixSize; c++){
                    uint8_t v = matrices[b][r][c];
                    v ^= pixel_keys[i][r * matrixSize + c];

                    v = (uint8_t)((v >> 1) | ((v & 0x01) << 7));
                    matrices[b][r][c] = v;
                }
            }
        }
        // Transpo
        vector<vector<uint8_t>> temp(matrixSize, vector<uint8_t>(matrixSize, 0));
        for (int b = 0; b < numBlocks; b++){
            Transposition(matrices[b], temp);
            matrices[b] = temp;
        }
    }

    ofstream out("cryptext.bin", ios::binary);
    if(!out.is_open()){ cerr << "Error writing cryptext.bin\n"; return -1; }
    out.write(reinterpret_cast<const char*>(&plain_len), sizeof(uint32_t));
    for (int b = 0; b < numBlocks; b++)
        for (int i = 0; i < matrixSize; i++)
            for (int j = 0; j < matrixSize; j++){
                char byte = (char)matrices[b][i][j];
                out.write(&byte, 1);
            }
    out.close();
    return 0;
}
