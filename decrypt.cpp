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

    ifstream in("cryptext.bin", ios::binary);
    if(!in.is_open()){ cerr << "Error opening cryptext.bin\n"; return -1; }

    uint32_t plain_len = 0;
    in.read(reinterpret_cast<char*>(&plain_len), sizeof(uint32_t));
    if(!in){ cerr << "Malformed file: cannot read header\n"; return -1; }

    vector<uint8_t> cipher((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
    in.close();

    if (cipher.empty() || (cipher.size() % 16) != 0){
        cerr << "Malformed cipher size (must be multiple of 16)\n";
        return -1;
    }

    const int matrixSize = 4;
    const int blockSize  = 16;
    int numBlocks = (int)(cipher.size() / blockSize);

    vector<vector<vector<uint8_t>>> matrices(numBlocks, vector<vector<uint8_t>>(matrixSize, vector<uint8_t>(matrixSize, 0)));

    size_t idx = 0;
    for (int b = 0; b < numBlocks; b++)
        for (int i = 0; i < matrixSize; i++)
            for (int j = 0; j < matrixSize; j++)
                matrices[b][i][j] = cipher[idx++];

    // rounds en orden inverso: 15 -> 0
    for (int i = region_size - 1; i >= 0; --i){
        // invers transp
        vector<vector<uint8_t>> temp(matrixSize, vector<uint8_t>(matrixSize, 0));
        for (int b = 0; b < numBlocks; b++){
            Transposition(matrices[b], temp);
            matrices[b] = temp;
        }


        for (int b = 0; b < numBlocks; b++){
            for (int r = 0; r < matrixSize; r++){
                for (int c = 0; c < matrixSize; c++){
                    uint8_t v = matrices[b][r][c];

                    v = (uint8_t)(((v << 1) & 0xFF) | ((v >> 7) & 0x01));
                    v ^= pixel_keys[i][r * matrixSize + c];
                    matrices[b][r][c] = v;
                }
            }
        }
    }
    //ofstream
    ofstream out("decrypt.txt", ios::binary);
    if(!out.is_open()){ cerr << "Error writing decrypt.txt\n"; return -1; }

    size_t written = 0;
    for (int b = 0; b < numBlocks && written < plain_len; b++){
        for (int i = 0; i < matrixSize && written < plain_len; i++){
            for (int j = 0; j < matrixSize && written < plain_len; j++){
                char ch = (char)matrices[b][i][j];
                out.write(&ch, 1);
                ++written;
            }
        }
    }
    out.close();
    return 0;
}
