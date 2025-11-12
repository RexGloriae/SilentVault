/*
Wrapper for calling functions from the Python module.
Part of the Services category.
*/
#ifndef PY_WRAP_H
#define PY_WRAP_H

extern "C" {
#include "python3/Python.h"
}

#include <string>
#include <vector>

class PythonWrapper {
   public:
    static PythonWrapper& get() {
        static PythonWrapper instance;
        return instance;
    }

    std::vector<char> encrypt(const std::vector<char>& key,
                              const std::vector<char>& iv,
                              const std::vector<char>& plain);

    std::vector<char> decrypt(const std::vector<char>& key,
                              const std::vector<char>& iv,
                              const std::vector<char>& cipher);

    std::string sha256(const std::string& data);

    void zip_files(const std::vector<std::string>& srcs,
                   const std::string&              dest);

    void unzip(const std::string& archive, const std::string& dest_dir);

   private:
    PythonWrapper() { Py_Initialize(); }
    ~PythonWrapper() { Py_Finalize(); }
    PythonWrapper(const PythonWrapper& O) = delete;
    PythonWrapper& operator=(const PythonWrapper& O) = delete;
    PythonWrapper(PythonWrapper&& O) = delete;
    PythonWrapper& operator=(PythonWrapper&& O) = delete;
};

#endif  // PY_WRAP_H