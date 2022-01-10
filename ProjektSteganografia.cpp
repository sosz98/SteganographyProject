#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <cmath>
#include <vector>
#include <typeinfo>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctime>

std::vector<int> getMessageInBits(const char*);

bool isFileFormatSupported(const std::string&);

void printProgramsManual();

std::string addMessageIdentifier(const char*);

void encryptMessageInFile(const std::string&, const char*);

void decryptFileHiddenMessage(const std::string&);

void checkIfFileIsEncrypted(const std::string&, const char*);

int returnFileFormatIdentifier(std::string&);

void printInformationAboutFile(std::string&);

void writeLastFileTimestamp(const char*);

void printBmpFileInformation(std::string&);

void printPpmFileInformation(std::string&);

int main(int argc, char** argv) {
    if (argc == 1)
        printProgramsManual();
    else if (argc == 2) {
        std::string first_argument = argv[1];
        if (first_argument == "-h" || first_argument == "--help")
            printProgramsManual();
        else
            std::cout << "Wrong arguments. Please try again or check \"-h\" flag\n";
    }
    else if (argc == 3) {
        std::string first_argument = argv[1];
        std::string second_argument = argv[2];
        if (first_argument == "-d" || first_argument == "--decrypt")
            decryptFileHiddenMessage(second_argument);
        else if (first_argument == "-i" || first_argument == "--info")
            printInformationAboutFile(second_argument);
        else
            std::cout << "Wrong arguments. Please try again or check \"-h\" flag\n";
    }
    else if (argc == 4) {
        std::string first_argument = argv[1];
        std::string second_argument = argv[2];
        std::string third_argument = argv[3];
        if (first_argument == "-c" || first_argument == "--check")
            checkIfFileIsEncrypted(second_argument, third_argument.c_str());
        else if (first_argument == "-e" || first_argument == "--encrypt")
            encryptMessageInFile(second_argument, third_argument.c_str());
        else
            std::cout << "Wrong arguments. Please try again or check \"-h\" flag\n";

    }
    else
        std::cout << "Wrong arguments. Please try again or check \"-h\" flag\n";
}

/**
 * Method checking if path leads to bmp or ppm file
 * @param path absolute path to the file
 * @return status of format correctness
 * */
bool isFileFormatSupported(const std::string& path) {

    std::ifstream file(path);
    uint8_t header[2];
    file.read((char*)header, sizeof(header));
    if (((header[0] == 0x42) && (header[1] == 0x4D)) || (int(header[0]) == 80 && int(header[1]) == 54)) {
        return true;
    }
    std::cout << "File is not BMP, not PPM format\n";
    return false;
}

/**
 * Method returning vector of bits which create every character's byte
 * @param message message to hide
 * @return vector of bits which create every character's byte
 * */
std::vector<int> getMessageInBits(const char* message) {
    std::vector<int> bytes;
    for (int i = 0; i < strlen(message); ++i) {
        int power = 7, ascii_value = int(unsigned(message[i]));

        while (power >= 0) {
            int value = int(pow(2, power));
            if (ascii_value - value >= 0) {
                bytes.push_back(1);
                ascii_value -= value;
            }
            else
                bytes.push_back(0);
            power--;
        }
    }
    return bytes;
}

/**
* Method which encodes the message in bmp or ppm file
* @param path absolute path to the file
* @param secret_message message to hide
* */
void encryptMessageInFile(const std::string& path, const char* secret_message) {
    if (!isFileFormatSupported(path))
        return;
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        std::cout << "Could not open the file.";
        return;
    }
    std::vector<int> to_change, message_in_bytes;
    std::string message = addMessageIdentifier(secret_message);
    int mess_byte_length = int(message.size() * 8);
    char* byteBuffer = new char[mess_byte_length];
    int i = 200;
    file.seekp(i);
    file.read(byteBuffer, mess_byte_length);
    for (int a = 0; a < mess_byte_length; ++a)
        to_change.push_back(int(byteBuffer[a]));
    delete[] byteBuffer;
    const char* message_to_hide = message.c_str();
    message_in_bytes = getMessageInBits(message_to_hide);


    for (int j = 0; j < message_in_bytes.size(); ++j) {
        if ((message_in_bytes[j] == 1 && to_change[j] % 2 == 0) ||
            (message_in_bytes[j] == 0 && to_change[j] % 2 == 1)) {
            file.seekp(i);
            file << char(to_change[j] + 1);
            i++;
        }
        else
            i++;
    }
    std::cout << "Your message was successfully hidden\n";


}

/**
 * Method which checks if the message is hidden inside the file
 * @param path absolute path to the file
 * @param secret_message hidden message
 * */
void checkIfFileIsEncrypted(const std::string& path, const char* secret_message) {
    if (!isFileFormatSupported(path))
        return;
    std::fstream file_to_check(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_to_check) {
        std::cout << "Could not open the file.";
        return;
    }
    std::string message;
    message = addMessageIdentifier(secret_message);
    const char* message_to_check = message.c_str();
    std::vector<int> to_check;
    file_to_check.seekp(200);
    int len = int(message.size() * 8);
    char* byteBuffer = new char[len];
    file_to_check.read(byteBuffer, len);
    to_check.reserve(len);
    for (int i = 0; i < strlen(message_to_check) * 8; ++i) {
        if (int(byteBuffer[i]) % 2 == 0)
            to_check.push_back(0);
        else
            to_check.push_back(1);
    }

    if (to_check == getMessageInBits(message_to_check))
        std::cout << "This message is hidden in this file\n";
    else {
        std::cout << "This message isn't hidden in this file\n";
        std::cout << "Try using \"-d\" option to check hidden message.\n";
    }
    file_to_check.seekg(0, std::ios::end);
    int size = int(file_to_check.tellg());

    if (len <= size - 200)
        std::cout << "This message can be written in this file.";
    else {
        std::cout << "This message can't be written in this file.\n";
        std::cout << "Message's size must be shorter than " << int(size / 8) << " characters long.";
    }

    delete[] byteBuffer;
}

/**
 * Method which writes hidden message (if exists)
 * @param path absolute path to the file
 * */
void decryptFileHiddenMessage(const std::string& path) {
    if (!isFileFormatSupported(path))
        return;
    std::fstream file_to_decode(path, std::ios::in | std::ios::out | std::ios::binary);
    if (!file_to_decode) {
        std::cout << "Could not open the file.";
        return;
    }
    std::vector<int> is_message;
    file_to_decode.seekp(200);
    char beg_buffer[24];
    file_to_decode.read(beg_buffer, sizeof(beg_buffer));
    int i = 224;

    for (char c : beg_buffer) {
        if (int(c) % 2 == 0)
            is_message.push_back(0);
        else
            is_message.push_back(1);
    }
    if (is_message == getMessageInBits("BOF"))
        std::cout << "Secret message in this file is: ";
    else {
        std::cout << "This file does not contain a secret message.\n";
        std::cout << "Try using \"-e\" option to hide the message.\n";
        return;
    }
    std::string secret_message;
    while (secret_message.find("EOF") == std::string::npos) {
        std::vector<int> to_calculate;
        file_to_decode.seekp(i);
        char byteBuffer[8];
        file_to_decode.read(byteBuffer, 8);
        for (char c : byteBuffer) {
            if (int(c) % 2 == 0)
                to_calculate.push_back(0);
            else
                to_calculate.push_back(1);
        }
        int power = 7;
        int result = 0;

        for (int k = 0; k < 8; ++k) {
            if (to_calculate[k] == 1)
                result += int(pow(2, power));
            power--;
        }
        char character = char(result);
        secret_message += character;
        i += 8;

    }

    std::cout << secret_message.substr(0, secret_message.size() - 3) << std::endl;
}

/**
* Method which returns message with added identifier
 * @param secret_message message to hide
 * @return message with identifier
*/
std::string addMessageIdentifier(const char* secret_message) {
    std::string message = "BOF";
    return message.append(std::string(secret_message)).append("EOF");
}

/**
* Method which returns file format identifier
* 1-bmp, 2-ppm, 0-Wrong file
* @param path absolute path to the file
*/
int returnFileFormatIdentifier(std::string& path) {
    if (!isFileFormatSupported(path))
        return 0;
    std::ifstream file(path);
    if (!file) {
        std::cout << "Could not open the file.";
        return 0;
    }
    uint8_t header[2];
    file.read((char*)header, sizeof(header));
    if (((header[0] == 0x42) && (header[1] == 0x4D))) {
        return 1;
    }
    if (int(header[0]) == 80 && int(header[1]) == 54)
        return 2;
    return 0;
}

/**
* Method which calls the right info method according to identifier
* @param path absolute path to the file
*/
void printInformationAboutFile(std::string& path) {
    int id = returnFileFormatIdentifier(path);
    switch (id) {
    case 1:
        printBmpFileInformation(path);
        break;
    case 2:
        printPpmFileInformation(path);
        break;
    default:
        std::cout << "File is not bmp, nor ppm format\n";
        break;
    }
}

/**
* Method writing bmp file information
* @param path absolute path to the file
*/
void printBmpFileInformation(std::string& path) {
    std::fstream bmpFile(path, std::ios::in);
    if (!bmpFile) {
        std::cout << "Could not open the file.";
        return;
    }
    bmpFile.seekg(0, std::ios::end);
    int size = int(bmpFile.tellg());
    bmpFile.seekg(0, std::ios::beg);
    std::cout << "File format is: bmp.\n";
    std::cout << "File size is: " << size << " bytes long." << std::endl;
    writeLastFileTimestamp(path.c_str());
}

/**
* Method writing ppm file information
* @param path absolute path to the file
*/
void printPpmFileInformation(std::string& path) {

    std::fstream ppmFile(path, std::ios::in);
    if (!ppmFile) {
        std::cout << "Could not open the file.";
        return;
    }
    ppmFile.seekg(0, std::ios::end);
    int size = int(ppmFile.tellg());
    ppmFile.seekg(0, std::ios::beg);
    std::cout << "File format is: .PPM\n";
    std::cout << "File size is: " << size << " bytes long.\n";
    writeLastFileTimestamp(path.c_str());
}

/**
* Method writing last write timestamp
* @param path absolute path to the file
*/
void writeLastFileTimestamp(const char* path) {
    struct stat attributes;
    stat(path, &attributes);
    printf("Last modified time: %s", ctime(&attributes.st_mtime));
}

/**
* Method writing project's manual
*/
void printProgramsManual() {
    std::cout << "\n";
    std::cout << "Project: \"Steganography\"\n";
    std::cout << "Author: Artur Soszynski\n";
    std::cout << "\n\n";
    std::cout << "This program works with bmp or ppm file. Every other format is not supported\n";
    std::cout << "Hidden message starts with string \"BOF\", and ends on \"EOF\".\n";
    std::cout << "Please, do not use \"EOF\" string in hidden message!";
    std::cout << "Below you can find program's functions description:\n";
    std::cout
        << "\tFlag \" - e\" or \"--encrypt\" taking two arguments (absolute path to the file and message to hide)\n";
    std::cout << "which encodes the message in the file.\n";
    std::cout
        << "\tFlag \" - c\" or \"--check\"taking two arguments (absolute path to the file and message to hide)\n";
    std::cout << "which checks if this file contains this exact message encoded.\n";
    std::cout
        << "\tFlag \" - d\" or \"--decrypt\" taking one argument (absolute path to the file), which decodes the\n";
    std::cout << "message from the file (if one exists)\n";
    std::cout
        << "\tFlag \" - i\" or \"--info\" taking one argument (absolute path to the file), which provides\n";
    std::cout << "information about the file (format, size and last modification time)\n";
    std::cout << "Flag \" - h\" or \"--help\" which takes no arguments and provides program's manual.\n";
    std::cout << "Attention! Program does not support combining flags. Every attempt of mixing functions or\n";
    std::cout << "running the program with wrong arguments will lead to failure.\n\n";


}













