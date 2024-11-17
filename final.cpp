#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <sstream>
#include <set>  // Asegúrate de incluir <set>
#include <cstdlib>  // Para rand() y srand()
#include <ctime>    // Para time()

class NeedlemanWunsch {
private:
    std::string sequence1, sequence2;
    std::vector<std::vector<int>> scoreMatrix;
    std::map<std::pair<char, char>, int> matchMatrix;
    int gapPenalty;
    
    void initializeScoreMatrix() {
        int rows = sequence1.length() + 1;
        int cols = sequence2.length() + 1;
        
        scoreMatrix.resize(rows, std::vector<int>(cols, 0));
    
        for(int i = 1; i < rows; i++) {
            scoreMatrix[i][0] = scoreMatrix[i-1][0] + gapPenalty;
        }
        for(int j = 1; j < cols; j++) {
            scoreMatrix[0][j] = scoreMatrix[0][j-1] + gapPenalty;
        }
    }
    
    void fillScoreMatrix() {
        for(int i = 1; i <= sequence1.length(); i++) {
            for(int j = 1; j <= sequence2.length(); j++) {
                int match = scoreMatrix[i-1][j-1] + 
                    matchMatrix[{sequence1[i-1], sequence2[j-1]}];
                int delete_gap = scoreMatrix[i-1][j] + gapPenalty;
                int insert_gap = scoreMatrix[i][j-1] + gapPenalty;
                
                // Modificación aquí:
                scoreMatrix[i][j] = std::max(match, std::max(delete_gap, insert_gap));
            }
        }
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
                matchMatrix[{c2, c1}] = score;  // Aseguramos que la relación sea simétrica
                std::cout << "Leído: " << c1 << " " << c2 << " " << score << "\n";  // Para depuración
            }
        }
    }

    std::pair<std::string, std::string> align() {
        initializeScoreMatrix();
        fillScoreMatrix();
        std::string aligned1, aligned2;
        int i = sequence1.length();
        int j = sequence2.length();
        while (i > 0 || j > 0) {
            if (i > 0 && j > 0 && scoreMatrix[i][j] == scoreMatrix[i-1][j-1] + 
                matchMatrix[{sequence1[i-1], sequence2[j-1]}]) {
                aligned1 = sequence1[i-1] + aligned1;
                aligned2 = sequence2[j-1] + aligned2;
                i--; j--;
            }
            else if (i > 0 && scoreMatrix[i][j] == scoreMatrix[i-1][j] + gapPenalty) {
                aligned1 = sequence1[i-1] + aligned1;
                aligned2 = '-' + aligned2;
                i--;
            }
            else {
                aligned1 = '-' + aligned1;
                aligned2 = sequence2[j-1] + aligned2;
                j--;
            }
        }
        
        return {aligned1, aligned2};
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

    // Nueva función para escribir la matriz de emparejamiento con puntajes aleatorios
    void generatePairwiseScoresFile(const std::string& outputFile) {
        std::ofstream file(outputFile);

        // Inicializa la semilla de rand() con el tiempo actual para obtener resultados aleatorios diferentes
        srand(static_cast<unsigned>(time(0)));

        // Usar un conjunto para evitar duplicados, asegurándonos de manejar (A, G) igual que (G, A)
        std::set<std::pair<char, char>> processedPairs;

        // Función para generar puntajes aleatorios entre -3 y 3
        auto generateRandomScore = []() -> int {
            return rand() % 7 - 9;  // Genera un número entre -3 y 3
        };

        // Iterar sobre todas las combinaciones posibles de caracteres en sequence1 y sequence2
        for (char c1 : sequence1) {
            for (char c2 : sequence2) {
                std::pair<char, char> pair = {std::min(c1, c2), std::max(c1, c2)};
                
                // Verificar si ya se procesó el par
                if (processedPairs.find(pair) == processedPairs.end()) {
                    int scoreValue = generateRandomScore();
                    file << pair.first << " " << pair.second << " " << scoreValue << "\n";
                    processedPairs.insert(pair);
                }
            }
        }

        file.close();
    }

    int getFinalScore() const {
        return scoreMatrix[sequence1.length()][sequence2.length()];
    }
};

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
                std::cerr << "Error: el valor de gap penalty no es válido.\n";
                return 1;
            }
        }
    }

    std::ifstream file1(seq1File), file2(seq2File);
    if (!file1.is_open() || !file2.is_open()) {
        std::cerr << "Error al abrir los archivos de las cadenas.\n";
        return 1;
    }

    std::string sequence1, sequence2;
    std::getline(file1, sequence1);
    std::getline(file2, sequence2);

    // Imprimir secuencias leídas para depuración
    std::cout << "Secuencia 1: " << sequence1 << "\n";
    std::cout << "Secuencia 2: " << sequence2 << "\n";

    try {
        NeedlemanWunsch nw(sequence1, sequence2, gapPenalty);
        nw.loadMatchMatrix(matchMatrixFile);

        auto [aligned1, aligned2] = nw.align();

        std::cout << "Puntaje final: " << nw.getFinalScore() << "\n";
        std::cout << "Alineamiento:\n" << aligned1 << "\n" << aligned2 << "\n";

        nw.generateDotFile(aligned1, aligned2, "alignment.dot");

        if (system("dot -Tpng alignment.dot -o alignment.png") != 0) {
            std::cerr << "Error al generar la imagen con Graphviz.\n";
        }

        // Genera el archivo con los pares de nucleótidos y sus puntajes aleatorios
        nw.generatePairwiseScoresFile("funU.tex");

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
