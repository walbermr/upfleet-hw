call activate env
g++ .\sketchSimu.cpp .\ipc\tcpclient.cpp -lws2_32
START python db-serial.py %*
TIMEOUT 10
a.exe