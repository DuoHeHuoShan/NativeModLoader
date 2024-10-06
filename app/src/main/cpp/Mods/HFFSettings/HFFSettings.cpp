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

std::map<std::string, std::string> HFFSettings;

BNM::Class HumanControls;
BNM::MethodBase HumanControls_ReadInput;
BNM::Field<float> HumanControl_lookHScale;
BNM::Field<float> HumanControl_lookVScale;
BNM::Class Ball;
BNM::MethodBase Ball_OnEnable;
BNM::Class GameObject;
BNM::Method<BNM::UnityEngine::Object *> GameObject_Find;
BNM::Method<void> GameObject_SetActive;
BNM::Class GamePlayerPrefs;
BNM::MethodBase GamePlayerPrefs_GetInt, GamePlayerPrefs_SetInt, GamePlayerPrefs_GetFloat, GamePlayerPrefs_SetFloat, GamePlayerPrefs_GetString, GamePlayerPrefs_SetString;
BNM::Class UnityPlayerPrefs;
BNM::MethodBase UnityPlayerPrefs_GetInt, UnityPlayerPrefs_SetInt, UnityPlayerPrefs_GetFloat, UnityPlayerPrefs_SetFloat, UnityPlayerPrefs_GetString, UnityPlayerPrefs_SetString;

float lookHScale = 12;
float lookVScale = 5;
bool visibleBall = false;
bool localSave = false;

void (*old_ReadInput)(BNM::UnityEngine::Object *, void *);
void new_ReadInput(BNM::UnityEngine::Object *thiz, void *outInputState) {
    HumanControl_lookHScale[thiz].Set(lookHScale);
    HumanControl_lookVScale[thiz].Set(lookVScale);
    old_ReadInput(thiz, outInputState);
}

void (*old_Ball_OnEnable)(BNM::UnityEngine::Object *);
void new_Ball_OnEnable(BNM::UnityEngine::Object *thiz) {
    old_Ball_OnEnable(thiz);
    if(visibleBall) GameObject_SetActive[GameObject_Find(BNM::CreateMonoString("/Player(Clone)/Ball/Sphere"))](true);
}

void ApplyLocalSave() {
    using namespace BNM;
    GamePlayerPrefs = Class("Mobile.SaveGameSystem", "PlayerPrefs");
    UnityPlayerPrefs = Class("UnityEngine", "PlayerPrefs");

    GamePlayerPrefs_GetInt = GamePlayerPrefs.GetMethod("GetInt");
    GamePlayerPrefs_SetInt = GamePlayerPrefs.GetMethod("SetInt");
    GamePlayerPrefs_GetFloat = GamePlayerPrefs.GetMethod("GetFloat");
    GamePlayerPrefs_SetFloat = GamePlayerPrefs.GetMethod("SetFloat");
    GamePlayerPrefs_GetString = GamePlayerPrefs.GetMethod("GetString");
    GamePlayerPrefs_SetString = GamePlayerPrefs.GetMethod("SetString");

    UnityPlayerPrefs_GetInt = UnityPlayerPrefs.GetMethod("GetInt");
    UnityPlayerPrefs_SetInt = UnityPlayerPrefs.GetMethod("SetInt");
    UnityPlayerPrefs_GetFloat = UnityPlayerPrefs.GetMethod("GetFloat");
    UnityPlayerPrefs_SetFloat = UnityPlayerPrefs.GetMethod("SetFloat");
    UnityPlayerPrefs_GetString = UnityPlayerPrefs.GetMethod("GetString");
    UnityPlayerPrefs_SetString = UnityPlayerPrefs.GetMethod("SetString");

    HOOK(GamePlayerPrefs_GetInt, UnityPlayerPrefs_GetInt.GetInfo()->methodPointer, nullptr);
    HOOK(GamePlayerPrefs_SetInt, UnityPlayerPrefs_SetInt.GetInfo()->methodPointer, nullptr);
    HOOK(GamePlayerPrefs_GetFloat, UnityPlayerPrefs_GetFloat.GetInfo()->methodPointer, nullptr);
    HOOK(GamePlayerPrefs_SetFloat, UnityPlayerPrefs_SetFloat.GetInfo()->methodPointer, nullptr);
    HOOK(GamePlayerPrefs_GetString, UnityPlayerPrefs_GetString.GetInfo()->methodPointer, nullptr);
    HOOK(GamePlayerPrefs_SetString, UnityPlayerPrefs_SetString.GetInfo()->methodPointer, nullptr);
}

void OnLoaded() {
    using namespace BNM;
    HumanControls = Class("", "HumanControls");
    HumanControls_ReadInput = HumanControls.GetMethod("ReadInput");
    HumanControl_lookHScale = HumanControls.GetField("lookHScale");
    HumanControl_lookVScale = HumanControls.GetField("lookVScale");
    Ball = BNM::Class("", "Ball");
    Ball_OnEnable = Ball.GetMethod("OnEnable");
    GameObject = BNM::Class("UnityEngine", "GameObject");
    GameObject_Find = GameObject.GetMethod("Find");
    GameObject_SetActive = GameObject.GetMethod("SetActive");
    if(localSave) ApplyLocalSave();

    InvokeHook(Ball_OnEnable, new_Ball_OnEnable, old_Ball_OnEnable);
    HOOK(HumanControls_ReadInput, new_ReadInput, old_ReadInput);
}

bool stob(const std::string &str) {
    if(str == "true") return true;
    return false;
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
    HFFSettings["visibleBall"] = "false";
    HFFSettings["localSave"] = "false";
}

void ReadSettings() {
    UseDefaultSettings();
    std::string workDir = GetWorkDir();
    std::fstream settingsFile;
    settingsFile.open(workDir + "/HFFSettings.txt", std::ios_base::in);
    if(!settingsFile) {
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
    visibleBall = stob(HFFSettings["visibleBall"]);
    localSave = stob(HFFSettings["localSave"]);

    return JNI_VERSION_1_6;
}