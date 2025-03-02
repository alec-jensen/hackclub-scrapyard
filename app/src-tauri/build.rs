use std::env;
use std::path::{PathBuf};
use std::process::Command;

fn main() {
    // Perform standard tauri build steps
    tauri_build::build();

    // Get the directory containing Cargo.toml for the tauri app.
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    // Compute the target directory for simon_game.
    // In our case, we assume the DLL and LIB are output to:
    // <root>/app/src_tauri/target/simon_game
    // Adjust the relative path as needed.
    let simon_game_target = manifest_dir
        .join("./target/simon_game/Release");

    // Define the library file path depending on OS.
    let lib_path = if cfg!(target_os = "windows") {
        simon_game_target.join("simon_game.dll")
    } else if cfg!(target_os = "macos") {
        simon_game_target.join("libsimon_game.dylib")
    } else {
        simon_game_target.join("libsimon_game.so")
    };

    // If the library doesn't exist, run CMake to build it.
    if !lib_path.exists() {
        // Create a build subdirectory inside the simon_game target directory.
        let build_dir = simon_game_target.join("build");
        std::fs::create_dir_all(&build_dir).expect("Failed to create build directory");

        // Locate the FFI source directory (adjust the relative path as needed).
        let ffi_dir = manifest_dir.join("../../simon_game_ffi");

        // Run CMake configuration.
        let cmake_status = Command::new("cmake")
            .current_dir(&build_dir)
            .arg(ffi_dir)
            .status()
            .expect("Failed to execute cmake");
        if !cmake_status.success() {
            panic!("CMake configuration failed");
        }

        // Build the C++ library in Release configuration.
        let build_status = Command::new("cmake")
            .current_dir(&build_dir)
            .args(&["--build", ".", "--config", "Release"])
            .status()
            .expect("Failed to build C++ library");
        if !build_status.success() {
            panic!("C++ library build failed");
        }
    }

    // Instruct Cargo to add the simon_game target directory to the linker search path.
    println!("cargo:rustc-link-search=native={}", simon_game_target.display());
    
    // Link to the simon_game library and other needed Windows libraries.
    if cfg!(target_os = "windows") {
        println!("cargo:rustc-link-lib=dylib=simon_game");
        println!("cargo:rustc-link-lib=dylib=user32");
        println!("cargo:rustc-link-lib=dylib=gdi32");
    } else {
        println!("cargo:rustc-link-lib=dylib=simon_game");
    }
    
    // Re-run the build script if the FFI source or include directories change.
    println!("cargo:rerun-if-changed=../simon_game_ffi/src");
    println!("cargo:rerun-if-changed=../simon_game_ffi/include");
}
