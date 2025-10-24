import hashlib

def hash(data: bytes) -> bytes:
    return hashlib.sha256(data).digest()