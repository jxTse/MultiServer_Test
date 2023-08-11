#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
  // 假设要测试的数据集为以下浮点数
  float test_dataset[5] = {1.23, 4.56, 7.89, 0.12, 3.45};

  // 创建本地PC的socket
  int local_socket, x86_socket, taishan_socket;
  struct sockaddr_in local_addr, x86_addr, taishan_addr;
  int server_port = 12345; // 用于传输数据的端口，可以自行指定

  // 设置x86服务器的IP地址和端口号
  x86_addr.sin_family = AF_INET;
  x86_addr.sin_addr.s_addr = inet_addr("171.16.50.65");
  x86_addr.sin_port = htons(server_port);

  // 设置泰山服务器的IP地址和端口号
  taishan_addr.sin_family = AF_INET;
  taishan_addr.sin_addr.s_addr = inet_addr("211.71.76.181");
  taishan_addr.sin_port = htons(server_port);

  local_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (local_socket < 0) {
    perror("无法创建socket");
    return 1;
  }

  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = htons(server_port);

  // 绑定本地PC的socket
  if (bind(local_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) <
      0) {
    perror("绑定socket失败");
    close(local_socket);
    return 1;
  }

  // 创建x86socket
  x86_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(x86_socket < 0){
  	perror("无法创建x86socket");
	return 1;
  }

  // 发起连接到x86服务器
  if (connect(x86_socket, (struct sockaddr *)&x86_addr, sizeof(x86_addr)) <0) {
    perror("连接x86服务器失败");
    close(x86_socket);
    return 1;
  }
  std::cout << "已连接到x86服务器" << std::endl;

  // 创建文件并将测试数据集保存到文件中
  std::ofstream dataset_file("test_dataset.bin", std::ios::binary);
  dataset_file.write(reinterpret_cast<const char *>(test_dataset), sizeof(test_dataset));
  dataset_file.close();

  // 发送测试数据集文件到x86服务器
  FILE *file_to_send_x86 = fopen("test_dataset.bin", "rb");
  if (file_to_send_x86 == NULL) {
    perror("无法打开文件");
    close(local_socket);
    close(x86_socket);
    return 1;
  }

  unsigned char buffer[sizeof(test_dataset)];
  int read_bytes;

  while ((read_bytes = fread(buffer, sizeof(unsigned char),sizeof(test_dataset), file_to_send_x86)) > 0) {
      int sent_bytes = send(x86_socket, buffer, read_bytes, 0);
      if (sent_bytes < 0) {
      	perror("发送数据到X86服务器失败");
      	fclose(file_to_send_x86);
      	close(local_socket);
	close(x86_socket);
	return 1;
      }
    // 输出发送的字节数
    std::cout << "已发送" << sent_bytes << "字节的数据" << std::endl;
  }

  fclose(file_to_send_x86);

  // 接收x86服务器传回的测试结果文件
  FILE *x86_file_to_receive = fopen("result.bin", "wb");
  if (x86_file_to_receive == NULL) {
    perror("无法创建文件");
    return 1;
  }

  unsigned char x86_buffer[1024];
  int x86_read_bytes;

  while ((x86_read_bytes =
              recv(x86_socket, x86_buffer, sizeof(x86_buffer), 0)) > 0) {
    fwrite(x86_buffer, sizeof(unsigned char), x86_read_bytes,
           x86_file_to_receive);
  }
  std::cout << "接受到" << read_bytes << "字节的数据" << std::endl;
  fclose(x86_file_to_receive);

  // 关闭与x86服务器的连接
  close(x86_socket);

  // 创建泰山socket
  taishan_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(taishan_socket < 0){
  	perror("无法创建泰山socket");
	return 1;
  }

  std::cout << "等待连接到泰山服务器..." << std::endl;

  // 发起连接到泰山服务器
  if (connect(taishan_socket, (struct sockaddr *)&taishan_addr,sizeof(taishan_addr)) < 0) {
    perror("连接泰山服务器失败");
    close(local_socket);
    return 1;
  }
  std::cout << "已连接到泰山服务器" << std::endl;

  // 发送测试数据集文件到泰山服务器
  FILE *file_to_send_taishan = fopen("test_dataset.bin", "rb");
  if (file_to_send_taishan == NULL) {
    perror("无法打开文件");
    close(local_socket);
    return 1;
  }

  while ((read_bytes = fread(buffer, sizeof(unsigned char),
                             sizeof(test_dataset), file_to_send_taishan)) > 0) {
    if (send(taishan_socket, buffer, read_bytes, 0) < 0) {
      perror("发送数据到泰山服务器失败");
      fclose(file_to_send_taishan);
      close(local_socket);
      return 1;
    }
  }

  fclose(file_to_send_taishan);

  // 关闭与泰山服务器的连接
  close(taishan_socket);

  // 关闭本地PC的socket
  close(local_socket);

  std::cout << "测试数据集已保存到文件 test_dataset.bin" << std::endl;



  // 接收泰山服务器传回的测试结果文件
  FILE *taishan_file_to_receive = fopen("taishan_test_result.bin", "wb");
  if (taishan_file_to_receive == NULL) {
    perror("无法创建文件");
    return 1;
  }

  unsigned char taishan_buffer[1024];
  int taishan_read_bytes;

  while ((taishan_read_bytes = recv(taishan_socket, taishan_buffer,
                                    sizeof(taishan_buffer), 0)) > 0) {
    fwrite(taishan_buffer, sizeof(unsigned char), taishan_read_bytes,
           taishan_file_to_receive);
  }

  fclose(taishan_file_to_receive);

  // 比较两个测试结果文件

  // 打开两个文件，逐字节比较
  FILE *file_x86 = fopen("result.bin", "rb");
  FILE *file_taishan = fopen("taishan_test_result.bin", "rb");

  if (file_x86 == NULL || file_taishan == NULL) {
    perror("无法打开文件");
    return 1;
  }

  int byte_x86, byte_taishan;
  bool files_equal = true;
  int byte_counter = 0;

  while (true) {
    byte_x86 = fgetc(file_x86);
    byte_taishan = fgetc(file_taishan);

    if (byte_x86 == EOF && byte_taishan == EOF) {
      break;
    }

    if (byte_x86 != byte_taishan) {
      files_equal = false;
      break;
    }

    byte_counter++;
  }

  fclose(file_x86);
  fclose(file_taishan);

  // 打印结果
  if (files_equal) {
    std::cout << "两个测试结果文件相同，比较通过！" << std::endl;
  } else {
    std::cout << "两个测试结果文件不相同，比较未通过！" << std::endl;
  }

  return 0;
}

