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

    /**
     * @brief Zip a list of files.
     * @param srcs A vector of filenames for the files targeted for
     * archiving.
     * @param dest Name of the achieved archive.
     */
    void zip_files(const std::vector<std::string>& srcs,
                   const std::string&              dest);

    /**
     * @brief Unzip an archive.
     * @param archive Archive name.
     * @param dest_dir Directory in which to unzip the archive.
     */
    void unzip(const std::string& archive, const std::string& dest_dir);

    /**
     * @brief Compute public key for client for registration.
     * @param pass Client's password.
     * @return A pair with the public key on the first position and the
     * salt on the second one.
     */
    std::pair<std::vector<char>, std::vector<char>> client_pub_from_pass(
        std::string pass);
    /**
     * @brief Create client commit. First step for authentication.
     * @param pass Client's password.
     * @param salt Client's salt. Generated at public key computation.
     * @return A pair with R_bytes on the first position and R integer on
     * the second.
     */
    std::pair<std::vector<char>, std::vector<char>> client_commit(
        std::string pass, std::vector<char> salt);
    /**
     * @brief Compute client's response for the server-given challenge.
     * Third step for auth.
     * @param pass Client's password.
     * @param r R integer generated at commit.
     * @param salt Client's salt.
     * @param c Server's challenge.
     * @return Client's response.
     */
    std::vector<char> client_compute_response(std::string       pass,
                                              std::vector<char> r,
                                              std::vector<char> salt,
                                              std::vector<char> c);

    /**
     * @brief Generate challenge for authentication. Second step for auth.
     * @return Challenge integer.
     */
    std::vector<char> server_gen_challenge();
    /**
     * @brief Verify client's response. Forth and last step of auth.
     * @param r_bytes R_bytes generated at client's commit.
     * @param response Client's computated response.
     * @param pub_key Client's public key.
     * @param c Server's generated challenge.
     * @return `true` if the computated response is valid, else `false`.
     */
    bool server_verify(std::vector<char> r_bytes,
                       std::vector<char> response,
                       std::vector<char> pub_key,
                       std::vector<char> c);

   private:
    PythonWrapper();
    ~PythonWrapper();
    PythonWrapper(const PythonWrapper& O) = delete;
    PythonWrapper& operator=(const PythonWrapper& O) = delete;
    PythonWrapper(PythonWrapper&& O) = delete;
    PythonWrapper& operator=(PythonWrapper&& O) = delete;

   private:
    PyThreadState* m_thread_state;
};

#endif  // PY_WRAP_H