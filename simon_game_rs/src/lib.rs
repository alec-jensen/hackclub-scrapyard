mod ffi;
mod error;

use std::ffi::CString;
use std::ptr;
use std::sync::{Arc, Mutex};

pub use error::SimonError;

struct CallbackStore {
    send: Option<Box<dyn Fn(i32) + Send + 'static>>,
    receive: Option<Box<dyn Fn() -> bool + Send + 'static>>,
}

lazy_static::lazy_static! {
    static ref CALLBACKS: Arc<Mutex<CallbackStore>> = Arc::new(Mutex::new(CallbackStore {
        send: None,
        receive: None,
    }));
}

pub struct SerialMonitor {
    handle: ffi::serial_monitor_t,
}

impl SerialMonitor {
    /// Create a new SerialMonitor instance
    ///
    /// # Arguments
    ///
    /// * `port` - Name of the serial port to connect to (e.g., "COM6")
    ///
    /// # Returns
    ///
    /// A new SerialMonitor instance or an error if creation failed
    pub fn new(port: &str) -> Result<Self, SimonError> {
        let port_c = CString::new(port).map_err(|_| SimonError::InvalidParameter)?;
        
        unsafe {
            let handle = ffi::sm_create(port_c.as_ptr());
            if handle.is_null() {
                return Err(SimonError::NullHandle);
            }
            
            Ok(SerialMonitor { handle })
        }
    }
    
    pub fn connect(&self) -> Result<(), SimonError> {
        unsafe {
            match ffi::sm_connect(self.handle) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn disconnect(&self) -> Result<(), SimonError> {
        unsafe {
            match ffi::sm_disconnect(self.handle) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn send_command(&self, command: &str) -> Result<(), SimonError> {
        let cmd_c = CString::new(command).map_err(|_| SimonError::InvalidParameter)?;
        
        unsafe {
            match ffi::sm_send_command(self.handle, cmd_c.as_ptr()) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn send_simon_game_length(&self, length: i32) -> Result<(), SimonError> {
        if length <= 0 {
            return Err(SimonError::InvalidParameter);
        }
        
        unsafe {
            match ffi::sm_send_simon_game_length(self.handle, length) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    pub fn verify_simon_game_success(&self, timeout_ms: i32) -> bool {
        unsafe {
            ffi::sm_verify_simon_game_success(self.handle, timeout_ms) != 0
        }
    }
    
    pub fn is_connected(&self) -> bool {
        unsafe {
            ffi::sm_is_connected(self.handle) != 0
        }
    }
}

impl Drop for SerialMonitor {
    fn drop(&mut self) {
        unsafe {
            if !self.handle.is_null() {
                ffi::sm_destroy(self.handle);
                self.handle = ptr::null_mut();
            }
        }
    }
}

unsafe impl Send for SerialMonitor {}
unsafe impl Sync for SerialMonitor {}

unsafe extern "C" fn send_trampoline(counter: std::os::raw::c_int) {
    if let Ok(callbacks) = CALLBACKS.lock() {
        if let Some(ref send_cb) = callbacks.send {
            send_cb(counter);
        }
    }
}

unsafe extern "C" fn receive_trampoline() -> std::os::raw::c_int {
    if let Ok(callbacks) = CALLBACKS.lock() {
        if let Some(ref receive_cb) = callbacks.receive {
            return if receive_cb() { 1 } else { 0 };
        }
    }
    0
}

pub struct KeyboardMiddleware {
    handle: ffi::keyboard_middleware_t,
}

impl KeyboardMiddleware {
    pub fn new() -> Result<Self, SimonError> {
        unsafe {
            let handle = ffi::km_create();
            if handle.is_null() {
                return Err(SimonError::NullHandle);
            }
            
            Ok(KeyboardMiddleware { handle })
        }
    }
    
    pub fn initialize(&self) -> Result<(), SimonError> {
        unsafe {
            match ffi::km_initialize(self.handle) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn register_key(&self, key_code: i32, target_count: i32) -> Result<(), SimonError> {
        if target_count <= 0 {
            return Err(SimonError::InvalidParameter);
        }
        
        unsafe {
            match ffi::km_register_key(self.handle, key_code, target_count) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn register_callbacks<F, G>(&self, send_callback: F, receive_callback: G) -> Result<(), SimonError>
    where
        F: Fn(i32) + Send + 'static,
        G: Fn() -> bool + Send + 'static,
    {
        {
            let mut callbacks = CALLBACKS.lock().map_err(|_| SimonError::Unknown)?;
            callbacks.send = Some(Box::new(send_callback));
            callbacks.receive = Some(Box::new(receive_callback));
        }
        
        unsafe {
            match ffi::km_register_callbacks(
                self.handle,
                Some(send_trampoline),
                Some(receive_trampoline)
            ) {
                ffi::simon_error_t::SIMON_SUCCESS => Ok(()),
                err => Err(SimonError::from(err)),
            }
        }
    }
    
    pub fn cleanup(&self) -> Result<(), SimonError> {
        unsafe {
            match ffi::km_cleanup(self.handle) {
                ffi::simon_error_t::SIMON_SUCCESS => {
                    let mut callbacks = CALLBACKS.lock().map_err(|_| SimonError::Unknown)?;
                    callbacks.send = None;
                    callbacks.receive = None;
                    Ok(())
                },
                err => Err(SimonError::from(err)),
            }
        }
    }
}

impl Drop for KeyboardMiddleware {
    fn drop(&mut self) {
        unsafe {
            if !self.handle.is_null() {
                let _ = self.cleanup();
                ffi::km_destroy(self.handle);
                self.handle = ptr::null_mut();
            }
        }
    }
}

unsafe impl Send for KeyboardMiddleware {}
unsafe impl Sync for KeyboardMiddleware {}