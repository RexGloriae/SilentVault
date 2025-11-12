import hashlib

def hash(data: str) -> str:
    encoded = data.encode('utf-8')
    return hashlib.sha3_256(encoded).hexdigest()