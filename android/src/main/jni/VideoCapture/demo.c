
#include "dll_video_capture.h"
#include "jni.h"


VideoCaptureFunc *func;
JNIEXPORT void Java_project_xplat_test_nativeDemoEntry(){
    
    video_capture_QueryInterface(&func);
}