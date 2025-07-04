@echo off
REM Unified Shell Build Script for Windows
REM Cross-platform build automation for Windows environments

setlocal enabledelayedexpansion

REM Default values
set BUILD_TYPE=Release
set INSTALL=false
set CLEAN=false
set USE_CMAKE=false
set VERBOSE=false

REM Colors (if supported)
set RED=[91m
set GREEN=[92m
set YELLOW=[93m
set BLUE=[94m
set NC=[0m

:parse_args
if "%1"=="" goto main
if "%1"=="-h" goto show_help
if "%1"=="--help" goto show_help
if "%1"=="-d" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto parse_args
)
if "%1"=="-r" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if "%1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if "%1"=="-i" (
    set INSTALL=true
    shift
    goto parse_args
)
if "%1"=="--install" (
    set INSTALL=true
    shift
    goto parse_args
)
if "%1"=="-c" (
    set CLEAN=true
    shift
    goto parse_args
)
if "%1"=="--clean" (
    set CLEAN=true
    shift
    goto parse_args
)
if "%1"=="-m" (
    set USE_CMAKE=true
    shift
    goto parse_args
)
if "%1"=="--cmake" (
    set USE_CMAKE=true
    shift
    goto parse_args
)
if "%1"=="-v" (
    set VERBOSE=true
    shift
    goto parse_args
)
if "%1"=="--verbose" (
    set VERBOSE=true
    shift
    goto parse_args
)
if "%1"=="--check-deps" goto check_deps
if "%1"=="--setup-dev" goto setup_dev
if "%1"=="--version" goto show_version

echo Unknown option: %1
goto show_help

:show_help
echo Unified Shell Build Script for Windows
echo.
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo     -h, --help          Show this help message
echo     -d, --debug         Build in debug mode
echo     -r, --release       Build in release mode (default)
echo     -i, --install       Install after building
echo     -c, --clean         Clean build artifacts before building
echo     -m, --cmake         Use CMake instead of Make
echo     -v, --verbose       Enable verbose output
echo     --check-deps        Check dependencies and exit
echo     --setup-dev         Setup development environment
echo     --version           Show version and exit
echo.
echo Examples:
echo     %0                      # Build release version with Make
echo     %0 --debug --cmake      # Build debug version with CMake
echo     %0 --clean --install    # Clean, build, and install
echo     %0 --check-deps         # Check if all dependencies are available
echo.
echo Requirements:
echo     - MSYS2 or MinGW-w64 environment
echo     - g++ compiler
echo     - readline library
echo     - make or cmake
echo.
goto end

:show_version
echo Unified Shell Build Script v1.0.0
echo Cross-platform command execution environment build system
goto end

:print_info
echo %BLUE%[INFO]%NC% %1
goto :eof

:print_success
echo %GREEN%[SUCCESS]%NC% %1
goto :eof

:print_warning
echo %YELLOW%[WARNING]%NC% %1
goto :eof

:print_error
echo %RED%[ERROR]%NC% %1
goto :eof

:detect_env
call :print_info "Detecting Windows environment..."

REM Check if we're in MSYS2
if defined MSYSTEM (
    set ENVIRONMENT=MSYS2
    call :print_info "Detected MSYS2 environment: %MSYSTEM%"
    goto :eof
)

REM Check if we're in MinGW
if defined MINGW_PREFIX (
    set ENVIRONMENT=MinGW
    call :print_info "Detected MinGW environment"
    goto :eof
)

REM Check if we're in regular Windows Command Prompt
where cl >nul 2>&1
if %errorlevel%==0 (
    set ENVIRONMENT=MSVC
    call :print_info "Detected MSVC environment"
    goto :eof
)

set ENVIRONMENT=Unknown
call :print_warning "Could not detect build environment"
goto :eof

:check_deps
call :print_info "Checking dependencies for Windows..."
call :detect_env

REM Check for compilers
where g++ >nul 2>&1
if %errorlevel%==0 (
    call :print_success "g++ compiler found"
    for /f "tokens=*" %%i in ('g++ --version 2^>nul ^| findstr /r "^g++"') do call :print_info "  %%i"
) else (
    where cl >nul 2>&1
    if !errorlevel!==0 (
        call :print_success "MSVC compiler found"
    ) else (
        call :print_error "No C++ compiler found! Please install g++ or MSVC"
        goto error
    )
)

REM Check for build systems
if "%USE_CMAKE%"=="true" (
    where cmake >nul 2>&1
    if !errorlevel!==0 (
        call :print_success "CMake found"
        for /f "tokens=*" %%i in ('cmake --version 2^>nul ^| findstr /r "cmake"') do call :print_info "  %%i"
    ) else (
        call :print_error "CMake not found! Please install CMake"
        goto error
    )
) else (
    where make >nul 2>&1
    if !errorlevel!==0 (
        call :print_success "Make found"
    ) else (
        where mingw32-make >nul 2>&1
        if !errorlevel!==0 (
            call :print_success "MinGW Make found"
            set MAKE_CMD=mingw32-make
        ) else (
            call :print_error "Make not found! Please install make or use --cmake"
            goto error
        )
    )
)

REM Check for readline
if exist "C:\msys64\mingw64\include\readline\readline.h" (
    call :print_success "Readline found in MSYS2 MinGW64"
) else if exist "C:\msys64\ucrt64\include\readline\readline.h" (
    call :print_success "Readline found in MSYS2 UCRT64"
) else (
    call :print_warning "Readline not found. The shell will have limited functionality."
    call :print_info "Install with: pacman -S mingw-w64-ucrt-x86_64-readline"
)

call :print_success "Dependency check complete!"
goto end

:setup_dev
call :print_info "Setting up development environment for Windows..."
call :detect_env

if "%ENVIRONMENT%"=="MSYS2" (
    call :print_info "Installing dependencies via MSYS2..."
    pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-readline mingw-w64-ucrt-x86_64-cmake make
    if !errorlevel!==0 (
        call :print_success "MSYS2 dependencies installed successfully!"
    ) else (
        call :print_error "Failed to install MSYS2 dependencies"
        goto error
    )
) else (
    call :print_error "Automated setup only supported in MSYS2 environment"
    call :print_info "Please install MSYS2 and run this script from MSYS2 terminal"
    call :print_info "Download MSYS2 from: https://www.msys2.org/"
    goto error
)

goto end

:clean_build
call :print_info "Cleaning build artifacts..."

if "%USE_CMAKE%"=="true" (
    if exist build\ (
        rmdir /s /q build
        call :print_success "CMake build directory cleaned"
    )
) else (
    if defined MAKE_CMD (
        %MAKE_CMD% clean >nul 2>&1
    ) else (
        make clean >nul 2>&1
    )
    call :print_success "Make artifacts cleaned"
)
goto :eof

:build_with_make
call :print_info "Building with Make (%BUILD_TYPE% mode)..."

set MAKE_TARGET=
if "%BUILD_TYPE%"=="Debug" set MAKE_TARGET=debug

if defined MAKE_CMD (
    set BUILD_CMD=%MAKE_CMD% %MAKE_TARGET%
) else (
    set BUILD_CMD=make %MAKE_TARGET%
)

if "%VERBOSE%"=="true" (
    %BUILD_CMD%
) else (
    %BUILD_CMD% >build.log 2>&1
)

if %errorlevel%==0 (
    call :print_success "Build completed successfully!"
) else (
    call :print_error "Build failed! Check build.log for details"
    goto error
)
goto :eof

:build_with_cmake
call :print_info "Building with CMake (%BUILD_TYPE% mode)..."

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if "%VERBOSE%"=="true" set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON

cmake .. %CMAKE_ARGS%
if %errorlevel% neq 0 (
    call :print_error "CMake configuration failed!"
    cd ..
    goto error
)

REM Build
cmake --build . --config %BUILD_TYPE%
if %errorlevel% neq 0 (
    call :print_error "CMake build failed!"
    cd ..
    goto error
)

cd ..
call :print_success "CMake build completed successfully!"
goto :eof

:install_binary
call :print_info "Installing unified_shell..."

if "%USE_CMAKE%"=="true" (
    cd build
    cmake --install .
    cd ..
) else (
    if defined MAKE_CMD (
        %MAKE_CMD% install
    ) else (
        make install
    )
)

if %errorlevel%==0 (
    call :print_success "Installation completed!"
) else (
    call :print_warning "Installation may have failed (check permissions)"
)
goto :eof

:run_tests
call :print_info "Running basic tests..."

set BINARY_PATH=unified_shell.exe
if "%USE_CMAKE%"=="true" set BINARY_PATH=build\unified_shell.exe

if exist "%BINARY_PATH%" (
    REM Test version
    "%BINARY_PATH%" --version >nul 2>&1
    if !errorlevel!==0 (
        call :print_success "Version test passed"
    ) else (
        call :print_warning "Version test failed"
    )
    
    REM Test help
    "%BINARY_PATH%" --help >nul 2>&1
    if !errorlevel!==0 (
        call :print_success "Help test passed"
    ) else (
        call :print_warning "Help test failed"
    )
) else (
    call :print_warning "Binary not found at %BINARY_PATH%"
)
goto :eof

:main
call :print_info "Starting Unified Shell build process for Windows..."

REM Detect environment
call :detect_env

REM Clean if requested
if "%CLEAN%"=="true" call :clean_build

REM Check dependencies
call :check_deps
if %errorlevel% neq 0 goto error

REM Build
if "%USE_CMAKE%"=="true" (
    call :build_with_cmake
) else (
    call :build_with_make
)
if %errorlevel% neq 0 goto error

REM Run tests
call :run_tests

REM Install if requested
if "%INSTALL%"=="true" call :install_binary

call :print_success "Build process completed successfully!"
call :print_info "You can now run the unified shell with: .\unified_shell.exe"
if "%INSTALL%"=="true" (
    call :print_info "Or simply: unified_shell (if installed to PATH)"
)

goto end

:error
call :print_error "Build process failed!"
exit /b 1

:end
endlocal
exit /b 0