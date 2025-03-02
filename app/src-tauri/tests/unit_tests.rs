use std::ptr;
use simon_game::{SerialMonitor, KeyboardMiddleware, SimonError};
use std::sync::{Arc, Mutex, atomic::{AtomicBool, Ordering}};

mod mock_ffi;

fn setup() {
    mock_ffi::reset_mock_state();
}

#[test]
fn test_serial_monitor_creation() {
    setup();
    
    let monitor = SerialMonitor::new("COM6");
    assert!(monitor.is_ok());
    
    let monitor = monitor.unwrap();
    assert!(!monitor.is_connected());
}

#[test]
fn test_serial_monitor_connect_disconnect() {
    setup();
    
    let monitor = SerialMonitor::new("COM6").unwrap();
    
    let result = monitor.connect();
    assert!(result.is_ok());
    assert!(monitor.is_connected());
    
    let result = monitor.disconnect();
    assert!(result.is_ok());
    assert!(!monitor.is_connected());
}

#[test]
fn test_send_simon_game_length() {
    setup();
    
    let monitor = SerialMonitor::new("COM6").unwrap();
    let _ = monitor.connect();
    
    let result = monitor.send_simon_game_length(5);
    assert!(result.is_ok());
    
    let result = monitor.send_simon_game_length(0);
    assert!(result.is_err());
    match result {
        Err(SimonError::InvalidParameter) => {},
        _ => panic!("Expected InvalidParameter error"),
    }
    
    let _ = monitor.disconnect();
    let result = monitor.send_simon_game_length(5);
    assert!(result.is_err());
}

#[test]
fn test_verify_simon_game_success() {
    setup();
    
    let monitor = SerialMonitor::new("COM6").unwrap();
    let _ = monitor.connect();
    
    mock_ffi::set_simon_game_success(true);
    assert!(monitor.verify_simon_game_success(1000));
    
    mock_ffi::set_simon_game_success(false);
    assert!(!monitor.verify_simon_game_success(1000));
}

#[test]
fn test_keyboard_middleware_initialize() {
    setup();
    
    let middleware = KeyboardMiddleware::new();
    assert!(middleware.is_ok());
    
    let middleware = middleware.unwrap();
    let result = middleware.initialize();
    assert!(result.is_ok());
}

#[test]
fn test_register_key() {
    setup();
    
    let middleware = KeyboardMiddleware::new().unwrap();
    let _ = middleware.initialize();

    let result = middleware.register_key('W' as i32, 5);
    assert!(result.is_ok());
    
    let result = middleware.register_key('A' as i32, 0);
    assert!(result.is_err());
    match result {
        Err(SimonError::InvalidParameter) => {},
        _ => panic!("Expected InvalidParameter error"),
    }
}

#[test]
fn test_register_callbacks() {
    setup();
    
    let middleware = KeyboardMiddleware::new().unwrap();
    let _ = middleware.initialize();
    
    let send_called = Arc::new(AtomicBool::new(false));
    let receive_called = Arc::new(AtomicBool::new(false));
    
    let send_called_clone = send_called.clone();
    let receive_called_clone = receive_called.clone();
    
    let result = middleware.register_callbacks(
        move |_| { send_called_clone.store(true, Ordering::SeqCst); },
        move || { receive_called_clone.store(true, Ordering::SeqCst); true }
    );
    
    assert!(result.is_ok());
}

#[test]
fn test_cleanup() {
    setup();
    
    let middleware = KeyboardMiddleware::new().unwrap();
    let _ = middleware.initialize();
    let _ = middleware.register_key('W' as i32, 5);
    
    let result = middleware.cleanup();
    assert!(result.is_ok());
}

#[test]
fn test_error_handling() {
    let mut monitor = SerialMonitor::new("COM6").unwrap();
    unsafe {
        monitor.handle = ptr::null_mut();
    }

    let connect_result = monitor.connect();
    assert!(matches!(connect_result, Err(SimonError::NullHandle)));
    
    let send_result = monitor.send_simon_game_length(5);
    assert!(matches!(send_result, Err(SimonError::NullHandle)));
    assert!(!monitor.is_connected());
}