#include "../include/resolver.hxx"

#include "../../PythonWrapper/wrap.hxx"
#include "../include/data.hxx"

IPayload* Resolver::solve(IPayload* request) {
    switch (request->opcode()) {
        case GET_PUB_AND_SALT_REQUEST:
            return solve(
                static_cast<const GetPubAndSaltRequestPayload&>(*request));
        case GET_PUB_AND_SALT:
            return solve(
                static_cast<const GetPublicAndSaltPayload&>(*request));
        case GET_RESPONSE:
            return solve(static_cast<const GetResponsePayload&>(*request));
        case GET_CHALLENGE_REQUEST:
            return solve(
                static_cast<const GetChallengeRequest&>(*request));
        default:
            throw std::runtime_error("Unknown opcode in Resolver");
    }
}

IPayload* Resolver::solve(const GetPubAndSaltRequestPayload& request) {
    std::string       user = request.user();
    Data&             data = Data::getInstance();
    std::vector<char> pub = data.read_public_key(user);
    std::vector<char> salt = data.read_salt(user);
    return PayloadCreator::create_post_public_and_salt_payload(
        user, pub, salt);
}

IPayload* Resolver::solve(const GetPublicAndSaltPayload& request) {
    std::string user = request.user();
    Data&       data = Data::getInstance();
    data.create_file(data.path(user, PUB));
    data.create_file(data.path(user, SALT));
    data.write_public_key(user, request.pub());
    data.write_salt(user, request.salt());
    return nullptr;
}

IPayload* Resolver::solve(const GetResponsePayload& request) {
    std::string       user = request.user();
    PythonWrapper&    wrapper = PythonWrapper::get();
    Data&             data = Data::getInstance();
    std::vector<char> pub_key = data.read_public_key(user);
    bool              ok = wrapper.server_verify(request.r_bytes(),
                                    request.response(),
                                    pub_key,
                                    request.challenge());
    return PayloadCreator::create_post_auth_response_payload(user, ok);
}

IPayload* Resolver::solve(const GetChallengeRequest& request) {
    std::string       user = request.user();
    PythonWrapper&    wrapper = PythonWrapper::get();
    std::vector<char> challenge = wrapper.server_gen_challenge();
    return PayloadCreator::create_post_challenge_payload(user, challenge);
}