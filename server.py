import socket
import random
import time

def server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('localhost', 65432))
    server_socket.listen(1)
    print("Server is listening on port 65432...")

    conn, addr = server_socket.accept()
    print(f"Connection from {addr}")

    try:
        while True:
            number = random.randint(0, 100)
            conn.sendall(str(number).encode() + b'\n')
            time.sleep(0.01)
    except (BrokenPipeError, ConnectionResetError):
        print("Connection closed by client.")
    finally:
        conn.close()
        server_socket.close()

if __name__ == "__main__":
    server()
