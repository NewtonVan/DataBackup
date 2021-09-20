#include "utils.h"

#include <iostream>

using namespace std;

void testGetFileName() {
    cout << Utils::GetFileNameOnLinux("/mnt/sdcard/media/HAHAWTF.mp3") << endl;
}

int main() {
    testGetFileName();
}