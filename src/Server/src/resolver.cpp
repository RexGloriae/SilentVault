#include "../include/resolver.hxx"

#include "../../PythonWrapper/wrap.hxx"
#include "../include/data.hxx"

IPayload* Resolver::solve(IPayload* request) {
    switch (request->opcode()) {
        case GET_SALT_REQUEST:
            return solve(
                static_cast<const GetSaltRequestPayload&>(*request));
        case GET_SALT:
            return solve(static_cast<const GetSaltPayload&>(*request));
        case GET_RESPONSE:
            return solve(static_cast<const GetResponsePayload&>(*request));
        case GET_CHALLENGE_REQUEST:
            return solve(
                static_cast<const GetChallengeRequest&>(*request));
        case GET_DELETE_REQUEST:
            return solve(static_cast<const GetDeleteRequest&>(*request));
        case GET_DOWNLOAD_REQUEST:
            return solve(static_cast<const GetDownloadRequest&>(*request));
        case GET_LIST_REQUEST:
            return solve(static_cast<const GetListRequest&>(*request));
        case GET_UPLOAD:
            return solve(static_cast<const GetUpload&>(*request));
        default:
            throw std::runtime_error("Unknown opcode in Resolver");
    }
}

IPayload* Resolver::solve(const GetSaltRequestPayload& request) {
    std::string       user = request.user();
    Data&             data = Data::getInstance();
    std::vector<char> salt = data.read_salt(user);
    return PayloadCreator::create_post_salt_payload(user, salt);
}

IPayload* Resolver::solve(const GetSaltPayload& request) {
    std::string user = request.user();
    Data&       data = Data::getInstance();
    data.create_file(data.path(user, SALT));
    data.write_salt(user, request.salt());
    return nullptr;
}

IPayload* Resolver::solve(const GetResponsePayload& request) {
    std::string    user = request.user();
    PythonWrapper& wrapper = PythonWrapper::get();
    bool           ok = wrapper.server_verify(request.r_bytes(),
                                    request.response(),
                                    request.pub(),
                                    request.challenge());
    return PayloadCreator::create_post_auth_response_payload(user, ok);
}

IPayload* Resolver::solve(const GetChallengeRequest& request) {
    std::string       user = request.user();
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> challenge = wrapper.server_gen_challenge();
    return PayloadCreator::create_post_challenge_payload(user, challenge);
}

IPayload* Resolver::solve(const GetDeleteRequest& request) {
    std::string       user = request.user();
    std::vector<char> filename = request.filename();
    Data&             data = Data::getInstance();
    data.delete_file(user, filename);
    return nullptr;
}
IPayload* Resolver::solve(const GetDownloadRequest& request) {
    std::string       user = request.user();
    std::vector<char> filename = request.filename();
    Data&             data = Data::getInstance();
    std::vector<char> file_data = data.get_file(user, filename);
    return PayloadCreator::create_post_download_response_payload(
        user, filename, file_data);
}
IPayload* Resolver::solve(const GetListRequest& request) {
    std::string                    user = request.user();
    Data&                          data = Data::getInstance();
    std::vector<std::vector<char>> filenames = data.list_files(user);
    return PayloadCreator::create_post_list_response_payload(user,
                                                             filenames);
}
IPayload* Resolver::solve(const GetUpload& request) {
    std::string       user = request.user();
    std::vector<char> filename = request.filename();
    std::vector<char> file_data = request.data();
    Data&             data = Data::getInstance();
    data.add_file(user, filename, file_data);
    return nullptr;
}