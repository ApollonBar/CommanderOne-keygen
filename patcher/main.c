#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <memory.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

const char pubkey[9][72] = {
    "-----BEGIN PUBLIC KEY-----",
    "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxqkTcfbKw8ysVygePlcB",
    "oUAhCF6oniyP13iDtu85ZsHwqw8PnMyTp6n6FnMN9YinleIAy6NFveBu/vshTN8S",
    "oXbYyy5AqdZ8CQpfvuriO9UNfgV1l7SFdPPpruFAmOw+uzA3GawMsg3QNK/htqJe",
    "b4xKHFS04xC2AueE2RTmk6tJcL8TEBfRG7DEYOHPjebKl1NQ3ZIu15U97cCPYKO2",
    "pWHzsb+Fr4Wj0DChLoxlXxaBcJ2ozogaq0tW2t4Aopvt9kRSuSK9HcgxICJM5ct4",
    "naU91WFGWlw0+0JpiMIl5OnMbpak/5xQre9DL8zM8LjRy14I88txvXvhPEsWaYCO",
    "1QIDAQAB",
    "-----END PUBLIC KEY-----"
};

void help() {
    printf("Usage:\n");
    printf("    ./patcher <CommanderOne executable file>\n");
    printf("\n");
}

size_t search_pubkey_location(uint8_t* pFileContent, size_t FileSize) {
    static const char search_str[] = "-----BEGIN PUBLIC KEY-----";

    size_t i = 0;
    if (FileSize < sizeof(search_str) - 1) return (size_t)-1;
    FileSize -= sizeof(search_str);
    for (; i < FileSize; ++i) {
        if (pFileContent[i] == '-' && memcmp(pFileContent + i, search_str, sizeof(search_str) - 1) == 0)
            return i;
    }
    return (size_t)-1;
}

int do_patch(uint8_t* pFileContent, size_t offset) {
    char patch_str[1024] = { };
    sprintf(patch_str, "%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s",
            pubkey[0],
            pubkey[1],
            pubkey[2],
            pubkey[3],
            pubkey[4],
            pubkey[5],
            pubkey[6],
            pubkey[7],
            pubkey[8]);
    if (strlen((char*)pFileContent + offset) == strlen(patch_str)) {
        strcpy((char*)pFileContent + offset, patch_str);
        return 1;
    } else {
        return 0;
    }
}

int main(int argc, char* argv[], char* envp[]) {
    int status = 0;
    int fd = -1;
    struct stat fd_stat = {};
    uint8_t* file_content = 0;

    if (argc != 2) {
        help();
        return 0;
    }

    fd = open(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        printf("Failed to open file. CODE: 0x%08x\n", errno);
        status = errno;
        goto main_fin;
    } else {
        printf("Open file successfully.\n");
    }

    if (fstat(fd, &fd_stat) != 0) {
        printf("Failed to get file size. CODE: 0x%08x\n", errno);
        status = errno;
        goto main_fin;
    } else {
        printf("Get file size successfully: %zu\n", fd_stat.st_size);
    }

    file_content = mmap(NULL, fd_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (file_content == (void*)-1) {
        printf("Failed to map file. CODE: 0x%08x\n", errno);
        status = errno;
        goto main_fin;
    } else {
        printf("Map file successfully.\n");
    }

    size_t offset = search_pubkey_location(file_content, fd_stat.st_size);
    if (offset == (size_t)-1) {
        printf("Failed to find pubkey location.\n");
        goto main_fin;
    }

    printf("offset = 0x%016llx\n", offset);
    
    if(do_patch(file_content, offset)) {
        printf("Success!\n");
    } else {
        printf("Failed!\n");
    }
    
main_fin:
    if (file_content != NULL) munmap(file_content, fd_stat.st_size);
    if (fd != -1) close(fd);
    return status;
}

