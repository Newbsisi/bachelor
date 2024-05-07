import ctypes
import socket

# Load the shared library containing Dilithium functions
dilithium_lib = ctypes.CDLL("ref/libpqcrystals_dilithium3_ref.so")  # Update with the correct path

# Define the function prototypes
# Example for keypair function
dilithium_lib.pqcrystals_dilithium2_ref_keypair.restype = ctypes.c_int
dilithium_lib.pqcrystals_dilithium2_ref_keypair.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_uint8)]

# Example for signature function
dilithium_lib.pqcrystals_dilithium2_ref_signature.restype = ctypes.c_int
dilithium_lib.pqcrystals_dilithium2_ref_signature.argtypes = [ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint8)]

# Define networking parameters
HOST = '192.168.1.100'  # Replace with the IP address of the target Raspberry Pi
PORT = 12345  # Choose a suitable port

def send_message(message):
    # Create a socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Connect to the server
        s.connect((HOST, PORT))
        # Send the message
        s.sendall(message.encode())
        # Receive data from the server (optional)
        data = s.recv(1024)
        print('Received', repr(data))

# Example function to generate key pair
def generate_key_pair():
    pk = ctypes.create_string_buffer(1952)
    sk = ctypes.create_string_buffer(4032)
    if dilithium_lib.pqcrystals_dilithium3_ref_keypair(pk, sk) != 0:
        raise ValueError("Key pair generation failed")
    return pk.raw, sk.raw

# Example function to sign a message
def sign_message(message, sk):
    sig = ctypes.create_string_buffer(3293)
    siglen = ctypes.c_size_t(0)
    if dilithium_lib.pqcrystals_dilithium3_ref_signature(sig, ctypes.byref(siglen), message, len(message), sk) != 0:
        raise ValueError("Signature creation failed")
    return sig.raw[:siglen.value]

# Example function to verify a signature
def verify_signature(sig, message, pk):
    return dilithium_lib.pqcrystals_dilithium3_ref_verify(sig, len(sig), message, len(message), pk) == 0

if __name__ == "__main__":
    # Example usage
    pk, sk = generate_key_pair()
    message = "Hello, World!"
    signature = sign_message(message.encode(), sk)
    print("Signature:", signature)
    if verify_signature(signature, message.encode(), pk):
        print("Signature verified successfully.")
        send_message(message)
    else:
        print("Signature verification failed.")
