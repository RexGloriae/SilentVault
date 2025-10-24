from Cryptodome.Cipher import AES
from Cryptodome.Random import get_random_bytes

def pad(data: bytes) -> bytes:
    pad_len = AES.block_size - len(data) % AES.block_size
    return data + bytes([pad_len] * pad_len)

def unpad(data: bytes) -> bytes:
    padding_len = data[-1]
    if padding_len > AES.block_size:
        raise ValueError("Padding value is invalid")
    return data[:-padding_len]

def enc(key:bytes, iv:bytes, plain_text:bytes) -> bytes:
    cipher = AES.new(key, AES.MODE_CBC, iv)
    padded = pad(plain_text)
    ciphertext = cipher.encrypt(padded)
    return ciphertext

def decr(key:bytes, iv:bytes, cipher_text:bytes) -> bytes:
    cipher = AES.new(key, AES.MODE_CBC, iv)
    padded_plain = cipher.decrypt(cipher_text)
    return unpad(padded_plain)
