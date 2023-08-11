#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>

int main() {
    // 创建泰山服务器的socket
    int taishan_socket;
    struct sockaddr_in taishan_addr;
    int server_port = 12346; // 用于传输数据的端口，与本地PC的端口一致

    taishan_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (taishan_socket < 0) {
        perror("无法创建socket");
        return 1;
    }

    taishan_addr.sin_family = AF_INET;
    taishan_addr.sin_addr.s_addr = INADDR_ANY;
    taishan_addr.sin_port = htons(server_port);

    // 绑定泰山服务器的socket到指定端口
    if (bind(taishan_socket, (struct sockaddr *)&taishan_addr, sizeof(taishan_addr)) < 0) {
        perror("绑定socket失败");
        close(taishan_socket);
        return 1;
    }

    // 监听指定端口上的连接请求
    if (listen(taishan_socket, 1) < 0) {
        perror("监听连接请求失败");
        close(taishan_socket);
        return 1;
    }

    // 接收连接请求并处理
    int addr_len = sizeof(taishan_addr);
    int client_socket = accept(taishan_socket, (struct sockaddr *)&taishan_addr, (socklen_t *)&addr_len);
    if (client_socket < 0) {
        perror("接收连接失败");
        close(taishan_socket);
        return 1;
    }

    // 从客户端接收测试数据集文件并进行处理
    unsigned char buffer[1024];
    int read_bytes;
    std::ofstream dataset_file("test_dataset.bin", std::ios::binary);
    FILE *received_file = fopen("test_dataset.bin", "wb");
    if (received_file == NULL) {
        perror("无法创建接收文件");
        close(client_socket);
        close(taishan_socket);
        return 1;
    }

    while ((read_bytes = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(unsigned char), read_bytes, received_file);
    }

    fclose(received_file);

    std::cout << "泰山服务器：接收完成" << std::endl;

    //在此处添加运行测试程序的代码
    int exit_status = system("./test test_dataset.bin result.bin");

    // 检查测试程序运行结果
    if (exit_status == 0) {
        std::cout << "测试程序运行成功" << std::endl;

        // 发送测试结果文件
        FILE *result_file = fopen("result.bin", "rb");
        if (result_file == NULL) {
            perror("无法打开结果文件");
            return 1;
        }

        while (true) {
            int read_bytes = fread(buffer, sizeof(unsigned char), sizeof(buffer), result_file);
            if (read_bytes <= 0) {
                break;
            }
            int sent_bytes = send(client_socket, buffer, read_bytes, 0);
            if (sent_bytes < 0) {
                perror("发送数据失败");
                fclose(result_file);
                return 1;
            }
        }

        fclose(result_file);
        std::cout << "测试结果文件已发送" << std::endl;
    } else {
        std::cerr << "测试程序运行失败" << std::endl;
        return 1;
    }
    return 0;
}

