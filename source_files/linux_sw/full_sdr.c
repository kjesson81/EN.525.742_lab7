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

#define SDR_PERIPH_ADDRESS 0x43C10000
#define SDR_FAKE_ADC_PINC_OFFSET 0
#define SDR_TUNER_PINC_OFFSET 1
#define SDR_RST_REG_OFFSET 2
#define SDR_TIMER_REG_OFFSET 3

#define ETH_FIFO_ADDR 0x43C00000
#define ETH_DATA_OFFSET 0
#define ETH_DATA_CNT_OFFSET 1

#define BUFFER_SIZE 50
#define CMPLX_SAMPLE_PACKET 256
#define PACKET_SIZE 1026 // bytes

// A packet will contain 256 16 bit complex samples
#define PACKET_BYTE_SIZE 256
// global variables to track frequency value;
float GlobalFrequency;
float GlobalTuneFrequency;
int32_t *testData;

int volumeLevel = 5;

volatile unsigned int *get_a_pointer(unsigned int phys_addr);
void SDR_tuneRadio(volatile unsigned int *ptrToRadio, float tune_frequency);
void SDR_setADCfreq(volatile unsigned int *ptrToRadio, float freq);
void play_tune(volatile unsigned int *ptrToRadio, float base_frequency);
void print_benchmark(volatile unsigned int *periph_base);
void userMenu(volatile unsigned int *ptrToRadio, volatile unsigned int *ptrToFIFO);
uint32_t readClockCount(volatile unsigned int *ptrToRadio);
uint32_t getRdCnt(volatile unsigned int *ptrToFIFO);
int32_t getData(volatile unsigned int *ptrToFIFO);
void packetFifoBenchmark(volatile unsigned int *ptrToFIFO, volatile unsigned int *ptrToRadio, int sampleSize);
void packetMode(volatile unsigned int *ptrToFIFO, char *ipAddr, int portNum);
int isKeyPressed();
int32_t *readColumnAsUint32(const char *filename, size_t *countOut);
void configure_codec_volume(int volume);

int main()
{

    // first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *sdr_periph = get_a_pointer(SDR_PERIPH_ADDRESS);
    volatile unsigned int *fifo_periph = get_a_pointer(ETH_FIFO_ADDR);

    size_t count = 0;
    testData = readColumnAsUint32("./ila_fir_data.csv", &count);
    if (!testData)
    {
        return 1;
    }

    printf("\r\n\r\n\r\nLab 7 Kai Jesson - Full SDR with Ethernet Streaming\n\r");

    *(sdr_periph + SDR_RST_REG_OFFSET) = 0; // make sure radio isn't in reset

    printf("Tuning Radio to 30MHz\n\r");
    GlobalTuneFrequency = 30e6;
    SDR_tuneRadio(sdr_periph, GlobalTuneFrequency);
    printf("Playing Tune at near 30MHz\r\n");
    play_tune(sdr_periph, GlobalTuneFrequency);

    print_benchmark(sdr_periph);

    userMenu(sdr_periph, fifo_periph);

    return 0;
}

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int *get_a_pointer(unsigned int phys_addr)
{

    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr);
    volatile unsigned int *radio_base = (volatile unsigned int *)map_base;
    return (radio_base);
}

void SDR_tuneRadio(volatile unsigned int *ptrToRadio, float tune_frequency)
{
    float pinc = (-1.0 * tune_frequency) * (float)(1 << 27) / 125.0e6;
    *(ptrToRadio + SDR_TUNER_PINC_OFFSET) = (int)pinc;
}

void SDR_setADCfreq(volatile unsigned int *ptrToRadio, float freq)
{
    float pinc = freq * (float)(1 << 27) / 125.0e6;
    *(ptrToRadio + SDR_FAKE_ADC_PINC_OFFSET) = (int)pinc;
}

void play_tune(volatile unsigned int *ptrToRadio, float base_frequency)
{
    int i;
    float freqs[16] = {1760.0, 1567.98, 1396.91, 1318.51, 1174.66, 1318.51, 1396.91, 1567.98, 1760.0, 0, 1760.0, 0, 1760.0, 1975.53, 2093.0, 0};
    float durations[16] = {1, 1, 1, 1, 1, 1, 1, 1, .5, 0.0001, .5, 0.0001, 1, 1, 2, 0.0001};
    for (i = 0; i < 16; i++)
    {
        SDR_setADCfreq(ptrToRadio, freqs[i] + base_frequency);
        usleep((int)(durations[i] * 500000));
    }
    // set default values
    GlobalFrequency = (float)(30e6 + 1000);
    GlobalTuneFrequency = (float)30e6;
}

void print_benchmark(volatile unsigned int *periph_base)
{
    // the below code does a little benchmark, reading from the peripheral a bunch
    // of times, and seeing how many clocks it takes.  You can use this information
    // to get an idea of how fast you can generally read from an axi-lite slave device
    unsigned int start_time;
    unsigned int stop_time;
    start_time = *(periph_base + SDR_TIMER_REG_OFFSET);

    for (int i = 0; i < 2048; i++)
    {
        stop_time = *(periph_base + SDR_TIMER_REG_OFFSET);
    }

    printf("Elapsed time in clocks = %u\n", stop_time - start_time);

    float throughput = 0;
    unsigned int bytes_transferred = 8196;

    float time_spent = (float)(stop_time - start_time) / 125e6;

    throughput = (float)bytes_transferred / time_spent;

    printf("You transferred %d  bytes of data in %f seconds\n", bytes_transferred, time_spent);
    printf("Measured Transfer throughput = %f Mbytes/sec\n", throughput / 1000000);
}

void userMenu(volatile unsigned int *ptrToRadio, volatile unsigned int *ptrToFIFO)
{

    while (1)
    {

        printf("\n\r    - Press S/s to test the DDS System with a tune");
        printf("\n\r    - Press T/t to tune the system to a desired frequency");
        printf("\n\r    - Press E/e to enter a frequency");
        printf("\n\r    - Press U/u to increase frequency by 1000Hz/100Hz");
        printf("\n\r    - Press D/d to decrease frequency by 1000Hz/100Hz");
        printf("\n\r    - Press R/r to reset phase");
        printf("\n\r    - Press +/- to increase/decrease volume");
        printf("\n\r    - Press C/c to read the clock counts");
        printf("\n\r    - Press P/p to stream the packet data over ethernet. You will chosose IP address and port to send to.\n\rNote: This is configured for the default provided collect_data_complex.m script\n\r");
        printf("\n\r    - Press Q/q to read the packet data count");
        printf("\n\r    - Press B/b to benchmark 480,000 packet samples\n\r\n\r");
        char option[BUFFER_SIZE];
        scanf("%s", option);

        if ((*option != 'B') && (*option != 'b') && (*option != 'S') && (*option != 's') && (*option != 'T') && (*option != 't') && (*option != 'E') && (*option != 'e') && (*option != 'F') && (*option != 'f') && (*option != 'C') && (*option != 'c') && (*option != 'U') && (*option != 'u') && (*option != 'D') && (*option != 'd') && (*option != 'R') && (*option != 'r') && (*option != '+') && (*option != '-') && (*option != 'P') && (*option != 'p') && (*option != 'Q') && (*option != 'q'))
        {
            printf("ERROR: Invalid selection! You entered: %c. Please try again.\n\r", *option);
        }
        else
        {
            if ((*option == 'S') || (*option == 's'))
            {
                play_tune(ptrToRadio, GlobalTuneFrequency);
            }
            else if ((*option == 'T') || (*option == 't'))
            {
                printf("Please enter your tuning frequency: ");
                ;
                char userInput[BUFFER_SIZE];
                scanf("%49s", userInput);
                GlobalTuneFrequency = strtof(userInput, NULL);
                SDR_tuneRadio(ptrToRadio, GlobalTuneFrequency);
            }
            else if ((*option == 'E') || (*option == 'e'))
            {
                printf("Please enter your desired frequency: ");
                ;
                char userInput[BUFFER_SIZE];
                scanf("%49s", userInput);
                GlobalFrequency = strtof(userInput, NULL);
                printf("\n\r");
                SDR_setADCfreq(ptrToRadio, GlobalFrequency);
            }
            else if (*option == 'U')
            {
                GlobalFrequency += 1000;
                SDR_setADCfreq(ptrToRadio, GlobalFrequency);
                printf("Increased by 1000 Hz\n\r");
            }
            else if (*option == 'u')
            {
                GlobalFrequency += 100;
                SDR_setADCfreq(ptrToRadio, GlobalFrequency);
                printf("Increased by 100 Hz\n\r");
            }
            else if (*option == 'D')
            {
                GlobalFrequency -= 1000;
                SDR_setADCfreq(ptrToRadio, GlobalFrequency);
                printf("Decreased by 1000 Hz\n\r");
            }
            else if (*option == 'd')
            {
                GlobalFrequency -= 100;
                SDR_setADCfreq(ptrToRadio, GlobalFrequency);
                printf("Decreased by 100 Hz\n\r");
            }
            else if ((*option == 'R') || (*option == 'r'))
            {
                *(ptrToRadio + SDR_RST_REG_OFFSET) = 1;
                usleep(1000);
                *(ptrToRadio + SDR_RST_REG_OFFSET) = 0;
            }
            else if ((*option == 'C') || (*option == 'c'))
            {
                printf("Current Clock Count :  %" PRIu32 "\n\r", readClockCount(ptrToRadio));
            }
            else if (*option == '+')
            {
                if (volumeLevel < 9)
                {
                    volumeLevel++;
                }
                else
                {
                    printf("Volume can not go any higher!\n\r");
                }
                configure_codec_volume(volumeLevel);
            }
            else if (*option == '-')
            {
                if (volumeLevel > 0)
                {
                    volumeLevel--;
                }
                else
                {
                    printf("Volume can not go any lower!\n\r");
                }
                configure_codec_volume(volumeLevel);
            }
            else if ((*option == 'P') || (*option == 'p'))
            {

                printf("Please enter the IP address to send ethernet data to: ");
                char ip_addr[BUFFER_SIZE];
                scanf("%49s", ip_addr);
                printf("\n\r");

                printf("Please enter the port you want to connect to: ");
                char userInput[BUFFER_SIZE];
                scanf("%49s", userInput);
                printf("\n\r");
                int selectedPort = atoi(userInput);

                printf("Sending UDP ethernet data to IP address %s, port %d\n\r", ip_addr, selectedPort);
                printf("Hit any key to stop.\n\r");

                packetMode(ptrToFIFO, ip_addr, selectedPort); //"192.168.196.127"
            }
            else if ((*option == 'Q') || (*option == 'q'))
            {
                printf("Current packet data count is :  %" PRIu32 "\n\r", getRdCnt(ptrToFIFO));
            }
            else if ((*option == 'B') || (*option == 'b'))
            {
                printf("Running Benchmark - please wait....\n\r");
                packetFifoBenchmark(ptrToFIFO, ptrToRadio, 480000);
            }
            printf("Current Frequency : %.2f Hz\n\rCurrent Tuning Frequency %.2f\n\r", GlobalFrequency, GlobalTuneFrequency);
        }
    }
}

uint32_t readClockCount(volatile unsigned int *ptrToRadio)
{
    uint32_t value = *(ptrToRadio + SDR_TIMER_REG_OFFSET);
    return value;
}

void packetFifoBenchmark(volatile unsigned int *ptrToFIFO, volatile unsigned int *ptrToRadio, int sampleSize)
{
    int32_t packet_data_storage[sampleSize];
    int prev_value = -1;
    int i = 0;
    unsigned int start_time;
    unsigned int stop_time;
    start_time = *(ptrToRadio + SDR_TIMER_REG_OFFSET);

    // wait until enough samples
    while (i < sampleSize)
    {
        if (getRdCnt(ptrToFIFO) > 0)
        {
            int new_value = getData(ptrToFIFO); // Get the new value
            if (new_value != prev_value)
            {                                       // Compare with the previous value
                packet_data_storage[i] = new_value; // Store it if different
                prev_value = new_value;             // Update the previous value
                i++;
            }
        }
    }
    stop_time = *(ptrToRadio + SDR_TIMER_REG_OFFSET);
    float time_spent = (float)(stop_time - start_time) / 125e6;
    printf("%d samples of data in approximately %f seconds\n", sampleSize, time_spent);
};

int32_t getData(volatile unsigned int *ptrToFIFO)
{
    return *(ptrToFIFO + ETH_DATA_OFFSET);
};

uint32_t getRdCnt(volatile unsigned int *ptrToFIFO)
{
    return *(ptrToFIFO + ETH_DATA_CNT_OFFSET);
};

void packetMode(volatile unsigned int *ptrToFIFO, char *ipAddr, int portNum)
{
    // volatile unsigned int *fifo_periph = get_a_pointer(ETH_FIFO_ADDR);

    uint8_t packet_data[PACKET_SIZE]; // minus 2 to fit header
    int32_t prev_value = 0;
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

    while (!isKeyPressed())
    {
        // Assign the packet counter to the beginning of the packet
        packet_data[0] = (packet_counter) & 0xFF;
        packet_data[1] = (packet_counter >> 8) & 0xFF;

        i = 0; // Reset the sample index
        while (i < CMPLX_SAMPLE_PACKET)
        {
            if (getRdCnt(ptrToFIFO) > 0)
            {
                int32_t new_value = getData(ptrToFIFO); // Get the new value
                if (new_value != prev_value)
                {
                    // Populate the data starting after the packet counter
                    // packet_data[(i * 4) + 2] = 0x11; //(new_value) & 0xFF;
                    // packet_data[(i * 4) + 3] = 0x22; //(new_value >> 8) & 0xFF;
                    // packet_data[(i * 4) + 4] = 0x33; //(new_value >> 16) & 0xFF;
                    // packet_data[(i * 4) + 5] = 0x44; //(new_value >> 24) & 0xFF;

                    // printf("new value is :  0x%08X\n\r",new_value);

                    packet_data[(i * 4) + 2] = (new_value) & 0xFF;
                    packet_data[(i * 4) + 3] = (new_value >> 8) & 0xFF;
                    packet_data[(i * 4) + 4] = (new_value >> 16) & 0xFF;
                    packet_data[(i * 4) + 5] = (new_value >> 24) & 0xFF;

                    // packet_data[(i * 4) + 2] = (testData[j]) & 0xFF;       // (new_value) & 0xFF;
                    // packet_data[(i * 4) + 3] = (testData[j] >> 8) & 0xFF;  // (new_value >> 8) & 0xFF;
                    // packet_data[(i * 4) + 4] = (testData[j] >> 16) & 0xFF; // (new_value >> 16) & 0xFF;
                    // packet_data[(i * 4) + 5] = (testData[j] >> 24) & 0xFF; // (new_value >> 24) & 0xFF;

                    prev_value = new_value; // Update the previous value
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
            }
        }
        sendto(sockfd, packet_data, PACKET_SIZE, 0, (struct sockaddr *)NULL, sizeof(servaddr));
        packet_counter++;
    }

    // close the descriptor
    close(sockfd);
};

int isKeyPressed()
{
    struct timeval tv = {0, 0}; // No waiting
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Use select to check if input is ready
    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0;
}

int32_t *readColumnAsUint32(const char *filename, size_t *countOut)
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

int i2c_write_reg(const char *i2cDev, int deviceAddr, uint8_t reg, uint8_t value)
{
    int fd = open(i2cDev, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open I2C device");
        return -1;
    }

    if (ioctl(fd, I2C_SLAVE, deviceAddr) < 0)
    {
        perror("Failed to set I2C address");
        close(fd);
        return -1;
    }

    uint8_t data[2] = {reg, value};
    if (write(fd, data, 2) != 2)
    {
        perror("Failed to write to I2C device");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

void configure_codec_volume(int volume)
{
    uint8_t codecVolume;

    switch (volume)
    {
    case 0:
        codecVolume = 0x43;
        volumeLevel = 0;
        break;
    case 1:
        codecVolume = 0x49;
        volumeLevel = 1;
        break;
    case 2:
        codecVolume = 0x4F;
        volumeLevel = 2;
        break;
    case 3:
        codecVolume = 0x5B;
        volumeLevel = 3;
        break;
    case 4:
        codecVolume = 0x61;
        volumeLevel = 4;
        break;
    case 5:
        codecVolume = 0x67;
        volumeLevel = 5;
        break;
    case 6:
        codecVolume = 0x6D;
        volumeLevel = 6;
        break;
    case 7:
        codecVolume = 0x73;
        volumeLevel = 7;
        break;
    case 8:
        codecVolume = 0x79;
        volumeLevel = 8;
        break;
    case 9:
        codecVolume = 0x7F;
        volumeLevel = 9;
        break;
    default:
        printf("Invalid option selected.\n");
        return;
    }

    i2c_write_reg("/dev/i2c-0", 0x1A, 0x04, codecVolume);
    i2c_write_reg("/dev/i2c-0", 0x1A, 0x06, codecVolume);
}