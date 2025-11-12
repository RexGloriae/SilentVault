#include "wrap.hxx"

#include <stdexcept>

std::vector<char> PythonWrapper::encrypt(const std::vector<char>& key,
                                         const std::vector<char>& iv,
                                         const std::vector<char>& plain) {
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString("../CryptoService"));
    PyObject* py_module = PyImport_ImportModule("aes");
    PyObject* py_func = PyObject_GetAttrString(py_module, "enc");

    PyObject* py_key = PyBytes_FromStringAndSize(key.data(), key.size());
    PyObject* py_iv = PyBytes_FromStringAndSize(iv.data(), iv.size());
    PyObject* py_plain =
        PyBytes_FromStringAndSize(plain.data(), plain.size());

    PyObject* args = PyTuple_Pack(3, py_key, py_iv, py_plain);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(sys_path);
    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_key);
    Py_DECREF(py_iv);
    Py_DECREF(py_plain);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        throw std::runtime_error("AES Encryption Failed");
    }

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        throw std::runtime_error("Failed to convert Python bytes");
    }

    Py_DECREF(result);

    return std::vector<char>(buffer, buffer + len);
}

std::vector<char> PythonWrapper::decrypt(const std::vector<char>& key,
                                         const std::vector<char>& iv,
                                         const std::vector<char>& cipher) {
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString("../CryptoService"));
    PyObject* py_module = PyImport_ImportModule("aes");
    PyObject* py_func = PyObject_GetAttrString(py_module, "decr");

    PyObject* py_key = PyBytes_FromStringAndSize(key.data(), key.size());
    PyObject* py_iv = PyBytes_FromStringAndSize(iv.data(), iv.size());
    PyObject* py_cipher =
        PyBytes_FromStringAndSize(cipher.data(), cipher.size());

    PyObject* args = PyTuple_Pack(3, py_key, py_iv, py_cipher);
    PyObject* result = PyObject_CallObject(py_func, args);

    if (!result) {
        PyErr_Print();
        throw std::runtime_error("AES Decryption failed");
    }

    Py_DECREF(sys_path);
    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_key);
    Py_DECREF(py_iv);
    Py_DECREF(py_cipher);
    Py_DECREF(args);

    char*      buffer;
    Py_ssize_t len;
    if (PyBytes_AsStringAndSize(result, &buffer, &len) == -1) {
        throw std::runtime_error("Failed to convert Python bytes");
    }

    Py_DECREF(result);

    return std::vector<char>(buffer, buffer + len);
}

std::string PythonWrapper::sha256(const std::string& data) {
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString("../../CryptoService/"));
    PyObject* py_module = PyImport_ImportModule("sha");
    PyObject* py_func = PyObject_GetAttrString(py_module, "hash");

    PyObject* py_data = PyUnicode_FromString(data.c_str());
    PyObject* args = PyTuple_Pack(1, py_data);
    PyObject* result = (PyObject_CallObject(py_func, args));

    if (!result) {
        PyErr_Print();
        throw std::runtime_error("Failed to hash data");
    }

    Py_DECREF(sys_path);
    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_data);
    Py_DECREF(args);

    Py_ssize_t  len;
    const char* buffer = PyUnicode_AsUTF8AndSize(result, &len);
    if (!buffer) {
        throw std::runtime_error("Failed to convert Python bytes");
    }

    Py_DECREF(result);

    return std::string(buffer, len);
}

void PythonWrapper::zip_files(const std::vector<std::string>& srcs,
                              const std::string&              dest) {
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString("../CryptoService"));
    PyObject* py_module = PyImport_ImportModule("zip");
    PyObject* py_func = PyObject_GetAttrString(py_module, "zip_file");

    PyObject* py_list = PyList_New(srcs.size());
    for (size_t i = 0; i < srcs.size(); ++i) {
        PyList_SetItem(py_list, i, PyUnicode_FromString(srcs[i].c_str()));
    }
    // to be continued...
    PyObject* py_dest = PyUnicode_FromString(dest.c_str());
    PyObject* args = PyTuple_Pack(2, py_list, py_dest);
    PyObject* result = PyObject_CallObject(py_func, args);

    Py_DECREF(sys_path);
    Py_DECREF(py_module);
    Py_DECREF(py_func);
    Py_DECREF(py_list);
    Py_DECREF(py_dest);
    Py_DECREF(args);

    if (!result) {
        PyErr_Print();
        throw std::runtime_error("Failed to zip files");
    }

    Py_DECREF(result);
}

void PythonWrapper::unzip(const std::string& archive,
                          const std::string& dest_dir) {
    PyObject* sys_path = PySys_GetObject("path");
    PyList_Append(sys_path, PyUnicode_FromString("../CryptoService"));
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
        throw std::runtime_error("Failed to unzip archive");
    }
    Py_DECREF(result);
}