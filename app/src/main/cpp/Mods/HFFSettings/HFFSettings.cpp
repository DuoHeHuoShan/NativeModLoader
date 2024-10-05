#include <jni.h>
#include <BNM/Loading.hpp>
#include "BNM/Class.hpp"
#include "BNM/MethodBase.hpp"
#include "BNM/Method.hpp"
#include "BNM/Utils.hpp"
#include "BNM/Field.hpp"
#include <map>
#include <string>
#include <fstream>
#include <unistd.h>

struct Vector2
{
    float x;
    float y;
};

std::map<std::string, std::string> HFFSettings;

BNM::Class HumanControls;
BNM::MethodBase HumanControls_ReadInput;
BNM::Field<float> HumanControl_lookHScale;
BNM::Field<float> HumanControl_lookVScale;

float lookHScale = 12;
float lookVScale = 5;

void (*old_ReadInput)(BNM::UnityEngine::Object *, void *);
void new_ReadInput(BNM::UnityEngine::Object *thiz, void *outInputState) {
    HumanControl_lookHScale[thiz].Set(lookHScale);
    HumanControl_lookVScale[thiz].Set(lookVScale);
    old_ReadInput(thiz, outInputState);
}

void OnLoaded() {
    using namespace BNM;
    HumanControls = Class("", "HumanControls");
    HumanControls_ReadInput = HumanControls.GetMethod("ReadInput");
    HumanControl_lookHScale = HumanControls.GetField("lookHScale");
    HumanControl_lookVScale = HumanControls.GetField("lookVScale");
    HOOK(HumanControls_ReadInput, new_ReadInput, old_ReadInput);
}

std::string GetWorkDir() {
    Dl_info dlInfo;
    if(dladdr((void *) &GetWorkDir, &dlInfo) > 0) {
        std::string soDir = dlInfo.dli_fname;
        return soDir.substr(0, soDir.find_last_of('/'));
    }
    return "";
}

void UseDefaultSettings() {
    HFFSettings["lookHScale"] = "12";
    HFFSettings["lookVScale"] = "5";
}

void ReadSettings() {
    char *buffer = getcwd(nullptr, 0);
    std::string workDir = GetWorkDir();
    std::fstream settingsFile;
    settingsFile.open(workDir + "/HFFSettings.txt", std::ios_base::in);
    if(!settingsFile) {
        UseDefaultSettings();
        BNM_LOG_ERR("%s", "Could not open settings file!");
        return;
    }
    std::string line;
    while(std::getline(settingsFile, line)) {
        if(line.empty()) continue;
        auto mid = line.find_first_of('=');
        std::string key = line.substr(0, mid);
        std::string value = line.substr(mid + 1, line.length() - mid - 1);
        HFFSettings[key] = value;
    }
    settingsFile.close();
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);

    // Load BNM by finding the path to libil2cpp.so
    BNM::Loading::TryLoadByJNI(env);

    // Or load using KittyMemory (as an example)
    // Example_07();

    BNM::Loading::AddOnLoadedEvent(OnLoaded);

    ReadSettings();
    lookHScale = std::stof(HFFSettings["lookHScale"]);
    lookVScale = std::stof(HFFSettings["lookVScale"]);

    return JNI_VERSION_1_6;
}