#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "opencv2/opencv.hpp"

#include "face_detect_cnn.h"
#include "timer_local.h"

#ifdef WIN32
#include <io.h>
#else
#include <memory.h>
#include <dirent.h>
#endif

std::string GetFileName(std::string full_path) {
#ifdef WIN32
    full_path.replace(0, full_path.find_last_of('\\') + 1, "");
#else
	full_path.replace(0, full_path.find_last_of('/') + 1, "");
#endif
    return full_path;
}

/**
 * Use OpenCV to read image and get confidence score by {@code GetConfidenceScore128x128rbg}
 * @param name
 */
void PrintImage(const std::string &name) {
    using namespace cv;
    Mat mat = imread(name, IMREAD_COLOR);
    std::vector<Mat> img_channels;
    split(mat, img_channels);
    auto *img_data = new float[(int) (img_channels[0].total() + img_channels[1].total()
                                      + img_channels[2].total())];
    for (int i = 0; i < img_channels[2].total(); ++i) {
        img_data[i] = (float) img_channels[2].data[i] / 255.0f;
    }
    for (int i = 0; i < img_channels[1].total(); ++i) {
        img_data[i + img_channels[2].total()] = (float) img_channels[1].data[i] / 255.0f;
    }
    for (int i = 0; i < img_channels[0].total(); ++i) {
        img_data[i + img_channels[2].total() + img_channels[1].total()] =
                (float) img_channels[0].data[i] / 255.0f;
    }
    std::cout << "img_name: ";
    float out = GetConfidenceScore128x128rbg(img_data, img_channels[0].rows, img_channels[0].cols);
    std::cout << GetFileName(name) << "\nScore:    " << out << "\n\n";
}

/**
 * Get all files in {@code path} by full path
 * @param path given dir
 * @param files files found
 * @param recursively if find files recursively
 */
void GetAllFiles(const std::string &path, std::vector<std::string> &files, bool recursively = false) {
#ifdef WIN32
    long h_file;
    _finddata_t file_info{};
    std::string p;
    if ((h_file = _findfirst(p.assign(path).append("\\*").c_str(), &file_info)) != -1) {
        do {
            if ((file_info.attrib & _A_SUBDIR)) {
                if ((strcmp(file_info.name, ".") != 0 && strcmp(file_info.name, "..") != 0) && recursively) {
                    GetAllFiles(p.assign(path).append("\\").append(file_info.name), files);
                }
            } else {
                files.push_back(p.assign(path).append("\\").append(file_info.name));
            }
        } while (_findnext(h_file, &file_info) == 0);
        _findclose(h_file);
    }
#else
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (entry->d_type == DT_DIR){
            if (!recursively || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            std::string dir_new = path + "/" + entry->d_name;
            std::vector<std::string> temp_path;
            GetAllFiles(dir_new, temp_path);
            files.insert(files.end(), temp_path.begin(), temp_path.end());
        }else {
            std::string name = entry->d_name;
            std::string img_dir = path;
            img_dir += "/" + name;
            files.push_back(img_dir);
        }
    }
    closedir(dir);
#endif
	std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b) {return a < b;});
}

int main() {

    //the dir witch contains all the img files
    const std::string kImagesPath = "path/to/imgs";

    std::vector<std::string> file_names;
    GetAllFiles(kImagesPath, file_names);
    std::stringstream name_stream;
    name_stream << "All " << file_names.size() << " files";

    TIME_INIT
    TIME_START
    for (const auto &file_name : file_names) {
        PrintImage(file_name);
    }
    TIME_END(name_stream.str())

    std::cout.flush();
    return 0;
}