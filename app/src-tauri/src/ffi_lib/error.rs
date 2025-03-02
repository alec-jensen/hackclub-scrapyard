use super::ffi;
use std::fmt;
use std::error::Error;

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SimonError {
    NullHandle,
    ConnectionFailed,
    PortUnavailable,
    InvalidParameter,
    HookFailed,
    Timeout,
    Unknown,
}

impl fmt::Display for SimonError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            SimonError::NullHandle => write!(f, "Null handle provided"),
            SimonError::ConnectionFailed => write!(f, "Connection to serial device failed"),
            SimonError::PortUnavailable => write!(f, "Serial port is unavailable"),
            SimonError::InvalidParameter => write!(f, "Invalid parameter provided"),
            SimonError::HookFailed => write!(f, "Failed to set up keyboard hook"),
            SimonError::Timeout => write!(f, "Operation timed out"),
            SimonError::Unknown => write!(f, "Unknown error occurred"),
        }
    }
}

impl Error for SimonError {}

impl From<ffi::simon_error_t> for SimonError {
    fn from(err: ffi::simon_error_t) -> Self {
        match err {
            ffi::simon_error_t::SIMON_ERROR_NULL_HANDLE => SimonError::NullHandle,
            ffi::simon_error_t::SIMON_ERROR_CONNECTION_FAILED => SimonError::ConnectionFailed,
            ffi::simon_error_t::SIMON_ERROR_PORT_UNAVAILABLE => SimonError::PortUnavailable,
            ffi::simon_error_t::SIMON_ERROR_INVALID_PARAMETER => SimonError::InvalidParameter,
            ffi::simon_error_t::SIMON_ERROR_HOOK_FAILED => SimonError::HookFailed,
            ffi::simon_error_t::SIMON_ERROR_TIMEOUT => SimonError::Timeout,
            _ => SimonError::Unknown,
        }
    }
}