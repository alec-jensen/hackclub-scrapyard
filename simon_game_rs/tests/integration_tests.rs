#![cfg(feature = "hardware_tests")]
use std::env;
use simon_game::{SerialMonitor, KeyboardMiddleware};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::Duration;


fn get_com_port() -> String {
    env::var("SIMON_GAME_COM_PORT").unwrap_or_else(|_| "COM6".to_string())
}

#[test]
fn test_real_serial_connection() {
    let port = get_com_port();
    println!("Testing with COM port: {}", port);
    
    let monitor = SerialMonitor::new(&port).expect("Failed to create SerialMonitor");

    match monitor.connect() {
        Ok(_) => {
            println!("Successfully connected to hardware");
            assert!(monitor.is_connected());
        
            let result = monitor.send_command("PING");
            assert!(result.is_ok());
            let _ = monitor.disconnect();
            assert!(!monitor.is_connected());
        },
        Err(err) => {
            println!("Hardware not available: {:?}", err);
            assert!(true); 
        }
    }
}

#[test]
fn test_real_simon_game() {
    let port = get_com_port();
    
    let monitor = SerialMonitor::new(&port).expect("Failed to create SerialMonitor");
    if let Err(err) = monitor.connect() {
        println!("Hardware not available: {:?}", err);
        return;
    }

    let result = monitor.send_simon_game_length(3);
    assert!(result.is_ok());
    println!("Simon game pattern sent with length 3");
    println!("Please complete the pattern on the hardware...");
    let success = monitor.verify_simon_game_success(20000); 
    println!("Pattern completion result: {}", success);
    let _ = monitor.disconnect();
}
#[test]
fn test_keyboard_middleware_with_hardware() {
    let port = get_com_port();
    let monitor = Arc::new(Mutex::new(SerialMonitor::new(&port).expect("Failed to create SerialMonitor")));
    if let Err(err) = monitor.lock().unwrap().connect() {
        println!("Hardware not available: {:?}", err);
        return;
    }
    let middleware = KeyboardMiddleware::new().expect("Failed to create KeyboardMiddleware");
    let _ = middleware.initialize();
    let _ = middleware.register_key('W' as i32, 3);
    let monitor_clone = monitor.clone();
    let result = middleware.register_callbacks(
        move |length| {
            println!("Sending Simon game pattern with length: {}", length);
            let _ = monitor_clone.lock().unwrap().send_simon_game_length(length);
        },
        move || {
            println!("Verifying pattern completion...");
            monitor.lock().unwrap().verify_simon_game_success(10000)
        }
    );
    assert!(result.is_ok());
    println!("Keyboard middleware registered successfully");
    println!("Press 'W' key to test (requires admin privileges)");
    thread::sleep(Duration::from_secs(30));
    let _ = middleware.cleanup();
    let _ = monitor.lock().unwrap().disconnect();
}