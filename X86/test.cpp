#include <iostream>
#include <fstream>
#include <vector>

// 假设测试数据是一系列的整数，每个整数占据4个字节
const int INT_SIZE = sizeof(int);

int main() {
    std::ifstream inputFile("test_dataset.bin", std::ios::binary);

    if (!inputFile) {
        std::cerr << "Failed to open test_dataset.bin" << std::endl;
        return 1;
    }

    std::vector<int> testData;
    int num;

    // 从二进制文件读取数据
    while (inputFile.read(reinterpret_cast<char*>(&num), INT_SIZE)) {
        testData.push_back(num);
    }

    inputFile.close();

    // 在这里添加你的测试逻辑，对testData进行测试并生成测试结果
    std::vector<int> testResult;

    for (int value : testData) {
        // 假设测试逻辑是简单的将每个值乘以2作为测试结果
        int result = value * 2;
        testResult.push_back(result);
    }

    // 将测试结果写入二进制文件
    std::ofstream outputFile("result.bin", std::ios::binary);

    if (!outputFile) {
        std::cerr << "Failed to open result.bin" << std::endl;
        return 1;
    }

    for (int result : testResult) {
        outputFile.write(reinterpret_cast<const char*>(&result), INT_SIZE);
    }

    outputFile.close();

    std::cout << "Test results written to result.bin" << std::endl;

    return 0;
}

