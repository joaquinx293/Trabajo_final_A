#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <set>  
#include <cstdlib> 
#include <ctime>    


/// agregar prints para ver el camino atomar 
class NeedlemanWunsch {

private:
    std::string sequence1, sequence2;
    std::vector<std::vector<int>> scoreMatrix;
    std::map<std::pair<char, char>, int> matchMatrix;
    int gapPenalty =-2;
    
 void initializeScoreMatrix() {
    int rows = sequence1.length() + 1;
    int cols = sequence2.length() + 1;
    
    scoreMatrix.resize(rows, std::vector<int>(cols, 0));
    
    // Inicializa la primera columna (con valores de penalización de gaps)
    for(int i = 1; i < rows; i++) {
        scoreMatrix[i][0] = scoreMatrix[i-1][0] + gapPenalty;
    }
    
    // Inicializa la primera fila (con valores de penalización de gaps)
    for(int j = 1; j < cols; j++) {
        scoreMatrix[0][j] = scoreMatrix[0][j-1] + gapPenalty;
    }
}

void printPathOnMatrix(const std::vector<std::pair<int, int>>& path) const {
    // Crear una copia de la matriz para mostrar el camino
    std::vector<std::vector<std::string>> visualMatrix(scoreMatrix.size(),
                                                       std::vector<std::string>(scoreMatrix[0].size(), " "));

    // Llenar la matriz con los valores de scoreMatrix
    for (size_t i = 0; i < scoreMatrix.size()-1; ++i) {
        for (size_t j = 0; j < scoreMatrix[i].size()-1; ++j) {
            visualMatrix[i][j] = std::to_string(scoreMatrix[i][j]);
        }
    }

    // Marcar el camino con un asterisco
    for (const auto& [row, col] : path) {
        visualMatrix[row][col] = "*";
    }

    // Imprimir las etiquetas de las columnas (sequence2)
    std::cout << " \t";
    for (char c : sequence2) {
        std::cout << c << "\t";
    }
    std::cout << std::endl;

    // Imprimir la matriz visual con etiquetas de filas (sequence1)
    for (size_t i = 0; i < visualMatrix.size(); ++i) {
        if (i > 0) 
            std::cout << sequence1[i - 1] << "\t";  // Etiqueta de fila
        else 
            std::cout << " \t";

        for (const auto& cell : visualMatrix[i]) {
            std::cout << cell << "\t";
        }
        std::cout << std::endl;
    }
}

    
    void fillScoreMatrix() {
    for (int i = 1; i <= sequence1.length(); i++) {
        for (int j = 1; j <= sequence2.length(); j++) {
            
            int match_mismatch = scoreMatrix[i - 1][j - 1] + (sequence1[i - 1] == sequence2[j - 1] ? 1 : -1); 
            int gap_sequence2 = scoreMatrix[i - 1][j] + gapPenalty;
            int gap_sequence1 = scoreMatrix[i][j - 1] + gapPenalty;
            
            scoreMatrix[i][j] = std::max(match_mismatch,std::max(gap_sequence2, gap_sequence1));
        }
    }
}
   std::pair<std::string, std::string> align() {
    std::string aligned1, aligned2;
    int i = sequence1.length();
    int j = sequence2.length();
    
    // Vector para almacenar el camino recorrido
    std::vector<std::pair<int, int>> path;

    while (i > 0 || j > 0) {
     // Almacenar la posición actual en el camino
        path.emplace_back(i, j); 

        if (i > 0 && j > 0 && scoreMatrix[i][j] == scoreMatrix[i-1][j-1] + (sequence1[i-1] == sequence2[j-1] ? 1 : -1)) {
            aligned1 = sequence1[i-1] + aligned1;
            aligned2 = sequence2[j-1] + aligned2;
            i--; j--;
        } else if (i > 0 && scoreMatrix[i][j] == scoreMatrix[i-1][j] + gapPenalty) {
            aligned1 = sequence1[i-1] + aligned1;
            aligned2 = '-' + aligned2;
            i--;
        } else {
            aligned1 = '-' + aligned1;
            aligned2 = sequence2[j-1] + aligned2;
            j--;
        }
    }

    path.emplace_back(i, j); // Añadir la posición inicial (0, 0)

    // Imprimir el camino en forma de matriz
    std::cout << "\nCamino recorrido (Visualizado en la matriz):\n";
    printPathOnMatrix(path);

    return {aligned1, aligned2};
}


public:
    NeedlemanWunsch(const std::string& seq1, const std::string& seq2, int gap_penalty) 
        : sequence1(seq1), sequence2(seq2), gapPenalty(gap_penalty) {}
    
    void loadMatchMatrix(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo de matriz de emparejamiento");
        }
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            char c1, c2;
            int score;
            if (iss >> c1 >> c2 >> score) {
                matchMatrix[{c1, c2}] = score;
                matchMatrix[{c2, c1}] = score;  
            }
        }
    }

    void generateDotFile(const std::string& aligned1, const std::string& aligned2, 
                         const std::string& outputFile) {
        std::ofstream file(outputFile);
        file << "digraph alignment {\n";
        file << "    node [shape=plaintext];\n";
        file << "    alignment [label=<\n";
        file << "        <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
        file << "        <TR>\n";
        for(char c : aligned1) {
            file << "            <TD>" << c << "</TD>\n";
        }
        file << "        </TR>\n";
        file << "        <TR>\n";
        for(char c : aligned2) {
            file << "            <TD>" << c << "</TD>\n";
        }
        file << "        </TR>\n";
        file << "        </TABLE>\n";
        file << "    >];\n";
        file << "}\n";
        
        file.close();
    }

    int getFinalScore() const {
        return scoreMatrix[sequence1.length()][sequence2.length()];
    }

    void alignAndPrint() {
        // Llenar la matriz de puntuación
        initializeScoreMatrix();
        fillScoreMatrix();

        // Imprime la matriz de puntuación (programación dinámica)
        std::cout << "Matriz de programación dinámica:\n";
        printLabeledMatrix();

        // Obtén el alineamiento óptimo a partir de la matriz
        auto [aligned1, aligned2] = align();  // Usamos la función ya implementada 'align'

        // Imprime el alineamiento óptimo
        std::cout << "\nAlineamiento óptimo:\n" << aligned1 << "\n" << aligned2 << std::endl;

        // Imprime el puntaje final
        std::cout << "Puntaje final: " << getFinalScore() << std::endl;

        // Opcional: exporta el alineamiento a un archivo
        generateDotFile(aligned1, aligned2, "resultado.dot");
        if (system("dot -Tpng resultado.dot -o resultado.png") != 0) {
            std::cerr << "Error al generar la imagen con Graphviz.\n";
        }
    }

   void printLabeledMatrix() const {
    // Imprime el encabezado con los caracteres de sequence2
    std::cout << "\t";  
    for (char c : sequence2) {
        std::cout << c << "\t";
    }
    std::cout << std::endl;

    // Imprime las filas de la matriz con las letras de sequence1
    for (size_t i = 0; i < sequence1.length(); ++i) {  
        if (i > 0) 
            std::cout << sequence1[i - 1] << "\t";  
        else 
            std::cout << " " << "\t";  

        // Imprime los valores de la matriz, excluyendo la fila adicional innecesaria
        for (size_t j = 0; j < sequence2.length(); ++j) {  
            std::cout << scoreMatrix[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}


};

// Función main para probar el código
int main(int argc, char* argv[]) {
    if (argc < 9) {
        std::cerr << "Uso: ./programa -C1 <archivo1> -C2 <archivo2> -U <archivo_matriz> -V <gap_penalty>\n";
        return 1;
    }
    
    std::string seq1File, seq2File, matchMatrixFile;
    int gapPenalty = 0;
    for (int i = 1; i < argc; i += 2) {
        std::string arg = argv[i];
        if (arg == "-C1") seq1File = argv[i+1];
        else if (arg == "-C2") seq2File = argv[i+1];
        else if (arg == "-U") matchMatrixFile = argv[i+1];
        else if (arg == "-V") {
            try {
                gapPenalty = std::stoi(argv[i+1]);
            } catch (...) {
                std::cerr << "Error: El valor de la penalización debe ser un número entero.\n";
                return 1;
            }
        }
    }


    // Cargar secuencias
    std::ifstream file1(seq1File), file2(seq2File);
    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Error al abrir los archivos de secuencias.\n";
        return 1;
    }
    std::string sequence1((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
    std::string sequence2((std::istreambuf_iterator<char>(file2)), std::istreambuf_iterator<char>());
    
    // Crear el objeto NeedlemanWunsch
    NeedlemanWunsch nw(sequence1, sequence2, gapPenalty);
    
    // Cargar la matriz de emparejamiento
    nw.loadMatchMatrix(matchMatrixFile);
    
    // Realizar el alineamiento y mostrar los resultados
    nw.alignAndPrint();
    
    return 0;
}
