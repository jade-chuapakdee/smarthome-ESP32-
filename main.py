import serial

ser = serial.Serial('COM3', 115200, timeout=1)

print("Listening for Arduino RTT results...")

while True:
    try:
        data = ser.readline().decode(errors="ignore").strip()
        if data:
            print("Arduino ->", data)
        if data == "PING":
            ser.write(b"PONG\n")
    except Exception as e:
        print("Error:", e)
        break
