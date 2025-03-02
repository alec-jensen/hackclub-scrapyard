mod ffi_lib;

pub use ffi_lib::{SerialMonitor, KeyboardMiddleware, SimonError};

use tauri::Manager;
use std::sync::{Arc, Mutex};

struct AppState {
    serial_monitor: Mutex<Option<Arc<Mutex<SerialMonitor>>>>,
    keyboard_middleware: Mutex<Option<Arc<Mutex<KeyboardMiddleware>>>>,
}

#[tauri::command]
async fn connect_serial(state: tauri::State<'_, AppState>, port_name: String) -> Result<String, String> {
    // First lock the mutex to get a mutable reference to the Option inside
    let mut serial_monitor_guard = state.serial_monitor.lock().map_err(|_| "Lock failed")?;
    
    // Close any existing connection first
    if let Some(existing_monitor) = &*serial_monitor_guard {
        // Get a lock on the existing monitor
        let mut monitor = existing_monitor.lock().map_err(|_| "Lock failed")?;
        
        // If already connected, disconnect first
        if monitor.is_connected() {
            monitor.disconnect().map_err(|e| format!("Failed to close existing connection: {:?}", e))?;
        }
    }
    
    // Create the new monitor instance
    let monitor = SerialMonitor::new(&port_name).map_err(|e| format!("{:?}", e))?;
    
    // Connect to the serial port
    monitor.connect().map_err(|e| format!("Failed to connect: {:?}", e))?;
    
    // Verify connection
    if !monitor.is_connected() {
        return Err("Serial monitor created but connection failed".into());
    }
    
    // Wrap in Arc<Mutex<>>
    let monitor_arc = Arc::new(Mutex::new(monitor));
    
    // Replace or create new monitor reference
    *serial_monitor_guard = Some(monitor_arc);
    
    Ok("Serial monitor connected successfully".into())
}

#[tauri::command]
fn is_connected(state: tauri::State<'_, AppState>) -> Result<bool, String> {
    let serial_monitor_guard = state.serial_monitor.lock().map_err(|_| "Lock failed")?;
    
    match serial_monitor_guard.as_ref() {
        Some(monitor_ref) => {
            let monitor = monitor_ref.lock().map_err(|_| "Lock failed")?;
            Ok(monitor.is_connected())
        },
        None => Ok(false)
    }
}

#[tauri::command]
fn send_command(state: tauri::State<'_, AppState>, command: String) -> Result<(), String> {
    // First lock the outer mutex
    let serial_monitor_guard = state.serial_monitor.lock().map_err(|_| "Lock failed")?;
    
    // Then check if the Option has a value
    match serial_monitor_guard.as_ref() {
        Some(monitor_ref) => {
            // Then lock the inner mutex
            let monitor = monitor_ref.lock().map_err(|_| "Lock failed")?;
            monitor.send_command(&command).map_err(|e| format!("{:?}", e))?;
            Ok(())
        },
        None => {
            Err("Serial monitor not initialized".into())
        }
    }
}

#[tauri::command]
fn verify_simon_game_success(state: tauri::State<'_, AppState>, timeout_ms: i32) -> Result<bool, String> {
    // First lock the outer mutex
    let serial_monitor_guard = state.serial_monitor.lock().map_err(|_| "Lock failed")?;
    
    // Then check if the Option has a value
    match serial_monitor_guard.as_ref() {
        Some(monitor_ref) => {
            // Then lock the inner mutex
            let monitor = monitor_ref.lock().map_err(|_| "Lock failed")?;
            Ok(monitor.verify_simon_game_success(timeout_ms))
        },
        None => {
            Err("Serial monitor not initialized".into())
        }
    }
}

#[tauri::command]
fn initialize_keyboard_middleware(state: tauri::State<'_, AppState>) -> Result<String, String> {
    // Lock the mutex to get a mutable reference
    let mut keyboard_middleware_guard = state.keyboard_middleware.lock().map_err(|_| "Lock failed")?;
    
    // Clean up any existing middleware first
    if let Some(existing_middleware) = &*keyboard_middleware_guard {
        let middleware = existing_middleware.lock().map_err(|_| "Lock failed")?;
        let _ = middleware.cleanup();
    }
    
    // Create new middleware instance
    let middleware = KeyboardMiddleware::new().map_err(|e| format!("{:?}", e))?;
    
    // Initialize the middleware
    middleware.initialize().map_err(|e| format!("Failed to initialize: {:?}", e))?;
    
    // Wrap in Arc<Mutex<>>
    let middleware_arc = Arc::new(Mutex::new(middleware));
    
    // Set the middleware reference
    *keyboard_middleware_guard = Some(middleware_arc);
    
    Ok("Keyboard middleware initialized successfully".into())
}

#[tauri::command]
fn register_key(state: tauri::State<'_, AppState>, key_code: i32, target_count: i32) -> Result<(), String> {
    let middleware_guard = state.keyboard_middleware.lock().map_err(|_| "Lock failed")?;
    
    match &*middleware_guard {
        Some(middleware) => {
            let middleware = middleware.lock().map_err(|_| "Lock failed")?;
            middleware.register_key(key_code, target_count)
                .map_err(|e| format!("Failed to register key: {:?}", e))
        },
        None => Err("Keyboard middleware not initialized".into())
    }
}

#[tauri::command]
fn register_keyboard_callbacks(state: tauri::State<'_, AppState>) -> Result<(), String> {
    let middleware_guard = state.keyboard_middleware.lock().map_err(|_| "Lock failed")?;
    let serial_monitor_guard = state.serial_monitor.lock().map_err(|_| "Lock failed")?;
    
    let middleware = match &*middleware_guard {
        Some(m) => m.clone(),
        None => return Err("Keyboard middleware not initialized".into())
    };
    
    let monitor = match &*serial_monitor_guard {
        Some(m) => m.clone(),
        None => return Err("Serial monitor not initialized".into())
    };
    
    let middleware_lock = middleware.lock().map_err(|_| "Lock failed")?;
    let monitor_clone = monitor.clone();
    
    middleware_lock.register_callbacks(
        move |length| {
            if let Ok(mut monitor) = monitor_clone.lock() {
                let _ = monitor.send_simon_game_length(length);
            }
        },
        move || {
            if let Ok(monitor) = monitor.lock() {
                monitor.verify_simon_game_success(5000) // 5 second timeout
            } else {
                false
            }
        }
    ).map_err(|e| format!("Failed to register callbacks: {:?}", e))
}

#[tauri::command]
fn cleanup_keyboard_middleware(state: tauri::State<'_, AppState>) -> Result<(), String> {
    let middleware_guard = state.keyboard_middleware.lock().map_err(|_| "Lock failed")?;
    
    if let Some(middleware) = &*middleware_guard {
        let middleware = middleware.lock().map_err(|_| "Lock failed")?;
        middleware.cleanup().map_err(|e| format!("Failed to cleanup: {:?}", e))
    } else {
        Ok(()) // Nothing to clean up
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    // Initialize your state
    let app_state = AppState {
        serial_monitor: Mutex::new(None),
        keyboard_middleware: Mutex::new(None),
    };

    tauri::Builder::default()
        // Register the state with Tauri
        .manage(app_state)
        .setup(|app| {
            #[cfg(debug_assertions)] // only include this code on debug builds
            {
                let window = app.get_webview_window("main").unwrap();
                window.open_devtools();
                window.close_devtools();
            }
            Ok(())
        })
        .plugin(tauri_plugin_shell::init())
        .plugin(tauri_plugin_fs::init())
        .plugin(tauri_plugin_store::Builder::default().build())
        .plugin(tauri_plugin_http::init())
        .invoke_handler(tauri::generate_handler![
            connect_serial,
            is_connected,
            send_command,
            verify_simon_game_success,
            initialize_keyboard_middleware,
            register_key,
            register_keyboard_callbacks,
            cleanup_keyboard_middleware
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}