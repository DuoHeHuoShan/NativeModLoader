#include <jni.h>
#include <BNM/Loading.hpp>
#include "BNM/Class.hpp"
#include "BNM/MethodBase.hpp"
#include "BNM/Method.hpp"
#include "BNM/Utils.hpp"
#include "BNM/ClassesManagement.hpp"
#include "BNM/UnityStructures.hpp"

struct HFFTimer : BNM::UnityEngine::MonoBehaviour {
    BNM_CustomClass(HFFTimer, BNM::CompileTimeClassBuilder(nullptr, OBFUSCATE_BNM("Delegates")).Build(), {}, {});
    void OnGUI() {
        BNM_CallCustomMethodOrigin(OnGUI, this);

    }
    BNM_CustomMethod(OnGUI, false, BNM::GetType<void>(), "OnGUI");
    BNM_CustomMethodSkipTypeMatch(OnGUI);
    BNM_CustomMethodMarkAsInvokeHook(OnGUI);
};

void OnLoaded() {
    using namespace BNM;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);


    // Load BNM by finding the path to libil2cpp.so
    BNM::Loading::TryLoadByJNI(env);

    // Or load using KittyMemory (as an example)
    // Example_07();

    BNM::Loading::AddOnLoadedEvent(OnLoaded);

    return JNI_VERSION_1_6;
}