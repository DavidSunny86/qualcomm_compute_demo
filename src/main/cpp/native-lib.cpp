#include <jni.h>
#include <string>
#include <android/log.h>
#include <GLES/gl.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/window.h>
#include <android/native_window_jni.h>
#define LOG(p, str, fmt)\
    __android_log_print(p, str, fmt);

extern "C"{
    jboolean JNICALL Java_com_example_tetsndk_MainActivity_initCrashPad(JNIEnv *env, jobject obj, jstring handlePath, jstring dbPath, jstring metricPath, jstring urlStr, jint mode, jboolean bUpload);
void JNICALL Java_com_example_tetsndk_MainActivity_MakeCrash(JNIEnv *env, jobject obj);
void JNICALL Java_com_example_tetsndk_MainActivity_MakeCrashWithoutDump(JNIEnv *env, jobject obj);

void JNICALL Java_com_example_tetsndk_MainActivity_InitBreakPad(JNIEnv *env, jobject obj, jstring dumpPath);
void JNICALL Java_com_example_tetsndk_MainActivity_CrashBreakPad(JNIEnv *env, jobject obj);
}

jstring TestCrash(JNIEnv *env){
    std::string hello = "Hello from C++";
    char* nullData = nullptr;//  new char[32];
    char* data = new char[32];
    memcpy(data, nullData, 16);
    return env->NewStringUTF(hello.c_str());
}
extern "C" JNIEXPORT void JNICALL
Java_com_example_tetsndk_MainActivity_compileShader(        JNIEnv *env,
                                                            jobject, jobject  surface/* this */, jstring src){
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    auto win = ANativeWindow_fromSurface(env,surface) ;

    auto curCtx = eglGetCurrentContext();
    EGLContext ctx = eglCreateContext(display, EGLConfig(), curCtx, NULL);
    EGLSurface sf = eglCreateWindowSurface(display, EGLConfig(), win, NULL);
    eglMakeCurrent(display, sf, sf, ctx);
    GLuint shaderId = glCreateShader(GL_FRAGMENT_SHADER);
    jboolean isCopy;

    const char* shaderSrc = env->GetStringUTFChars(src, &isCopy);
    int len = env->GetStringUTFLength(src);
    GLchar const* files[] = { shaderSrc };
    GLint lengths[] = { len };
    glShaderSource(shaderId, 1, files, lengths);

    //auto prog = glCreateProgram();
    //glAttachShader(prog, shaderId);

    //char* getShaderSrc;
    //glGetShaderSource(shaderId, 1, lengths, getShaderSrc);

}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_tetsndk_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */, jint v) {

//    jstring str = env->NewStringUTF("");
//    jobject obj;
//    Java_com_example_tetsndk_MainActivity_initCrashPad(env, obj, str, str, str,str, 0);
//    return env->NewStringUTF("");
    return TestCrash(env);
}

