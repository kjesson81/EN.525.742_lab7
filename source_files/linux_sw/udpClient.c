// NOTE : This code was modifed from https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/#
// udp client driver program
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>

#define BUFFER_SIZE 50
#define CMPLX_SAMPLE_PACKET 256
#define PACKET_SIZE 1026 // bytes

#define ETH_FIFO_ADDR 0x43C10000
#define ETH_DATA_OFFSET 0
#define ETH_DATA_CNT_OFFSET 1

// Driver code
int main()
{
    // volatile unsigned int *fifo_periph = get_a_pointer(ETH_FIFO_ADDR);

    uint8_t packet_data[PACKET_SIZE]; // minus 2 to fit header
    uint32_t prev_value = 0;
    uint16_t packet_counter = 0;
    int i = 0;
    int j = 0;
    uint32_t tone6Khz[] = {0, 7070, 10000, 7070, 0, -7070, -10000, -7070};

    int sockfd, n;
    struct sockaddr_in servaddr;

    // user added code
    char ipAddr[BUFFER_SIZE];
    printf("Please enter target IP address : ");
    scanf("%s", ipAddr);
    printf("\n\r");

    char portSel[BUFFER_SIZE];
    int portNum = 25344; // default value
    printf("Please enter your desired port number : ");
    scanf("%49s", portSel);
    printf("\n\r");
    portNum = atoi(portSel);

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

    // while (1)
    // {
    //     while (i < CMPLX_SAMPLE_PACKET)
    //     {
    //         uint32_t new_value = tone6Khz[j];

    //         packet_data[(i * 4)] = (new_value) & 0xFF; // High byte of signal
    //         packet_data[(i * 4) + 1] = (new_value >> 8) & 0xFF;
    //         packet_data[(i * 4) + 2] = (new_value >> 16) & 0xFF;
    //         packet_data[(i * 4) + 3] = (new_value >> 24) & 0xFF;

    //         prev_value = new_value; // Update the previous value
    //         i++;

    //         if (j == 7)
    //         {
    //             j = 0;
    //         }
    //         else
    //         {
    //             j++;
    //         }
    //     }

    //     // while (i < CMPLX_SAMPLE_PACKET)
    //     // {
    //     // if (getRdCnt(fifo_periph) > 0)
    //     // {
    //     //     uint32_t new_value = getData(fifo_periph); // Get the new value
    //     //     if (new_value != prev_value)
    //     //     {
    //     //         packet_data[(i*4)] = (new_value) & 0xFF;  // High byte of signal
    //     //         packet_data[(i*4)+1] = (new_value >> 8) & 0xFF;
    //     //         packet_data[(i*4)+2] = (new_value >> 16) & 0xFF;
    //     //         packet_data[(i*4)+3] = (new_value >> 24) & 0xFF;

    //     //         prev_value = new_value;     // Update the previous value
    //     //         i++;
    //     //     }
    //     // }
    //     // }
    //     packet_counter++;

    //     // MSB pf packet_data is the packet count.
    //     packet_data[PACKET_SIZE - 2] = (packet_counter) & 0xFF;
    //     packet_data[PACKET_SIZE - 1] = (packet_counter >> 8) & 0xFF;

    //     sendto(sockfd, packet_data, PACKET_SIZE, 0, (struct sockaddr *)NULL, sizeof(servaddr));
    //     i = 0;
    // }

while (1)
{
    // Assign the packet counter to the beginning of the packet
    packet_data[0] = (packet_counter) & 0xFF;
    packet_data[1] = (packet_counter >> 8) & 0xFF;

    i = 0; // Reset the sample index
    while (i < CMPLX_SAMPLE_PACKET)
    {
        uint32_t new_value = tone6Khz[j];

        // Populate the data starting after the packet counter
        packet_data[(i * 4) + 2] = (new_value) & 0xFF;          // Least significant byte
        packet_data[(i * 4) + 3] = (new_value >> 8) & 0xFF;
        packet_data[(i * 4) + 4] = (new_value >> 16) & 0xFF;
        packet_data[(i * 4) + 5] = (new_value >> 24) & 0xFF;    // Most significant byte

        prev_value = new_value; // Update the previous value
        i++;

        if (j == 7)
        {
            j = 0;
        }
        else
        {
            j++;
        }
    }
    sendto(sockfd, packet_data, PACKET_SIZE, 0, (struct sockaddr *)NULL, sizeof(servaddr));
    //     i = 0;
    packet_counter++;
}

    // close the descriptor
    close(sockfd);
};

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
// volatile unsigned int *get_a_pointer(unsigned int phys_addr)
// {

//     int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
//     void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr);
//     volatile unsigned int *radio_base = (volatile unsigned int *)map_base;
//     return (radio_base);
// }

// uint32_t getData(volatile unsigned int *ptrToFIFO)
// {
//     return *(ptrToFIFO + ETH_DATA_OFFSET);
// };

// uint32_t getRdCnt(volatile unsigned int *ptrToFIFO)
// {
//     return *(ptrToFIFO + ETH_DATA_CNT_OFFSET);
// };
