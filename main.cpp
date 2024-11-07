#include <iostream>
#include <math.h>
#include <chrono>
#include <pthread.h>

using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "/home/matthew/lessons/OS/sobel_filter/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "/home/matthew/lessons/OS/sobel_filter/stb_image_write.h"

struct ThreadData {
    int start_row;
    int end_row;
    int width;
    int height;
    unsigned char* input_img;
    unsigned char* output_img;
};

// Ядро фильтра Собела по оси X
const int kernelX[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};
    
// Ядро фильтра Собела по оси Y
const int kernelY[3][3] = {
    {1, 2, 1},
    {0, 0, 0},
    {-1, -2, -1}
};

// Функция для применения фильтра Собела к изображению
void* applySobelFilter(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int width = data->width;
    int height = data->height;
    int gx, gy;
    
    
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 1; j < width - 1; j++) {
            gx = 0;
            gy = 0;
            
            // Применяем ядра фильтра Собела к пикселям изображения
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int pixel = data->input_img[(k + i) * width + (l + j)];
                    gx += pixel * kernelX[k + 1][l + 1];
                    gy += pixel * kernelY[k + 1][l + 1];
                }
            }
            
            // Вычисляем градиент изображения
            double gradient = sqrt(gx * gx + gy * gy);
            if (gradient > 255) gradient = 255;
            if (gradient < 0) gradient = 0;
            data->output_img[i * data->width + j] = (unsigned char)gradient;
        }
    }
    pthread_exit(nullptr);
}

int main() 
{
    int width, height, channels;
    const int num = 6; 
    int threads[] = {1, 2, 4, 8, 16, 32};
    
    unsigned char* input_img = stbi_load("/home/matthew/lessons/OS/sobel_filter/adv_times.png", &width, &height, &channels, 1);


    unsigned char* output_img = new unsigned char[width * height * channels];


    for (int i = 0; i < num; i++){

        int rows = (height - 1) / threads[i];
        int start_row = 1;
        int end_row = rows;

        pthread_t threads_list[threads[i]]; 
        ThreadData thread_data[threads[i]];

        auto start = chrono::steady_clock::now(); 

        for (int j = 0; j < threads[i]; j++){
            thread_data[j].start_row = start_row;
            thread_data[j].end_row = end_row;

            thread_data[j].width = width;
            thread_data[j].height = height;
            thread_data[j].input_img = input_img;
            thread_data[j].output_img = output_img;

            pthread_create(&threads_list[j], nullptr, applySobelFilter, &thread_data[j]);

            start_row = end_row ;
            end_row += rows;
        }

        for(int j = 0; j < threads[i]; j++)
		    pthread_join(threads_list[j], nullptr);

        auto end = chrono::steady_clock::now();
        auto elapsed_ms = chrono::duration<float, milli>(end - start);

        cout << "Time taken with " << threads[i] << " thread(s): " << elapsed_ms.count() << " ms" << endl;
    }
    stbi_write_png("test.png", width, height, 1, output_img, width);

    stbi_image_free(input_img);
    return 0;
}