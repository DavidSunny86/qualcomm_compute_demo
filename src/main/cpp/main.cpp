/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <errno.h>
#include <cassert>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>


#define _ENABLE_DSP_

#ifdef _ENABLE_DSP_
// enable dsp support
#ifndef HETCOMPUTE_HAVE_QTI_DSP
    #define HETCOMPUTE_HAVE_QTI_DSP
#endif
// enable gpu support
//#define HETCOMPUTE_HAVE_GPU

#include <vector>
#include "hetcompute/hetcompute.hh"

#define BLOCK_SIZE  2048

#endif

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

using namespace std;

struct BoundBox{
    hetcompute::float3 pos0;
    hetcompute::float3 pos1;
    hetcompute::float3 pos2;
    hetcompute::float3 pos3;
    hetcompute::float3 pos4;
    hetcompute::float3 pos5;
    hetcompute::float3 pos6;
    hetcompute::float3 pos7;

public:
    inline void Init(){
        hetcompute::float3* start = (hetcompute::float3*)this;

        for(int i=0;i<8;i++){
            start[i].x = rand() * 25.01f;
            start[i].y = rand() * 59.01f;
            start[i].z = rand() * 63.01f;
            start[i] = hetcompute::normalize(start[i]);
        }
    }
};

typedef void (*pForIt)();

static int DspFun(BoundBox* bb, float* v){
    *v = hetcompute::dot(bb->pos0, bb->pos1) + hetcompute::dot(bb->pos1, bb->pos2);
    return 0;
}
static int DspFunPrint(float* v){
    for(int i=0;i<BLOCK_SIZE;i++){
        LOGI("$$$  dspPrint: %i %f", i, v[i]);
    }
    return 0;
}

extern "C" JNIEXPORT void JNICALL Java_com_yw_agaDemo_UnityPlayerActivity_TestQualcommComputeDspOcclusionCull(){

#ifdef _ENABLE_DSP_
    hetcompute::runtime::init();


#endif

#ifdef _ENABLE_DSP_
    std::vector<BoundBox> vin(BLOCK_SIZE);
    std::vector<bool> isContain(BLOCK_SIZE, 0);
    std::vector<float> dotRes(BLOCK_SIZE, 0.0f);


    auto boundBoxBuff = hetcompute::create_buffer<BoundBox>(BLOCK_SIZE);
    auto valueBuff = hetcompute::create_buffer<float>(BLOCK_SIZE);
    auto bbb = (BoundBox*) new BoundBox[BLOCK_SIZE];
    auto vb = (float*) new float[BLOCK_SIZE];


    int i=0;
    boundBoxBuff.acquire_wi();
    for(auto it = vin.begin(); it != vin.end(); it++){
        srand(i * 3);
        BoundBox boundBox;
        boundBox.Init();
        vin[i] = boundBox;
        bbb[i] = boundBox;
        boundBoxBuff[i] = boundBox;
        i++;
    }
    boundBoxBuff.release();

    int loopTime = 10;

    LOGI("$$$ DSP start ...");
    for(int i=0;i<loopTime;i++) {
        /*
        hetcompute::pfor_each(size_t(0), vin.size(), [&vin, &dotRes](size_t i) {
            BoundBox bb = vin[i];
            dotRes[i] = hetcompute::dot(bb.pos1, bb.pos2)
                + hetcompute::dot(bb.pos2, bb.pos3)
                + hetcompute::dot(bb.pos3, bb.pos4) + hetcompute::dot(bb.pos4, bb.pos5)
                + hetcompute::dot(bb.pos5, bb.pos6) + hetcompute::dot(bb.pos6, bb.pos7);

        });
         */


        //hetcompute::buffer_ptr<BoundBox>, hetcompute::buffer_ptr<float>
        auto dspKernel = hetcompute::create_dsp_kernel<>(DspFun);


        auto dspTask = hetcompute::create_task(dspKernel, &boundBoxBuff[0], &valueBuff[0]);
        //auto dspTask1 = hetcompute::create_task(DspFunPrint, vb);

        //dspTask->then(dspTask1);

        dspTask->launch();
        dspTask->wait_for();

    }

    LOGI("$$$ DSP end ...");


    for(int i=0;i<BLOCK_SIZE;i++) {
        float d = vb[i];
        LOGI("$$$ print %i dot: %f", i, d);
    }

    valueBuff.acquire_rw();
    for(int i=0;i<BLOCK_SIZE;i++) {
        float d = valueBuff[i];
        LOGI("$$$ print vbuff %i dot: %f", i, d);
    }
    valueBuff.release();


    LOGI("$$$ CPU start ...");
    for(int i=0;i<loopTime;i++) {
        for (int i = 0; i < BLOCK_SIZE; i++) {
            BoundBox bb = vin[i];
            dotRes[i] = hetcompute::dot(bb.pos1, bb.pos2)
                + hetcompute::dot(bb.pos2, bb.pos3)
                + hetcompute::dot(bb.pos3, bb.pos4) + hetcompute::dot(bb.pos4, bb.pos5)
                + hetcompute::dot(bb.pos5, bb.pos6) + hetcompute::dot(bb.pos6, bb.pos7);
        }
    }
    LOGI("$$$ CPU end ...");

#endif


#ifdef _ENABLE_DSP_
    hetcompute::runtime::shutdown();
#endif

}

extern "C" JNIEXPORT void JNICALL Java_com_yw_agaDemo_UnityPlayerActivity_TestQualcommComputeDsp(){

#ifdef _ENABLE_DSP_
    hetcompute::runtime::init();
#endif

#ifdef _ENABLE_DSP_
    std::vector<float_t> vin(1024, 0);
    LOGI("$$$ start ...");
    hetcompute::pfor_each(size_t(0), vin.size(), [&vin](size_t i) {
//        vin[i] = 2 * i * i;
        vin[i] = cos(pow(sin(pow(i,2)), 2));
    });

    LOGI("$$$ end ...");

    for(int i=0;i<1024;i++) {
        float a = vin[i];
        LOGI("$$$ print %f", (float) a);
    }
#endif


#ifdef _ENABLE_DSP_
    hetcompute::runtime::shutdown();
#endif

}

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }
    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
                 ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;

}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->animating = 1;
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(android_app* app) {

  if(!app)
    return nullptr;

  typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
  void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
  PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
      dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
  if (getInstanceForPackageFunc) {
    JNIEnv* env = nullptr;
    app->activity->vm->AttachCurrentThread(&env, NULL);

    jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
    jmethodID midGetPackageName = env->GetMethodID(android_content_Context,
                                                   "getPackageName",
                                                   "()Ljava/lang/String;");
    jstring packageName= (jstring)env->CallObjectMethod(app->activity->clazz,
                                                        midGetPackageName);

    const char *nativePackageName = env->GetStringUTFChars(packageName, 0);
    ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
    env->ReleaseStringUTFChars(packageName, nativePackageName);
    app->activity->vm->DetachCurrentThread();
    if (mgr) {
      dlclose(androidHandle);
      return mgr;
    }
  }

  typedef ASensorManager *(*PF_GETINSTANCE)();
  PF_GETINSTANCE getInstanceFunc = (PF_GETINSTANCE)
      dlsym(androidHandle, "ASensorManager_getInstance");
  // by all means at this point, ASensorManager_getInstance should be available
  assert(getInstanceFunc);
  dlclose(androidHandle);

  return getInstanceFunc();
}


/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = AcquireASensorManagerInstance(state);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
                                        engine.sensorManager,
                                        ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
                                    engine.sensorManager,
                                    state->looper, LOOPER_ID_USER,
                                    NULL, NULL);

    if (state->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
                                      (void**)&source)) >= 0) {

            // Process this event.
            if (source != NULL) {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                                       &event, 1) > 0) {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                             event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }

        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);
        }
    }
}
//END_INCLUDE(all)
