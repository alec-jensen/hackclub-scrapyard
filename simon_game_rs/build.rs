use std::env;
use std::path::{Path, PathBuf};
use std::process::Command;

fn main() {
    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());
    let lib_path = if cfg!(target_os = "windows") {
        out_dir.join("lib").join("simon_game.dll")
    } else if cfg!(target_os = "macos") {
        out_dir.join("lib").join("libsimon_game.dylib")
    } else {
        out_dir.join("lib").join("libsimon_game.so")
    };

    if !lib_path.exists() {
        let build_dir = out_dir.join("build");
        std::fs::create_dir_all(&build_dir).expect("Failed to create build directory");

        let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
        let ffi_dir = Path::new(&manifest_dir).join("../simon_game_ffi");

        let cmake_status = Command::new("cmake")
            .current_dir(&build_dir)
            .arg(ffi_dir)
            .status()
            .expect("Failed to execute cmake");

        if !cmake_status.success() {
            panic!("CMake configuration failed");
        }

        let build_status = Command::new("cmake")
            .current_dir(&build_dir)
            .args(&["--build", ".", "--config", "Release"])
            .status()
            .expect("Failed to build C++ library");

        if !build_status.success() {
            panic!("C++ library build failed");
        }
    }

    println!("cargo:rustc-link-search=native={}", out_dir.join("lib").display());
    
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=dylib=simon_game");
        println!("cargo:rustc-link-lib=dylib=user32");
        println!("cargo:rustc-link-lib=dylib=gdi32");
    } else {
        println!("cargo:rustc-link-lib=dylib=simon_game");
    }
    
    println!("cargo:rerun-if-changed=../simon_game_ffi/src");
    println!("cargo:rerun-if-changed=../simon_game_ffi/include");
}