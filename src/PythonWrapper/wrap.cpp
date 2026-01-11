#include "wrap.hxx"

#include <stdexcept>

PythonWrapper::PythonWrapper() {
    Py_Initialize();
    m_thread_state = PyEval_SaveThread();
    
    // Initialize path once
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* sys_path = PySys_GetObject("path");
    // Note: PySys_GetObject returns a borrowed reference, do NOT decref it.
    PyList_Append(sys_path, PyUnicode_FromString("../../CryptoService/"));
    PyList_Append(sys_path,
                  PyUnicode_FromString(
                      "../../../venv/lib/python3.12/site-packages/"));
    PyGILState_Release(gstate);
}

PythonWrapper::~PythonWrapper() {
    PyEval_RestoreThread(m_thread_state);
    Py_Finalize();
}

std::vector<char> PythonWrapper::encrypt(const std::vector<char>& key,
                                         const std::vector<char>& iv,
                                         const std::vector<char>& plain) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    // sys_path is already set in constructor
    PyObject* py_module = PyImport_ImportModule("aes");
    PyObject* py_func = PyObject_GetAttrString(py_module, "enc");

    PyObject* py_key = PyBytes_FromStringAndSize(key.data(), key.size());
    PyObject* py_iv = PyBytes_FromStringAndSize(iv.data(), iv.size());
    PyObject* py_plain =
        PyBytes_FromStringAndSize(plain.data(), plain.size());

    PyObject* args = PyTuple_Pack(3, py_key, py_iv, py_plain);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_key);
    Py_DECREF(py_iv);
    Py_DECREF(py_plain);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("AES Encryption Failed");
    }

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        Py_DECREF(result);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert Python bytes");
    }

    std::vector<char> output(buffer, buffer + len);
    Py_DECREF(result);
    PyGILState_Release(gstate);
    return output;
}

std::vector<char> PythonWrapper::decrypt(const std::vector<char>& key,
                                         const std::vector<char>& iv,
                                         const std::vector<char>& cipher) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("aes");
    PyObject* py_func = PyObject_GetAttrString(py_module, "decr");

    PyObject* py_key = PyBytes_FromStringAndSize(key.data(), key.size());
    PyObject* py_iv = PyBytes_FromStringAndSize(iv.data(), iv.size());
    PyObject* py_cipher =
        PyBytes_FromStringAndSize(cipher.data(), cipher.size());

    PyObject* args = PyTuple_Pack(3, py_key, py_iv, py_cipher);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_key);
    Py_DECREF(py_iv);
    Py_DECREF(py_cipher);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("AES Decryption failed");
    }

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        Py_DECREF(result);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert Python bytes");
    }

    std::vector<char> output(buffer, buffer + len);
    Py_DECREF(result);
    PyGILState_Release(gstate);
    return output;
}

std::string PythonWrapper::sha256(const std::string& data) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("sha");
    PyObject* py_func = PyObject_GetAttrString(py_module, "hash");

    PyObject* py_data = PyUnicode_FromString(data.c_str());
    PyObject* args = PyTuple_Pack(1, py_data);
    PyObject* result = (PyObject_CallObject(py_func, args));

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_data);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to hash data");
    }

    Py_ssize_t  len;
    const char* buffer = PyUnicode_AsUTF8AndSize(result, &len);
    if (!buffer) {
        Py_DECREF(result);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert Python bytes");
    }

    std::string output(buffer, len);
    Py_DECREF(result);
    PyGILState_Release(gstate);
    return output;
}

void PythonWrapper::zip_files(const std::vector<std::string>& srcs,
                              const std::string&              dest) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zip");
    PyObject* py_func = PyObject_GetAttrString(py_module, "zip_file");

    PyObject* py_list = PyList_New(srcs.size());
    for (size_t i = 0; i < srcs.size(); ++i) {
        PyList_SetItem(py_list, i, PyUnicode_FromString(srcs[i].c_str()));
    }
    PyObject* py_dest = PyUnicode_FromString(dest.c_str());
    PyObject* args = PyTuple_Pack(2, py_list, py_dest);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_list);
    Py_DECREF(py_dest);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to zip files");
    }

    Py_DECREF(result);
    PyGILState_Release(gstate);
}

void PythonWrapper::unzip(const std::string& archive,
                          const std::string& dest_dir) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zip");
    PyObject* py_func = PyObject_GetAttrString(py_module, "unzip");

    PyObject* py_src = PyUnicode_FromString(archive.c_str());
    PyObject* py_dest = PyUnicode_FromString(dest_dir.c_str());
    PyObject* args = PyTuple_Pack(2, py_src, py_dest);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_src);
    Py_DECREF(py_dest);
    Py_DECREF(args);
    Py_DECREF(py_func);
    Py_DECREF(py_module);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to unzip archive");
    }
    Py_DECREF(result);
    PyGILState_Release(gstate);
}

std::pair<std::vector<char>, std::vector<char>>
PythonWrapper::client_pub_from_pass(std::string pass) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    // sys.path is handled in constructor
    PyObject* py_module = PyImport_ImportModule("zk_auth");
    PyObject* py_func =
        PyObject_GetAttrString(py_module, "client_public_from_password");

    PyObject* py_pw = PyUnicode_FromString(pass.c_str());
    PyObject* py_args = PyTuple_Pack(1, py_pw);
    PyObject* py_res = PyObject_CallObject(py_func, py_args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_pw);
    Py_DECREF(py_args);

    if (!py_res) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error(
            "Failled to call client_public_from_password");
    }

    PyObject* pub = PySequence_GetItem(py_res, 0);

    if (!pub) {
        PyErr_Print();
        Py_DECREF(py_res);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to get pub");
    }

    PyObject* salt = PySequence_GetItem(py_res, 1);
    if (!salt) {
        PyErr_Print();
        Py_DECREF(py_res);
        Py_DECREF(pub);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to get salt");
    }

    char*      pub_buff;
    Py_ssize_t pub_len;
    if (PyBytes_AsStringAndSize(pub, &pub_buff, &pub_len) == -1) {
        PyErr_Print();
        Py_DECREF(py_res);
        Py_DECREF(pub);
        Py_DECREF(salt);
        PyGILState_Release(gstate);
        throw std::runtime_error("Error converting pub");
    }
    std::vector<char> out_pub(pub_buff, pub_buff + pub_len);

    char*      salt_buff;
    Py_ssize_t salt_len;
    if (PyBytes_AsStringAndSize(salt, &salt_buff, &salt_len) == -1) {
        PyErr_Print();
        Py_DECREF(py_res);
        Py_DECREF(pub);
        Py_DECREF(salt);
        PyGILState_Release(gstate);
        throw std::runtime_error("Error converting salt");
    }
    std::vector<char> out_salt(salt_buff, salt_buff + salt_len);

    Py_DECREF(py_res);
    Py_DECREF(pub);
    Py_DECREF(salt);

    PyGILState_Release(gstate);
    return {out_pub, out_salt};
}

std::pair<std::vector<char>, std::vector<char>>
PythonWrapper::client_commit(std::string pass, std::vector<char> salt) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zk_auth");
    PyObject* py_func = PyObject_GetAttrString(
        py_module, "client_create_commit_from_scalar");

    PyObject* py_aux_func =
        PyObject_GetAttrString(py_module, "password_to_scalar");
    PyObject* py_pw = PyUnicode_FromString(pass.c_str());
    PyObject* py_salt =
        PyBytes_FromStringAndSize(salt.data(), salt.size());
    PyObject* aux_args = PyTuple_Pack(2, py_pw, py_salt);
    PyObject* aux_res = PyObject_CallObject(py_aux_func, aux_args);
    
    Py_DECREF(py_aux_func);
    Py_DECREF(py_pw);
    Py_DECREF(py_salt);
    Py_DECREF(aux_args);

    if (!aux_res) {
        PyErr_Print();
        Py_DECREF(py_module);
        Py_DECREF(py_func);
        PyGILState_Release(gstate);
        throw std::runtime_error("call password_to_scalar failed");
    }

    PyObject* scalar = PySequence_GetItem(aux_res, 0);
    Py_DECREF(aux_res);

    if (!scalar) {
        PyErr_Print();
        Py_DECREF(py_module);
        Py_DECREF(py_func);
        PyGILState_Release(gstate);
        throw std::runtime_error("Scalar is null");
    }
    PyObject* args = PyTuple_Pack(1, scalar);
    PyObject* res = PyObject_CallObject(py_func, args);
    
    Py_DECREF(scalar);
    Py_DECREF(args);
    Py_DECREF(py_module);
    Py_DECREF(py_func);

    if (!res) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to call commit function");
    }
    PyObject*  py_R_bytes = PySequence_GetItem(res, 0);
    char*      R_bytes_buff;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(py_R_bytes, &R_bytes_buff, &len) == -1) {
        PyErr_Print();
        Py_DECREF(res);
        if (py_R_bytes) Py_DECREF(py_R_bytes);
         PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convery bytes to string");
    }
    std::vector<char> out_R_bytes(R_bytes_buff, R_bytes_buff + len);

    PyObject*  py_r = PySequence_GetItem(res, 1);
    char*      r_buff;
    Py_ssize_t r_len;
    if (PyBytes_AsStringAndSize(py_r, &r_buff, &r_len) == -1) {
        PyErr_Print();
        Py_DECREF(res);
        Py_DECREF(py_R_bytes);
        if (py_r) Py_DECREF(py_r);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert bytes to string");
    }

    std::vector<char> out_r(r_buff, r_buff + r_len);

    Py_DECREF(res);
    Py_DECREF(py_R_bytes);
    Py_DECREF(py_r);

    PyGILState_Release(gstate);
    return {out_R_bytes, out_r};
}

std::vector<char> PythonWrapper::client_compute_response(
    std::string       pass,
    std::vector<char> r,
    std::vector<char> salt,
    std::vector<char> c) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zk_auth");
    PyObject* py_func =
        PyObject_GetAttrString(py_module, "client_compute_response");

    PyObject* py_pass = PyUnicode_FromString(pass.c_str());
    PyObject* py_r = PyBytes_FromStringAndSize(r.data(), r.size());
    PyObject* py_salt =
        PyBytes_FromStringAndSize(salt.data(), salt.size());
    PyObject* py_c = PyBytes_FromStringAndSize(c.data(), c.size());

    PyObject* args = PyTuple_Pack(4, py_pass, py_r, py_salt, py_c);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_pass);
    Py_DECREF(py_r);
    Py_DECREF(py_salt);
    Py_DECREF(py_c);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("client_compute_response failed");
    }

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        Py_DECREF(result);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert Python bytes");
    }

    std::vector<char> output(buffer, buffer + len);
    Py_DECREF(result);

    PyGILState_Release(gstate);
    return output;
}

std::vector<char> PythonWrapper::server_gen_challenge() {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zk_auth");
    PyObject* py_func =
        PyObject_GetAttrString(py_module, "server_gen_challenge");

    PyObject* result = PyObject_CallObject(py_func, nullptr);

    Py_DECREF(py_module);
    Py_DECREF(py_func);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("server_gen_challenge failed");
    }

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        Py_DECREF(result);
        PyGILState_Release(gstate);
        throw std::runtime_error("Failed to convert Python bytes");
    }

    std::vector<char> output(buffer, buffer + len);
    Py_DECREF(result);

    PyGILState_Release(gstate);
    return output;
}

bool PythonWrapper::server_verify(std::vector<char> r_bytes,
                                  std::vector<char> response,
                                  std::vector<char> pub_key,
                                  std::vector<char> c) {
    PyGILState_STATE gstate = PyGILState_Ensure();
    PyObject* py_module = PyImport_ImportModule("zk_auth");
    PyObject* py_func = PyObject_GetAttrString(py_module, "server_verify");

    PyObject* py_rBytes =
        PyBytes_FromStringAndSize(r_bytes.data(), r_bytes.size());
    PyObject* py_response =
        PyBytes_FromStringAndSize(response.data(), response.size());
    PyObject* py_pub =
        PyBytes_FromStringAndSize(pub_key.data(), pub_key.size());
    PyObject* py_c = PyBytes_FromStringAndSize(c.data(), c.size());

    PyObject* args = PyTuple_Pack(4, py_rBytes, py_response, py_pub, py_c);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_rBytes);
    Py_DECREF(py_response);
    Py_DECREF(py_pub);
    Py_DECREF(py_c);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        PyGILState_Release(gstate);
        throw std::runtime_error("AES Decryption failed");
    }

    int is_valid = PyObject_IsTrue(result);
    Py_DECREF(result);

    PyGILState_Release(gstate);
    return is_valid;
}