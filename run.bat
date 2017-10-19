g++ .\sketchSimu.cpp .\ipc\tcpclient.cpp .\sketch\abrasion.c -lws2_32 || goto :error

CALL activate env
START python db-serial.py %*
TIMEOUT 10
a.exe
goto :EOF

:error
echo "Compilation error.";