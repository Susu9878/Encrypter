#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

int main() {
    ifstream file("plaintext.txt");  // abrir el archivo
    if (!file) {
        cerr << "Error: no se pudo abrir plaintext.txt" << endl;
        return 1;
    }

    string text;
    getline(file, text);  // leer la línea completa del archivo
    file.close();

    int blockSize = 16;  // 4x4 = 16
    int matrixSize = 4;

    // Calcular cuántos bloques necesitas
    int numBlocks = (text.size() + blockSize - 1) / blockSize;

    // Crear las matrices
    vector<vector<vector<char>>> matrices(numBlocks, vector<vector<char>>(matrixSize, vector<char>(matrixSize, ' ')));

    int idx = 0;
    for (int b = 0; b < numBlocks; b++) {
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                if (idx < text.size()) {
                    matrices[b][i][j] = text[idx++];
                } else {
                    matrices[b][i][j] = '_'; // relleno si falta
                }
            }
        }
    }

    // Imprimir las matrices
    for (int b = 0; b < numBlocks; b++) {
        cout << "Matrix " << b+1 << ":" << endl;
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                cout << matrices[b][i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    }

    return 0;
}