@echo off
setlocal
# Patch after comand cmake: replace port.cc into "C:\Path\to\grpc\third_party\protobuf\src\google\protobuf\port.cc"
set INSTALL_DIR=C:\grpc-static-with_grpc_plagin_debug
set BUILD_DIR=build-clang-debug

cd /d C:\Users\Max\source\Building\grpc

if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%
mkdir %BUILD_DIR%
cd %BUILD_DIR%

cmake .. -G "Ninja" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER="C:\clang+llvm-21.1.8-x86_64-pc-windows-msvc\bin\lld-link.exe" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DgRPC_ABSL_PROVIDER=module -DgRPC_PROTOBUF_PROVIDER=module -DgRPC_SSL_PROVIDER=module -DgRPC_ZLIB_PROVIDER=module -DgRPC_BUILD_GRPC_CPP_PLUGIN=ON -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_CODEGEN=ON -DgRPC_INSTALL=ON -DCMAKE_INSTALL_PREFIX="C:\grpc-static-with_grpc_plagin_debug" -DOPENSSL_USE_STATIC_LIBS=TRUE -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DBORINGSSL_BUILD_TOOL=OFF cmake .. -G "Ninja" -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER="C:\clang+llvm-21.1.8-x86_64-pc-windows-msvc\bin\lld-link.exe" -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=OFF -DgRPC_ABSL_PROVIDER=module -DgRPC_PROTOBUF_PROVIDER=module -DgRPC_SSL_PROVIDER=module -DgRPC_ZLIB_PROVIDER=module -DgRPC_BUILD_GRPC_CPP_PLUGIN=ON -DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF -DgRPC_BUILD_CODEGEN=ON -DgRPC_INSTALL=ON -DCMAKE_INSTALL_PREFIX="C:\grpc-static-with_grpc_plagin_debug" -DOPENSSL_USE_STATIC_LIBS=TRUE -DCMAKE_CXX_STANDARD=20 -DCMAKE_CXX_STANDARD_REQUIRED=ON -DBORINGSSL_BUILD_TOOL=OFF -DPROTOBUF_CONSTINIT_DISABLED=1 
if %ERRORLEVEL% neq 0 (echo CMake failed! & pause & exit /b 1)

ninja -j8
if %ERRORLEVEL% neq 0 (echo Build failed! & pause & exit /b 1)

ninja install

echo DONE — check %INSTALL_DIR%\bin\grpc_cpp_plugin.exe
pause

