from Crypto.Protocol.KDF import PBKDF2
from Crypto.Hash import SHA3_256
from Crypto.PublicKey import ECC
from Crypto.Random import random
from Crypto.Random import get_random_bytes

def _get_curve_info(curve_name = 'P-256'):
    curves = ECC._curves
    if curve_name not in curves:
        raise ValueError(f"Curve {curve_name} not found")
    return curves[curve_name]

def point_to_bytes(point, curve_name = 'P-256'):
    curve = _get_curve_info(curve_name)
    coord_size = (int(curve.p).bit_length() + 7) // 8

    x_bytes = int(point.x).to_bytes(coord_size, 'big')
    y_bytes = int(point.y).to_bytes(coord_size, 'big')
    return x_bytes + y_bytes

def bytes_to_point(b, curve_name = 'P-256'):
    curve = _get_curve_info(curve_name)
    coord_size = (int(curve.p).bit_length() + 7) // 8
    if len(b) != coord_size * 2:
        raise ValueError("Invalid ECC point length")

    x = int.from_bytes(b[:coord_size], 'big')
    y = int.from_bytes(b[coord_size:], 'big')

    return ECC.EccPoint(x, y, curve=curve_name)

def int_to_bytes_be(i, length):
    return int(i).to_bytes(length, 'big')

def bytes_be_to_int(b):
    return int.from_bytes(b, 'big')

def password_to_scalar(password: str,
                       salt: bytes = None,
                       iterations: int = 200000,
                       dklen: int = 32,
                       curve_name: str = 'P-256'):
    if salt is None:
        salt=get_random_bytes(16)
    if not isinstance(salt, bytes):
        raise TypeError("salt must be bytes")
    if not isinstance(password, str):
        raise TypeError("password must be a str")
    
    dk = PBKDF2(password.encode('utf-8'), salt, dklen, count=iterations, hmac_hash_module=SHA3_256)

    curve = _get_curve_info(curve_name)
    n = int(curve.order)
    k = int.from_bytes(dk, 'big') % n
    if k == 0:
        k = 1
    return k, salt


# client functions
def client_public_from_password(password: str,
                                salt: bytes = None,
                                iterations: int = 200000,
                                dklen: int = 32,
                                curve_name: str = 'P-256'):
    x, salt_used = password_to_scalar(password, salt=salt, iterations=iterations, dklen=dklen, curve_name=curve_name)
    curve = _get_curve_info(curve_name)
    Gx, Gy = int(curve.Gx), int(curve.Gy)
    G_point = ECC.EccPoint(Gx, Gy, curve=curve_name)
    P = G_point * x  # P = x*G
    return point_to_bytes(P, curve_name=curve_name), salt_used

def client_create_commit(password: str,
                         salt: bytes = None,
                         iterations: int = 200000,
                         dklen: int = 32,
                         curve_name: str = 'P-256'):
    x, salt_used = password_to_scalar(password, salt=salt, iterations=iterations, dklen=dklen, curve_name=curve_name)
    return client_create_commit_from_scalar(x, curve_name=curve_name), salt_used

def client_create_commit_from_scalar(x_scalar: int, curve_name: str = 'P-256'):
    curve = _get_curve_info(curve_name)
    n = int(curve.order)
    Gx, Gy = int(curve.Gx), int(curve.Gy)
    G_point = ECC.EccPoint(Gx, Gy, curve=curve_name)

    # choose ephemeral nonce r
    r = random.StrongRandom().randint(1, n - 1)
    R = G_point * r
    return point_to_bytes(R, curve_name=curve_name), r.to_bytes(32, 'big')

def client_compute_response_from_scalar(r_scalar: int, x_scalar: int, challenge_int: int, curve_name: str = 'P-256'):
    curve = _get_curve_info(curve_name)
    n = int(curve.order)
    s = (r_scalar + (challenge_int % n) * (x_scalar % n)) % n
    return s.to_bytes(32, 'big')

def client_compute_response(password: str,
                            r_bytes: bytes,
                            salt: bytes,
                            challenge_bytes: bytes,
                            iterations: int = 200000,
                            dklen: int = 32,
                            curve_name: str = 'P-256'):

    r_scalar = int.from_bytes(r_bytes, 'big')
    challenge_int = int.from_bytes(challenge_bytes, 'big')
    x, _ = password_to_scalar(password, salt=salt, iterations=iterations, dklen=dklen, curve_name=curve_name)
    return client_compute_response_from_scalar(r_scalar, x, challenge_int, curve_name=curve_name)


# server functions
def server_gen_challenge(num_bytes: int = 32):
    b = get_random_bytes(num_bytes)
    return b

def server_verify(commit_bytes: bytes,
                  response_bytes: bytes,
                  public_key_bytes: bytes,
                  challenge_bytes: bytes,
                  curve_name: str = 'P-256'):
    response_s = int.from_bytes(response_bytes, 'big')
    challenge_int = int.from_bytes(challenge_bytes, 'big')
    curve = _get_curve_info(curve_name)
    n = int(curve.order)
    Gx, Gy = int(curve.Gx), int(curve.Gy)
    G = ECC.EccPoint(Gx, Gy, curve=curve_name)

    # deserialize points
    R = bytes_to_point(commit_bytes, curve_name=curve_name)
    P = bytes_to_point(public_key_bytes, curve_name=curve_name)

    # compute left = s*G
    s_mod = int(response_s) % n
    left = G * s_mod

    # compute right = R + c*P
    c_mod = int(challenge_int) % n
    right = R + (P * c_mod)

    return (left.x == right.x) and (left.y == right.y)


# flow:
# 1. client puts password
# 2. client calls client_public_from_password(pw) and gets pub_bytes, salt
# 3. client creates commit from scalar (generate password_to_scalar with pw and salt)
# and received R_byes, r
# 4. server generates challenge
# 5. client computes a response with said challenge, receives s
# 6. server verifies with R_bytes, s, pub_bytes & c, returns ok(1/0)