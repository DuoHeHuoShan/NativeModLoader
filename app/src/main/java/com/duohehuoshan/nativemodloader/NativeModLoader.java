package com.duohehuoshan.nativemodloader;

import android.content.Context;

import java.io.File;

public class NativeModLoader {

    public static void Initialize(Context context) {
        File modsDir = new File(context.getFilesDir().getAbsolutePath() + "/Mods");
        File depsDir = new File(context.getFilesDir().getAbsolutePath() + "/Mods/deps");
        if(!modsDir.exists()) {
            modsDir.mkdirs();
        }
        if(!depsDir.exists()) {
            depsDir.mkdirs();
        }
        for(File depFile : depsDir.listFiles()) {
            if(depFile.getName().endsWith(".so"))
                System.load(depFile.getAbsolutePath());
        }
        for(File modFile : modsDir.listFiles()) {
            if(modFile.getName().endsWith(".so"))
                System.load(modFile.getAbsolutePath());
        }
    }
}
