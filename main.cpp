#include <iostream>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <linux/filter.h>

struct Packet   //Структура яка буде рахувати кількість пакетів і розмір
{   
    int size = 0;
    int count = 0;
};

int main() {
    
    int rawSocket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  //Сокет який буде приймати все
    if (rawSocket == -1) {
        std::cerr << "Failed to create a raw socket. Dont forget to use SUDO!" << std::endl;
        return 1;
    }

    struct sock_filter code[] = {       //BPF фільтр який буде фільтрувати "сміття"
    
    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, 0),       
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0x4500, 0, 1),
    BPF_STMT(BPF_LD + BPF_B + BPF_ABS, 9),       
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, 0, 1, 0), 
    BPF_STMT(BPF_RET + BPF_K, 0xFFFFFFFF),        
    BPF_STMT(BPF_RET + BPF_K, 0),                 
    };

    struct sock_fprog bpfProgram;       //Завантаження коду BPF-фільтра в структуру BPF
    bpfProgram.len = sizeof(code) / sizeof(struct sock_filter);
    bpfProgram.filter = code;

    if (setsockopt(rawSocket, SOL_SOCKET, SO_ATTACH_FILTER, &bpfProgram, sizeof(bpfProgram)) == -1) {       //Прикріплення BPF-фільтра до сокету
        std::cerr << "Failed to attach BPF filter." << std::endl;
        close(rawSocket);
        return 1;
    }

    char buffer[65536];
    Packet packet;

    while (true) { 
        
        ssize_t dataSize = recv(rawSocket, buffer, sizeof(buffer), 0);  //Записуємо розмір кожного пакету
        
        if (dataSize == -1) {
            std::cerr << "Failed to receive data via raw socket." << std::endl;
            close(rawSocket);
            return 1;
        }

        packet.size+=dataSize;
        packet.count++;
        
        system("clear");
        std::cout << "Total count: " << packet.count << " packets."<< std::endl;
        std::cout << "Total size: " << packet.size << " bytes."<< std::endl;
        
    }

    close(rawSocket);
    return 0;
}
