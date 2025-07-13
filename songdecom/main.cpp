#include <iostream>
#include <fstream>
#include <sav.h>    // Your LSDJ headers
#include <cstring>

int main(int argc, char *argv[])
{
    if (argc != 3)  // Now 2 args: input file and output bin file
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>\n";
        return 1;
    }

    const char *inputFile = argv[1];
    const char *outputFile = argv[2];

    lsdj_sav_t *sav = nullptr;

    // Read the .sav file into sav pointer
    if (lsdj_sav_read_from_file(inputFile, &sav, nullptr) != 0)
    {
        std::cerr << "Failed to read input file: " << inputFile << "\n";
        return 1;
    }

    const lsdj_song_t *song = lsdj_sav_get_working_memory_song_const(sav);
    if (!song)
    {
        std::cerr << "Failed to get song from sav\n";
        lsdj_sav_free(sav);
        return 1;
    }

    // Get size of lsdj_song_t to know how many bytes to write
    size_t songSize = sizeof(lsdj_song_t);

    // Open binary file for writing
    std::ofstream out(outputFile, std::ios::binary);
    if (!out)
    {
        std::cerr << "Failed to open output file: " << outputFile << "\n";
        lsdj_sav_free(sav);
        return 1;
    }

    // Write the raw bytes of the song struct into the file
    out.write(reinterpret_cast<const char*>(song), songSize);

    out.close();

    lsdj_sav_free(sav);

    std::cout << "Song dumped to binary file: " << outputFile << "\n";

    return 0;
}
