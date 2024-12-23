#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/select.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#define _BSD_SOURCE

#define BUFFER_SIZE 50
#define CMPLX_SAMPLE_PACKET 256
#define PACKET_SIZE 1026 // bytes

// A packet will contain 256 16 bit complex samples
#define PACKET_BYTE_SIZE 256

int32_t *testData;


void packetMode(char *ipAddr, int portNum, int numPackets);
int32_t *readColumnAsInt32(const char *filename, size_t *countOut);

int main()
{
    
    printf("\n\rKai Jesson - Lab 7 Milestone - UDP Ethernet Packet Streaming\n\r");
    printf("NOTE 1 : This packet data will be transmitted as fast as possible. \n\r");
    printf("NOTE 2 : The UDP packet payload data is abstracted from the ila_data.csv file that must be in the same directory as this executable.\n\r\n\r");

    size_t count = 0;
    testData = readColumnAsInt32("./ila_data.csv", &count);
    if (!testData)
    {
        return 1;
    }

    printf("Please enter the IP address to send ethernet data to: ");
    char ip_addr[BUFFER_SIZE];
    scanf("%49s", ip_addr);
    printf("\n\r");

    printf("Please enter the port you want to connect to: ");
    char userInput[BUFFER_SIZE];
    scanf("%49s", userInput);
    printf("\n\r");
    int selectedPort = atoi(userInput);

    printf("Please enter the number of packets you wish to send: ");
    char packets_to_send[BUFFER_SIZE];
    scanf("%49s", packets_to_send);
    printf("\n\r");
    int numPackets = atoi(packets_to_send);

    printf("Sending %d UDP ethernet packets to IP address %s, port %d\n\r", numPackets, ip_addr, selectedPort);

    packetMode(ip_addr, selectedPort, numPackets);


    return 0;
}

void packetMode(char *ipAddr, int portNum, int numPackets)
{

    uint8_t packet_data[PACKET_SIZE]; // minus 2 to fit header
    uint16_t packet_counter = 0;
    int i = 0;
    int j = 0;

    int sockfd, n;
    struct sockaddr_in servaddr;

    // clear servaddr
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr(ipAddr);
    servaddr.sin_port = htons(portNum);
    servaddr.sin_family = AF_INET;

    // create datagram socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // connect to server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        exit(0);
    }

    while (packet_counter < numPackets)
    {
        // Assign the packet counter to the beginning of the packet
        packet_data[0] = (packet_counter) & 0xFF;
        packet_data[1] = (packet_counter >> 8) & 0xFF;

        i = 0; // Reset the sample index

        while (i < CMPLX_SAMPLE_PACKET)
        {
            packet_data[(i * 4) + 2] = (testData[j]) & 0xFF;
            packet_data[(i * 4) + 3] = (testData[j] >> 8) & 0xFF;
            packet_data[(i * 4) + 4] = (testData[j] >> 16) & 0xFF;
            packet_data[(i * 4) + 5] = (testData[j] >> 24) & 0xFF;

            i++;
            if (j >= 62130) // this is where samples repeat again
            {
                j = 0;
            }
            else
            {
                j++;
            }
        }

        sendto(sockfd, packet_data, PACKET_SIZE, 0, (struct sockaddr *)NULL, sizeof(servaddr));
        packet_counter++;
    }

    close(sockfd);
};


int32_t *readColumnAsInt32(const char *filename, size_t *countOut)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("Error opening file");
        return NULL;
    }

    size_t capacity = 0;
    size_t count = 0;
    int32_t *array = NULL;
    char line[256];

    while (fgets(line, sizeof(line), file))
    {
        char *token = strtok(line, ",");
        if (token)
        {
            // Remove trailing newline
            token[strcspn(token, "\r\n")] = '\0';
            // Convert the token to int32_t (hex)
            int32_t value = (int32_t)strtoul(token, NULL, 16);

            // Resize array if needed
            if (count >= capacity)
            {
                size_t newCap = capacity ? capacity * 2 : 8;
                int32_t *temp = realloc(array, newCap * sizeof(int32_t));
                if (!temp)
                {
                    perror("Memory error");
                    free(array);
                    fclose(file);
                    return NULL;
                }
                array = temp;
                capacity = newCap;
            }

            array[count++] = value;
        }
    }

    fclose(file);
    if (countOut)
        *countOut = count;
    return array;
}
