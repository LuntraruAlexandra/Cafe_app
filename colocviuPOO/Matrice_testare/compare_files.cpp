#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

// Function to read lines from a CSV file into a vector of strings
std::vector<std::string> read_csv_lines(const fs::path& filepath) {
    std::vector<std::string> lines;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filepath.string());
    }

    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// Function to compare two files line by line
void compare_files(const fs::path& file1, const fs::path& file2) {
    std::vector<std::string> lines1 = read_csv_lines(file1);
    std::vector<std::string> lines2 = read_csv_lines(file2);

    size_t max_lines = std::max(lines1.size(), lines2.size());
    for (size_t i = 0; i < max_lines; ++i) {
        std::string line1 = (i < lines1.size()) ? lines1[i] : "[NO LINE]";
        std::string line2 = (i < lines2.size()) ? lines2[i] : "[NO LINE]";

        if (line1 != line2) {
            std::cout << "Difference found in " << file1 << " and " << file2 << " at line " << (i + 1) << ":\n";
            std::cout << "File 1: " << line1 << "\n";
            std::cout << "File 2: " << line2 << "\n\n";
        }
    }
}

// Function to recursively compare all CSV files in two directory hierarchies
void compare_directories(const fs::path& dir1, const fs::path& dir2) {
    for (const auto& entry : fs::recursive_directory_iterator(dir1)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == ".csv") {
            auto relative_path = fs::relative(entry.path(), dir1);
            auto corresponding_file = dir2 / relative_path;

            if (fs::exists(corresponding_file)) {
                compare_files(entry.path(), corresponding_file);
            } else {
                std::cout << "File missing in " << dir2 << ": " << relative_path << "\n";
            }
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        // Define the paths to compare relative to "Matrice_testare"
        fs::path dir1 = argv[1];
        fs::path dir2 = argv[2];

        std::cout << "Comparing CSV files between '" << dir1 << "' and '" << dir2 << "'...\n";

        compare_directories(dir1, dir2);

        std::cout << "Comparison completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
