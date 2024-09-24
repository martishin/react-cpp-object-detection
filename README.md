
# Real-time Object Detection with React, WebSockets, and C++

This project demonstrates a real-time object detection system using a C++ **WebSocket server** with **YOLO** for object detection, and a **React client** with Webcam integration for capturing and sending video frames. The server uses **OpenCV** to process frames and the **YOLO model** for detecting objects in the video stream.

<img src="https://i.giphy.com/media/v1.Y2lkPTc5MGI3NjExcWw2cHFycnAwbHJ2dzhma2ZieWc4OHpkdXk0dDYxdWZhNm95aWZuNiZlcD12MV9pbnRlcm5hbF9naWZfYnlfaWQmY3Q9Zw/8uwQcYcTHGawVJrvXz/giphy.gif" width="500"/>

## Running Locally

### Prerequisites

- C++ compiler (Clang or GCC)
- Node.js & npm (for React client)

### Steps

1. **Clone the repository**
    ```bash
    git clone https://github.com/martishin/react-cpp-object-detection
    cd react-cpp-object-detection
    ```
   
2. Install and configure vcpkg  
   Navigate to the server folder and install vcpkg
   ```bash
    cd server
    git clone https://github.com/microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    cd ..
    export VCPKG_ROOT=$(pwd)/vcpkg
    ```
3. Install server dependencies
   ```bash
    vcpkg install
    ```

4. **Build and run the WebSocket server (C++)**
    ```bash
    make run-server
    ```

5. **Running the React client**  
    Navigate to the client folder and install dependencies:

    ```bash
    cd ../client
    npm install
    npm run dev
    ```

    The client will be running at `http://localhost:5173/`.

## Technologies Used

- **C++ (Server):** For real-time frame processing and object detection using OpenCV and YOLO.
- **React (Client):** For capturing video frames and displaying object detection results.
- **WebSockets:** For efficient, low-latency, real-time communication between the client and the server.
- **YOLO:** A state-of-the-art object detection model.
- **OpenCV:** A popular computer vision library for image and video processing.
