#define MB(x) (1048576 * x)
#define GB(x) (1073741824 * x)
#define CHUNK_SIZE MB(512) // 512MiB chunk
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstddef>
#include <algorithm>

using namespace std;

unsigned char file_sig_wps[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1}; // MS Office file signature
unsigned char file_sig_office[] = {0x50, 0x4b, 0x03, 0x04, 0x0a};
size_t file_sig_max_length = max(sizeof(file_sig_wps), sizeof(file_sig_office));
unsigned char sig_word[] = {0x77,0x6f,0x72,0x64};
unsigned char sig_excel[] = {0x77,0x6f,0x72,0x6b,0x73,0x68,0x65,0x65,0x74,0x73};

template <class T1, class T2>
bool match(const T1* data, const T2* cmp, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        if (data[i] != cmp[i])
            return false;
    }
    return true;
}

const char* find_file_end(const char* begin, const char* end) {
    int count = 0; // expect 0x50
    for (const char* ptr = begin + 4000; ptr < end; ++ptr) {
        while (*ptr == '\0' and ptr < end) {
            ++ptr;
            ++count;
        }
        if (count >= 0x50)
            return ptr;
        count = 0;
    }
    return min(begin + MB(10), end);
}

int main(int argc, char* argv[]) { // the first argument is the disk image file / disk device
    if (argc != 2) {
        printf("at least/only 1 argument is required.\n");
        return 1;
    }
    ifstream disk_img(argv[1], ios::binary | ios::in);
    if (disk_img.bad()) {
        printf("file not found.\n");
        return 1;
    }
    if (disk_img.fail()) {
        printf("unable to open file.\n");
        return 1;
    }
    
    size_t file_size = 0, chunk_c = 0, chunk_i = 0;
    size_t xls_found = 0, doc_found = 0, zip_found = 0, wps_found = 0, files_found = 0;
    ofstream document;
    {
        auto begin = disk_img.tellg();
        disk_img.seekg(0, disk_img.end);
        auto end = disk_img.tellg();
        file_size = end - begin;
        disk_img.seekg(0, disk_img.beg);
        chunk_c = file_size / CHUNK_SIZE;
    }
    printf("Image size: %.2fGB\n", file_size/(double)GB(1));
    printf("Chunk size: %.2fMB\n", CHUNK_SIZE/(double)MB(1));
    printf("Allocating chunk...\n");
    char *content_buf = new char[CHUNK_SIZE];
    while(!(disk_img.eof() or disk_img.fail())) {
        size_t pos = (size_t)disk_img.tellg();
        size_t i = 0;
        size_t chunk_size;
        disk_img.read(content_buf, CHUNK_SIZE);
        if (disk_img.fail()) {
            chunk_size = file_size - pos - 1;
        }
        else {
            chunk_size = (size_t)disk_img.tellg() - pos;
        }
        printf("(%zu/%zu) offset: %zx\n", chunk_i, chunk_c, pos);
        while (i + file_sig_max_length < chunk_size) {
            const char* begin, *end, *ext;
            ptrdiff_t doc_size;
            if (match(&content_buf[i], file_sig_office, sizeof(file_sig_office))) {
                printf("found office file signature at %zx", pos + i);
                ++files_found;
                begin = &content_buf[i];
                end = find_file_end(begin, begin+chunk_size-1);
                doc_size = end - begin;
                
                if (std::search(begin, end, sig_excel, sig_excel + sizeof(sig_excel)) != end) {
                    ext = ".xls";
                    ++xls_found;
                }
                else if (std::search(begin, end, sig_word, sig_word + sizeof(sig_word)) != end) {
                    ext = ".doc";
                    ++doc_found;
                }
                else { // pptx or corrupted
                    ext = ".pptx";
                    ++zip_found;
                }
                printf(", likely a '%s' file.\n", ext);
                document.open("found/"+std::to_string(files_found)+string(ext), ios::binary | ios::out);
                document.write(begin, doc_size);
                document.flush();
                document.close();
                i += doc_size + sizeof(file_sig_office);
            }
            else if (match(&content_buf[i], file_sig_wps, sizeof(file_sig_wps))) {
                printf("found wps file signature at %zx\n", pos + i);
                ++files_found;
                ++wps_found;
                begin = &content_buf[i];
                end = find_file_end(begin, begin+chunk_size-1);
                doc_size = end - begin;
                document.open("found/"+std::to_string(files_found)+".wps", ios::binary | ios::out);
                document.write(begin, doc_size);
                document.flush();
                document.close();
                i += sizeof(file_sig_wps);
            }
            else {
                ++i;
            }
        }
        disk_img.seekg(disk_img.tellg() - (streampos)(file_sig_max_length - 1)); // thereare file_sig_max_length - 1 unused in each chunk
        ++chunk_i;
    }
    delete[] content_buf;
    disk_img.close();
    printf("Finished recovery, found %zu files (Excel: %zu, Word: %zu, Zip: %zu, WPS: %zu).\n", files_found, xls_found, doc_found, zip_found, wps_found);
}