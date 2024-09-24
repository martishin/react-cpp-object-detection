import { useEffect, useRef, useState, useCallback } from "react"
import Webcam from "react-webcam"

const WEBSOCKET_SERVER_URL = "ws://localhost:8080/ws"

const videoConstraints = {
  width: 640,
  height: 480,
  facingMode: "environment",
}

const Camera = () => {
  const webcamRef = useRef<Webcam>(null)
  const [receivedImage, setReceivedImage] = useState<string | null>(null)
  const socketRef = useRef<WebSocket | null>(null)

  useEffect(() => {
    socketRef.current = new WebSocket(WEBSOCKET_SERVER_URL)

    socketRef.current.onopen = () => {
      console.log("WebSocket connection established")
    }

    socketRef.current.onmessage = (event) => {
      console.log("Received data from server")
      const blob = event.data as Blob
      const reader = new FileReader()
      reader.onloadend = () => {
        const dataUrl = reader.result as string
        setReceivedImage(dataUrl)
      }
      reader.readAsDataURL(blob)
    }

    socketRef.current.onerror = (error) => {
      console.error("WebSocket error:", error)
    }

    socketRef.current.onclose = () => {
      console.log("WebSocket connection closed")
    }

    return () => {
      if (socketRef.current) {
        socketRef.current.close()
      }
    }
  }, [])

  const sendFramesToBackend = useCallback(() => {
    if (webcamRef.current && socketRef.current && socketRef.current.readyState === WebSocket.OPEN) {
      const imageSrc = webcamRef.current.getScreenshot({
        width: 640,
        height: 480,
      })

      if (imageSrc) {
        fetch(imageSrc)
          .then((res) => res.blob())
          .then((blob) => {
            socketRef.current?.send(blob)
          })
      } else {
        console.log("Failed to capture image")
      }
    } else {
      console.log("Webcam not ready or WebSocket not open")
    }
  }, [webcamRef, socketRef])

  // 60 FPS
  useEffect(() => {
    const intervalId = setInterval(sendFramesToBackend, 16)
    return () => clearInterval(intervalId)
  }, [sendFramesToBackend])

  return (
    <>
      <Webcam
        ref={webcamRef}
        audio={false}
        screenshotFormat="image/jpeg"
        videoConstraints={videoConstraints}
        style={{ width: 0, height: 0 }}
      />
      {receivedImage ? (
        <div>
          <div>
            <img src={receivedImage} alt="Processed Screenshot" />
          </div>
        </div>
      ) : (
        <p>Waiting for processed image...</p>
      )}
    </>
  )
}

export default Camera
