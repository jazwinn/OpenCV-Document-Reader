# 📄 OpenCV Document Scanner & OCR Reader

A lightweight **C++ document scanner and text reader** built using **OpenCV** and **Tesseract OCR**.  
This project detects document edges, warps the perspective, and extracts text using OCR — perfect for scanning receipts, papers, or forms directly from images.

---

## Features

- Automatic **document edge detection** and perspective correction  
- Integrated **Tesseract OCR** for text recognition  
- Built with **OpenCV** for fast and reliable image operations  

---

## Requirements

Before building, ensure you have:
- **CMake** ≥ 3.15  
- **vcpkg** (for dependency management)
- **OpenCV** and **Tesseract** installed via `vcpkg`

---

## Installing Dependencies

Use [vcpkg](https://github.com/microsoft/vcpkg) to install the required libraries:

```bash
# Clone vcpkg (if not installed)
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

# Install dependencies
vcpkg install opencv4 tesseract:x64-windows
```

## Usage
```bash
git clone https://github.com/<your-username>/opencv-document-scanner.git
cd opencv-document-scanner

# Create a build directory
mkdir build && cd build

# Configure CMake with vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build . --config Release

