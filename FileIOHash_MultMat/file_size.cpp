#include <iostream>
#include <sys/stat.h>

using namespace std;

long GetFileSize(std::string filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

int main(){
 long size = GetFileSize("ubuntu-22.04.1-desktop-amd64.iso");
 cout << "File size is " << size << " bytes." << endl;
 return 0;
}
