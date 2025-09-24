#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

void Transposition(const vector<vector<int>>& matriz, vector<vector<int>>& temp);

int main(){
    int width, height, channels;
    unsigned char* img = stbi_load("key.png", &width, &height, &channels, 0);  // Load as RGB

    if (img == nullptr) {
        cerr << "Failed to load image!" << std::endl;
        return -1;
    }

    int region_size = 16;
    vector<vector<int>> pixel_keys(region_size, vector<int>(region_size));

    for (int i = 0; i < region_size; ++i) {
        for (int j = 0; j < region_size; ++j) {
            if (i < height && j < width) {
            int idx = (i * width + j) * channels;

            int r = img[idx];
            int g = img[idx + 1];
            int b = img[idx + 2];

            pixel_keys[i][j] = (r ^ g ^ b);
            }
        }
    }
    stbi_image_free(img);

    ifstream inputFile("plaintext.txt");
    if(!inputFile.is_open()){
        cerr << "Error opening the file!" << endl;
        return -1; 
    }
    string line;
    getline(inputFile, line);
    inputFile.close();

    int blockSize = 16;  // 4x4 = 16
    int matrixSize = 4;

    int numBlocks = (line.size() + blockSize - 1) / blockSize;

    vector<vector<vector<int>>> matrices(numBlocks, vector<vector<int>>(matrixSize, vector<int>(matrixSize, 0)));

    int index = 0;
    for (int b = 0; b < numBlocks; b++) {
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                if (index < line.size()) {
                    matrices[b][i][j] = static_cast<int>(line[index++]);
                } else {
                    matrices[b][i][j] = 0; 
                }
            }
        }
    }

    //encription loop
    for(int i=0; i<region_size; i++){
        //xor all matrix with key[i]
        for(int j=0; j<matrices.size();j++){ //for every matrix
            for(int k=0; k<matrices[j].size(); k++){ //for every row
                for(int l=0; l<matrices[j][k].size(); l++){ //for every point at row
                    matrices[j][k][l] ^= pixel_keys[i][k*matrixSize + l]; //xor by key
                    
                    //difuse
                    //bit shifting
                    matrices[j][k][l] = (matrices[j][k][l] >> 1) | ((matrices[j][k][l] & 0x01) << 7);
                }
            }
        }
        
        //transposition
        vector<vector<int>> temp(matrixSize, vector<int>(matrixSize, 0));
        for(int j=0; j<matrices.size(); j++){
            Transposition(matrices[j], temp);
            matrices[j] = temp;
        }
    }

    //ofstream
    ofstream outputFile("cryptext.txt");
    if (outputFile.is_open()) {
        for(int j=0; j<matrices.size();j++){ //for every matrix
            for(int k=0; k<matrices[j].size(); k++){ //for every row
                for(int l=0; l<matrices[j][k].size(); l++){
                    outputFile << (matrices[j][k][l]);
                }
            }
        }
    } else {
        cerr << "Error writing the file!" << endl;
        return -1; 
    }
    outputFile.close();
}

void Transposition(const vector<vector<int>>& matriz, vector<vector<int>>& temp){
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            temp[j][i] = matriz[i][j];
        }
    }
}