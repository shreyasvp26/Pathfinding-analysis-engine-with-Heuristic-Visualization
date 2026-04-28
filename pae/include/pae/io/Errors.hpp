#pragma once

#include <stdexcept>
#include <string>

namespace pae {

/// Base class for all engine-defined exceptions.
class Error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/// Thrown by GridLoader / GridDumper on malformed or missing files.
class IoError : public Error {
public:
    using Error::Error;
};

/// Thrown by Registry::create on unknown name.
class UnknownNameError : public Error {
public:
    using Error::Error;
};

namespace cli {
/// Thrown by parseArgs on bad CLI usage. Caught in main.cpp.
class UsageError : public Error {
public:
    using Error::Error;
};
}  // namespace cli

}  // namespace pae
