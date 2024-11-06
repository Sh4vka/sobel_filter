#include <iostream>
#include <vector>
#include <thread>
#include <cmath>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "/home/matthew/lessons/OS/sobel_filter/stb_image.h"
#include "/home/matthew/lessons/OS/sobel_filter/stb_image_write.h"


struct ThreadData {
    stbi_uc* image;
    std::vector<unsigned char> result;
    int width, height, channels;
    int start_row, end_row;
};

// Функция для применения фильтра Собела к изображению
void applySobelFilter(ThreadData &data) {
    int gx, gy;
    
    // Ядро фильтра Собела по оси X
    int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    
    // Ядро фильтра Собела по оси Y
    int kernelY[3][3] = {
        {1, 2, 1},
        {0, 0, 0},
        {-1, -2, -1}
    };

    auto start = std::chrono::high_resolution_clock::now();  // Засекаем время начала

    for (int i = data.start_row; i < data.end_row; i++) {
        for (int j = 1; j < data.width - 1; j++) {
            gx = 0;
            gy = 0;
            
            // Применяем ядра фильтра Собела к пикселям изображения
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int pixel = data.image[((i + k) * data.width + (j + l)) * 3];
                    gx += pixel * kernelX[k + 1][l + 1];
                    gy += pixel * kernelY[k + 1][l + 1];
                }
            }
            
            // Вычисляем градиент изображения
            int magnitude = (int) std::sqrt(gx * gx + gy * gy);
            data.result[(i * data.width + j) * 3] = magnitude; // Градиент по оси R
            data.result[(i * data.width + j) * 3 + 1] = magnitude; // Градиент по оси G
            data.result[(i * data.width + j) * 3 + 2] = magnitude; // Градиент по оси B
        }
    }

    auto end = std::chrono::high_resolution_clock::now();  // Засекаем время конца
    std::chrono::duration<double> duration = end - start;  // Рассчитываем время выполнения

    std::cout << "Thread processed rows " << data.start_row << " to " << data.end_row
              << " in " << duration.count() << " seconds." << std::endl;  // Выводим время выполнения
}

int main() {
    int width, height, channels;
    
    // Загрузка изображения с использованием stbi_load, возвращается указатель на массив
    unsigned char* image = stbi_load("/home/matthew/lessons/OS/sobel_filter/olen_art_vektor_134088_1920x1080.jpg", &width, &height, &channels, 0);
    if (!image) {
        std::cerr << "Error loading image" << std::endl;
        return -1;
    }

    // Результирующий вектор для обработки изображения
    std::vector<unsigned char> result(width * height * 3);

    // Разделим работу на несколько потоков
    int num_threads = 4;
    std::vector<std::thread> threads;
    std::vector<ThreadData> thread_data(num_threads);

    int rows_per_thread = height / num_threads;

    // Разделяем работу между потоками
    for (int i = 0; i < num_threads; i++) {
        thread_data[i] = {image, std::vector<unsigned char>(width * height * 3), width, height, channels, i * rows_per_thread, (i == num_threads - 1) ? height : (i + 1) * rows_per_thread};
        threads.push_back(std::thread(applySobelFilter, std::ref(thread_data[i])));
    }

    // Ожидаем завершения всех потоков
    for (auto &t : threads) {
        t.join();
    }

    // Объединяем результаты
    for (int i = 0; i < num_threads; i++) {
        for (int row = thread_data[i].start_row; row < thread_data[i].end_row; row++) {
            for (int col = 0; col < width; col++) {
                result[(row * width + col) * 3] = thread_data[i].result[(row * width + col) * 3];
                result[(row * width + col) * 3 + 1] = thread_data[i].result[(row * width + col) * 3 + 1];
                result[(row * width + col) * 3 + 2] = thread_data[i].result[(row * width + col) * 3 + 2];
            }
        }
    }

    // Записываем результат в файл
    stbi_write_jpg("sobel_result.jpg", width, height, channels, result.data(), 100);

    // Освобождаем память, выделенную для изображения
    stbi_image_free(image);

    return 0;
}
