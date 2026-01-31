#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <cstring>
#include <string>
#include <unordered_map>


     
struct vec {
    int real_height = 0;
    int fake_height = 0;
    int real_width = 0;
    int fake_width = 1;
    int **data;

    vec(int height = 0, int width = 0)
            : fake_height(height), fake_width(width),
              real_height(height), real_width(width),
              data(nullptr) {
        if (height > 0 && width > 0) {
            data = new int *[height];
            for (int i = 0; i < height; ++i) {
                data[i] = new int[width];
                for (int j = 0; j < width; ++j) {
                    data[i][j] = 0;
                }
            }
        }
    }

    ~vec() {
        if (data) {
            for (int i = 0; i < fake_height; ++i) {
                delete[] data[i];
            }
            delete[] data;
        }
    }

    void resize_up() {
        fake_height *= 2;
        int **new_data = new int *[fake_height];
        for (int i = 0; i < fake_height; ++i) {
            new_data[i] = new int[fake_width];
            for (int j = 0; j < fake_width; ++j) {
                new_data[i][j] = 0;
            }
        }
        for (int i = 0; i < real_height; ++i) {
            for (int j = 0; j < real_width; ++j) {
                new_data[i][j] = data[i][j];
            }

        }
        for (int i = 0; i < fake_height / 2; ++i) {
            delete[] data[i];
        }
        delete[] data;
        data = new_data;
    }

    void resize_left() {
        fake_width *= 2;
        for (int j = 0; j < fake_height; ++j) {
            int *new_data = new int[fake_width];
            for (int i = 0; i < fake_width; ++i) {
                new_data[i] = 0;
            }
            for (int i = 0; i < real_width; ++i) {
                new_data[i] = data[j][i];
            }
            delete[] data[j];
            data[j] = new_data;
            //delete[] new_data;
        }
    }

    void push_right() {
        if (real_width == fake_width) {
            resize_left();
        }
        for (int i = 0; i < real_height; ++i) {
            data[i][real_width] = 0;
        }
        real_width++;
    }

    void push_left() {
        if (real_width == fake_width) {
            resize_left();
        }

        for (int i = 0; i < real_height; ++i) {
            for (int j = real_width; j > 0; --j) {
                data[i][j] = data[i][j - 1];
            }
            data[i][0] = 0;
        }
        real_width++;
    }

    void push_down() {
        if (real_height == fake_height) {
            resize_up();
        }
        for (int i = 0; i < fake_width; ++i) {
            data[real_height][i] = 0;
        }
        real_height++;
    }

    void push_top() {
        if (real_height == fake_height) {
            resize_up();
        }
        for (int i = real_height; i > 0; --i) {
            data[i] = data[i - 1];
        }
        data[0] = new int[fake_width];
        for (int i = 0; i < fake_width; ++i) {
            data[0][i] = 0;
        }
        real_height++;
    }

    vec &operator=(const vec &other) {
        if (this != &other) {
            if (data) {
                for (int i = 0; i < real_height; ++i) {
                    delete[] data[i];
                }
                delete[] data;
            }

            real_height = other.real_height;
            real_width = other.real_width;
            fake_height = other.fake_height;
            fake_width = other.fake_width;

            data = new int *[fake_height];
            for (int i = 0; i < fake_height; ++i) {
                data[i] = new int[fake_width];
                for (int j = 0; j < fake_width; ++j) {
                    if (i < real_height && j < real_width) {
                        data[i][j] = other.data[i][j];
                    } else {
                        data[i][j] = 0;
                    }
                }
            }

        }
        return *this;
    }

};

struct paths {
    char *input_fl_tsv;
    char *output_fl_bmp;


    paths() : output_fl_bmp(nullptr), input_fl_tsv(nullptr) {}

    ~paths() {
        delete[] output_fl_bmp;
        delete[] input_fl_tsv;
    }

    void setOut(const char *value) {

        output_fl_bmp = new char[strlen(value) + 1];
        strcpy(output_fl_bmp, value);
    }

    void setInp(const char *value) {

        input_fl_tsv = new char[strlen(value) + 1];
        strcpy(input_fl_tsv, value);
    }
};

struct pole {
    int width = 0;
    int height = 0;
    int frequency = 0;
    int counter = 0;
    vec pol;

    pole() : pol(10, 10) {

    }

    void init(int w = 1, int h = 1) {
        width = w;
        height = h;
        pol = vec(h, w);
    }

};

void read(pole &pole, paths &file_paths) {

    std::ifstream file(file_paths.input_fl_tsv);

    if (!file.is_open()) {
        printf("error reading tsv1");
        return;
    }
    char line[100];
    int i = 0;
    int x, y, color;
    int min_x = 1e9, min_y = 1e9;
    int max_x = -1e9, max_y = -1e9;
    while (file.getline(line, sizeof(line))) {
        char *token = strtok(line, " ");
        i = 0;
        while (token != nullptr) {
            if (i == 0) {
                x = atoi(token);
            }
            if (i == 1) {
                y = atoi(token);
            }
            if (i == 2) {
                color = atoi(token);
            }
            token = strtok(nullptr, " ");
            i++;
        }
        min_x = std::min(min_x, x);
        min_y = std::min(min_y, y);
        max_x = std::max(max_x, x);
        max_y = std::max(max_y, y);
    }
    file.close();

    std::ifstream file2(file_paths.input_fl_tsv);
    if (!file2.is_open()) {
        printf("error reading tsv2");
        return;
    }
    char line2[100];
    i = 0;
    x, y, color;

    pole.init(max_x - (min_x) + 100, max_y - (min_y) + 100);
    //std::cout << max_x - (min_x) + 300 << ' ';
    while (file2.getline(line2, sizeof(line2))) {
        char *token = strtok(line2, " ");
        i = 0;
        while (token != nullptr) {
            if (i == 0) {
                x = atoi(token);
            }
            if (i == 1) {
                y = atoi(token);
            }
            if (i == 2) {
                color = atoi(token);
            }
            token = strtok(nullptr, " ");
            i++;
        }
        //std::cout << pole.width;
        pole.pol.data[y - min_y + 99][x - min_x + 99] = color;
    }
    file2.close();


}


#pragma pack(push, 1) // Выравнивание структуры
struct BMPHeader {
    uint16_t fileType = 0x4D42; // 'BM'
    uint32_t fileSize = 0; // Размер файла в байтах
    uint16_t reserved1 = 0;
    uint16_t reserved2 = 0;
    uint32_t offsetData = 54 + 8 * 4; // Смещение до пикселй 54 - заголовок , 16 - палитра
};

struct DIBHeader {
    uint32_t size = 40; // Размер DIB заголовка
    int32_t width = 0; // Ширина изображения
    int32_t height = 0; // Высота изображения
    uint16_t planes = 1; // Количество цветовых плоскостей
    uint16_t bitCount = 4; // Количество бит на пиксель
    uint32_t compression = 0; // Без сжатия
    uint32_t sizeImage = 0; // Размер изображения
    int32_t xPixelsPerMeter = 0; // Горизонтальное разрешение
    int32_t yPixelsPerMeter = 0; // Вертикальное разрешение
    uint32_t colorsUsed = 16; // Количество используемых цветов
    uint32_t colorsImportant = 0; // Количество важных цветов
};

int f = 0;
#pragma pack(pop)

void createColoredRectangleBMP(std::string filename, int width, int height, pole &a) {
    std::ofstream bmpFile(std::to_string(f) + filename, std::ios::binary);
    BMPHeader bmpHeader;
    DIBHeader dibHeader;

    dibHeader.width = width;
    dibHeader.height = height;

    // Размер строки в байтах (выравнивание до 4 байт)
    int rowSize = (width + 1) / 2;
    rowSize = (rowSize + 3) & ~3; //округление  до ближайшего большего кратного 4



    int imageSize = rowSize * height;

    bmpHeader.fileSize = sizeof(BMPHeader) + sizeof(DIBHeader) + 8 * 4 + imageSize;

    // Записываем заголовки
    bmpFile.write((char *) (&bmpHeader), sizeof(bmpHeader));
    bmpFile.write((char *) (&dibHeader), sizeof(dibHeader));

    // Записываем палитру (8 цветов)
    uint8_t palette[8 * 4] = {0};
    palette[0] = 255;   //blue
    palette[1] = 255;   // green
    palette[2] = 255; //red
    palette[3] = 0;

    palette[4] = 0;
    palette[5] = 255;
    palette[6] = 0;
    palette[7] = 0;

    palette[8] = 250;
    palette[9] = 60;
    palette[10] = 150;
    palette[11] = 0;

    palette[12] = 0;
    palette[13] = 255;
    palette[14] = 255;
    palette[15] = 0;

    palette[16] = 0;
    palette[17] = 0;
    palette[18] = 0;
    palette[19] = 0;
    // Записываем палитру в файл
    bmpFile.write((char *) (palette), 8 * 4);


    uint8_t imageData[rowSize * height] = {0}; // 4 бита на пиксель
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * rowSize) + (x / 2);
            if (x % 2 == 0) {
                imageData[index] |= (a.pol.data[y][x]) << 4;
            } else {
                imageData[index] |= (a.pol.data[y][x]);
            }
        }
    }

    // Записываем пиксели
    bmpFile.write((char *) (imageData), rowSize * height);

    bmpFile.close();
    std::cout << "BMP made " << filename << std::endl;
}


bool updater(pole &pole, paths &paths) {
    bool ans = false;
    int update_left = 0, update_right = 0, update_top = 0, update_down = 0;
    for (int i = 0; i < pole.height; i++) {
        for (int j = 0; j < pole.width; j++) {
            int ii = i + update_top;
            int jj = j + update_left;
            if (pole.pol.data[ii][jj] < 4) {
                continue;
            }
            if (ii == 0 && update_top == 0) {
                pole.pol.push_top();
                update_top = 1;
            }

            if (ii == pole.height - 1 && update_down == 0) {
                pole.pol.push_down();
                update_down = 1;
            }

            if (jj == 0 && update_left == 0) {
                pole.pol.push_left();
                update_left = 1;
            }


            if (jj == pole.width - 1 && update_right == 0) {
                pole.pol.push_right();
                update_right = 1;
            }


            ii = i + update_top;
            jj = j + update_left;
            pole.pol.data[ii][jj] -= 4;
            if (pole.frequency != 0 && pole.counter % pole.frequency == 0) {
                f++;
                createColoredRectangleBMP(paths.output_fl_bmp, pole.width, pole.height, pole);
            }

            pole.pol.data[ii - 1][jj]++;
            pole.pol.data[ii][jj - 1]++;
            pole.pol.data[ii + 1][jj]++;
            pole.pol.data[ii][jj + 1]++;
            pole.counter--;

            ans = true;

        }
    }
    pole.height += update_top + update_down;
    pole.width += update_left + update_right;
    return ans;
}

bool ReadArgs(int argc, char *argv[], pole &pole, paths &paths) {
    //std::cout << argc << ' ';
    for (int i = 1; i < argc; ++i) {
        //std::cout << i << ' ' << argv[i] << ' ';


        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 2 < argc && strcmp(argv[i + 1], "=") == 0) {

                paths.setOut(argv[i + 2]);
                //std::cout << paths.output_fl_bmp << std::endl;
            } else {
                return false;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 2 < argc && strcmp(argv[i + 1], "=") == 0) {
                paths.setInp(argv[i + 2]);
                //std::cout << argv[i+2] << std::endl;
                //std::cout << 1 << std::endl;
            } else {
                return false;
            }
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--max-iter") == 0) {

            if (i + 2 < argc && strcmp(argv[i + 1], "=") == 0) {
                pole.counter = atoi(argv[i + 2]);//std::cout << i << std::endl;
            } else {
                return false;
            }
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--freq") == 0) {
            if (i + 2 < argc && strcmp(argv[i + 1], "=") == 0) {
                pole.frequency = atoi(argv[i + 2]);
            } else {
                return false;
            }
        } else {
            return false;
        }
        i += 2;

    }
    return true;
}

int main(int argc, char *argv[]) {
    //std::vector<std::vector<int>>a;
    //a.assign(200, std::vector<int>(100,2));
    pole pol;
    paths paths;
    if (!ReadArgs(argc, argv, pol, paths)) {
        printf("logs error");
        return 0;
    }
    //a.exe -i = pole.tsv -o = img.bmp -m = 10000000
    read(pol, paths);

    std::cout << pol.counter << std::endl;
    while (pol.counter > 0 && updater(pol, paths)) {}
    std::cout << pol.width << std::endl;
    createColoredRectangleBMP(paths.output_fl_bmp, pol.width, pol.height, pol);
    return 0;
}
