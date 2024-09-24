all:
	cmake -S . -B build
	cmake --build build

run-server: all
	./build/ws_yolo_server

clean:
	rm -rf build
