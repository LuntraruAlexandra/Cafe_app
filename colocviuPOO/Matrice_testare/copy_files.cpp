#include <iostream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

void copy_directory(const fs::path& source, const fs::path& destination) {
    if (!fs::exists(source) || !fs::is_directory(source)) {
        throw std::runtime_error("Source directory does not exist or is not a directory.");
    }

    fs::create_directories(destination);

    for (const auto& entry : fs::recursive_directory_iterator(source)) {
        const auto& path_in_source = entry.path();
        auto relative_path = fs::relative(path_in_source, source);
        auto path_in_destination = destination / relative_path;

        if (fs::is_directory(path_in_source)) {
            fs::create_directories(path_in_destination);
        } else if (fs::is_regular_file(path_in_source)) {
            fs::copy_file(path_in_source, path_in_destination, fs::copy_options::overwrite_existing);
        }
    }
}

int main(int argc, char *argv[]) {
    try {
        // Adjust paths to work relative to the current directory of "Matrice_testare"
        fs::path source = argv[1]; 
        fs::path destination = argv[2]; 

        std::cout << "Copying data from '" << source << "' to '" << destination << "'..." << std::endl;

        copy_directory(source, destination);

        std::cout << "Copy completed successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
