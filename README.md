![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
<img src="https://img.shields.io/badge/Python-FFD43B?style=for-the-badge&logo=python&logoColor=blue"/>

# SilentVault

SilentVault is a **secured file server**, written in **C++**. It permits storing and accessing files on a remote server using a **Pasword-Based Zero Knowledge Proof(ZKP)** authentication mechanism. The project is oriented to real-life cryptographic security.

---

## Main Characteristics

* **Zero Knowledge** Authentication – the secret password is never sent to the server
* Secure communications with **TLS (OpenSSL)**
* Modular client-server architecture
* Resilient server to **offline attacks**

---

## Security Model

SilentVault completely eliminates authentication methods based on hashes. The server only stores a public value mathematically derived from the secret password, and the authentication is made by verifying an algebraic relation.

Security properties:

* Zero Knowledge
* Resilient replay attacks
* Resilient brute-force offline
* Server compromise ≠ passwords compromise

---

## The Math behind (brief)

The protocol is based on **discrete logarithms** in a cyclic group (eliptic curve):

* Secret derived from password: `x = KDF(password, salt)`
* Public Key: `Y = g^x`
* Commitment: `R = g^r`
* Challenge: `c`
* Response: `s = r + c·x`

Server checks:

```
g^s == R · Y^c
```

without finding the secret `x`.

---

## Build & Run

### Dependencies

* C++17
* OpenSSL
* Make
* Python

### Build

```bash
git clone https://www.github.com/RexGloriae/SilentVault/
chmod +x setup.sh
./setup.sh
```

### Run
Server:
```bash
./src/Server/bin/server
```
Client:
```bash
./src/Client/bin/client
```

---

## Used Technologies

* C++17
* OpenSSL
* Eliptic Curves
* Zero Knowledge Proof (Schnorr)
* TLS
