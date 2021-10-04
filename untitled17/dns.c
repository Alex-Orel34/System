#include "dns.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define WORKING_DNS_HANDLE 42

typedef unsigned int uint;


#define SIZE 30001

uint* hashArray[SIZE];
uint lengths[SIZE];

char** hosts;
uint* ips;
uint* hashes;
uint cnt;

uint hash(const char* str) {
    uint result = 0;
    while (*str) {
        result = (result * 31 + (*str) - 'a') % SIZE;
        str++;
    }
    return result;
}

DNSHandle InitDNS( )
{
    return WORKING_DNS_HANDLE;
}

unsigned int getCntOfLines(FILE* fInput)
{
#define BUFFER_SIZE 200
    unsigned int lineCounter = 0;
    char buffer[BUFFER_SIZE];

    if ( !fInput )
        return 0;

    fseek(fInput, 0, SEEK_SET);
    while ( !feof(fInput) )
    {
        fgets(buffer, BUFFER_SIZE, fInput);
        lineCounter ++ ;
    }
    return lineCounter;
}

char buffer[201] = {0};

void LoadHostsFile( DNSHandle hDNS, const char* hostsFilePath )
{
#define SSIZE 200
    FILE* fInput = fopen(hostsFilePath, "r");
    uint ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
    uint i = 0;
    uint mx = 0;

    cnt = getCntOfLines(fInput);

    hosts = (char**) malloc( cnt * sizeof(char*));
    ips = (uint*) calloc( cnt , sizeof(uint));
    hashes = (uint*) malloc( cnt * sizeof(uint));

    fseek(fInput, 0, SEEK_SET);

    for ( i=0; i<cnt && !feof(fInput); i++ )
    {

        unsigned int uHostNameLength =  0;
        unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;


        if ( 5 != fscanf(fInput, "%d.%d.%d.%d %s", &ip1, &ip2, &ip3, &ip4, buffer, 200) )
            continue;

        ips[i] = ( ip1 & 0xFF ) << 24 |
                 ( ip2 & 0xFF ) << 16 |
                 ( ip3 & 0xFF ) << 8  |
                 ( ip4 & 0xFF ) ;


        uHostNameLength = strlen(buffer);
        if (uHostNameLength)
        {
            hosts[i] = (char*) malloc(uHostNameLength+1);
            strcpy(hosts[i], buffer);
            hashes[i] = hash(buffer);
            lengths[ hashes[i] ]++;
        } else {
            i--;
            cnt--;
        }
    }

    fclose(fInput);

    for (i = 0; i < SIZE; ++i)
        if (lengths[i] > 0) {
            hashArray[i] = ((uint*) malloc(lengths[i] * sizeof(uint))) + lengths[i] + 1;
        }
    for (i = 0; i < cnt; ++i)
        if (ips[i] != 0) {
            hashArray[ hashes[i] ]--;
            *(hashArray[ hashes[i] ]) = i;
        }
}


void ShutdownDNS( DNSHandle hDNS )
{
    int i = 0;
    for (i = 0; i < cnt; ++i) {
        if (ips[i] == 0) continue;
        free(hosts[i]);
    }
    free(hosts);
    free(ips);
    free(hashes);
}


IPADDRESS DnsLookUp( DNSHandle hDNS, const char* hostName )
{
    int i = 0;
    uint h = hash(hostName);
    for (i = 0; i < lengths[h]; ++i) {
        uint pos = hashArray[h][i];
        if (strcmp(hostName, hosts[pos]) == 0) return ips[pos];
    }

    return INVALID_IP_ADDRESS;
}