#include "../include/payload.hxx"

#include <cstring>

void add_int(std::vector<char>& vec, int value) {
    const char* bytes = reinterpret_cast<const char*>(&value);
    vec.insert(vec.end(), bytes, bytes + sizeof(int));
}

Payload::Payload(OPCODE opcode, std::string user) {
    _opcode = opcode;
    for (char c : user) {
        _user.push_back(c);
    }
    _user_len = user.size();
}

GetChallengeRequest::GetChallengeRequest(std::vector<char> payload)
    : Payload(GET_CHALLENGE_REQUEST, "") {
    _payload = payload;
}
std::vector<char> GetChallengeRequest::serialize() {
    return std::vector<char>();
}
bool GetChallengeRequest::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_CHALLENGE_REQUEST) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    return true;
}

PostSaltPayload::PostSaltPayload(std::string user, std::vector<char> salt)
    : Payload(POST_SALT, user) {
    _salt = salt;
    _salt_len = salt.size();
}
std::vector<char> PostSaltPayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _salt_len);
    for (char c : _salt) {
        buff.push_back(c);
    }
    return buff;
}
bool PostSaltPayload::deserialize() {
    // Not needed for this payload
    return false;
}

GetSaltPayload::GetSaltPayload(std::vector<char> payload)
    : Payload(GET_SALT, "") {
    _payload = payload;
}
bool GetSaltPayload::deserialize() {
    bool   success = true;
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_SALT) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_salt_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_salt_len < 0) return false;
    if (index + _salt_len > _payload.size()) return false;
    _salt.resize(_salt_len);
    for (int i = 0; i < _salt_len; i++) {
        _salt[i] = _payload[index++];
    }
    return success;
}
std::vector<char> GetSaltPayload::serialize() {
    // Not needed for this payload
    return std::vector<char>();
}

GetResponsePayload::GetResponsePayload(std::vector<char> payload)
    : Payload(GET_RESPONSE, "") {
    _payload = payload;
}
bool GetResponsePayload::deserialize() {
    bool   success = true;
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_RESPONSE) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_pub_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_pub_len < 0) return false;
    if (index + _pub_len > _payload.size()) return false;
    _pub.resize(_pub_len);
    for (int i = 0; i < _pub_len; i++) {
        _pub[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_response_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_response_len < 0) return false;
    if (index + _response_len > _payload.size()) return false;
    _response.resize(_response_len);
    for (int i = 0; i < _response_len; i++) {
        _response[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_r_bytes_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_r_bytes_len < 0) return false;
    if (index + _r_bytes_len > _payload.size()) return false;
    _r_bytes.resize(_r_bytes_len);
    for (int i = 0; i < _r_bytes_len; i++) {
        _r_bytes[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_challenge_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_challenge_len < 0) return false;
    if (index + _challenge_len > _payload.size()) return false;
    _challenge.resize(_challenge_len);
    for (int i = 0; i < _challenge_len; i++) {
        _challenge[i] = _payload[index++];
    }
    return success;
}
std::vector<char> GetResponsePayload::serialize() {
    // Not needed for this payload
    return std::vector<char>();
}

PostChallengePayload::PostChallengePayload(std::string       user,
                                           std::vector<char> challenge)
    : Payload(POST_CHALLENGE, user) {
    _challenge = challenge;
    _challenge_len = challenge.size();
}

std::vector<char> PostChallengePayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _challenge_len);
    for (char c : _challenge) {
        buff.push_back(c);
    }
    return buff;
}
bool PostChallengePayload::deserialize() {
    // Not needed for this payload
    return false;
}

PostAuthResponsePayload::PostAuthResponsePayload(std::string user,
                                                 bool        auth_response)
    : Payload(POST_AUTH_RESPONSE, user) {
    _auth_response = auth_response;
}

std::vector<char> PostAuthResponsePayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    buff.push_back(_auth_response);
    return buff;
}
bool PostAuthResponsePayload::deserialize() {
    // Not needed for this payload
    return false;
}

GetSaltRequestPayload::GetSaltRequestPayload(std::vector<char> payload)
    : Payload(GET_SALT_REQUEST, "") {
    _payload = payload;
}
std::vector<char> GetSaltRequestPayload::serialize() {
    return std::vector<char>();
}
bool GetSaltRequestPayload::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_SALT_REQUEST) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    return true;
}

GetUpload::GetUpload(std::vector<char> payload) : Payload(GET_UPLOAD, "") {
    _payload = payload;
}
std::vector<char> GetUpload::serialize() { return std::vector<char>(); }
bool              GetUpload::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_UPLOAD) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_data_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_data_len < 0) return false;
    if (index + _data_len > _payload.size()) return false;
    _data.resize(_data_len);
    for (int i = 0; i < _data_len; i++) {
        _data[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_filename_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_filename_len < 0) return false;
    if (index + _filename_len > _payload.size()) return false;
    _filename.resize(_filename_len);
    for (int i = 0; i < _filename_len; i++) {
        _filename[i] = _payload[index++];
    }
    return true;
}

PostDownload::PostDownload(std::vector<char> filename,
                           std::vector<char> data,
                           std::string       user)
    : Payload(POST_DOWNLOAD, user) {
    _filename = filename;
    _filename_len = filename.size();
    _data = data;
    _data_len = data.size();
}
std::vector<char> PostDownload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _data_len);
    for (char c : _data) {
        buff.push_back(c);
    }
    add_int(buff, _filename_len);
    for (char c : _filename) {
        buff.push_back(c);
    }
    return buff;
}
bool PostDownload::deserialize() {
    // Not needed for this payload
    return false;
}

GetDownloadRequest::GetDownloadRequest(std::vector<char> payload)
    : Payload(GET_DOWNLOAD_REQUEST, "") {
    _payload = payload;
}
std::vector<char> GetDownloadRequest::serialize() {
    return std::vector<char>();
}
bool GetDownloadRequest::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_DOWNLOAD_REQUEST) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_filename_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_filename_len < 0) return false;
    if (index + _filename_len > _payload.size()) return false;
    _filename.resize(_filename_len);
    for (int i = 0; i < _filename_len; i++) {
        _filename[i] = _payload[index++];
    }
    return true;
}

GetListRequest::GetListRequest(std::vector<char> payload)
    : Payload(GET_LIST_RESPONSE, "") {
    _payload = payload;
}
std::vector<char> GetListRequest::serialize() {
    return std::vector<char>();
}
bool GetListRequest::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_LIST_RESPONSE) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    return true;
}

PostListResponse::PostListResponse(
    std::string user, std::vector<std::vector<char>> filenames)
    : Payload(POST_LIST, user) {
    _filenames = filenames;
    _filenames_count = filenames.size();
}
std::vector<char> PostListResponse::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _filenames_count);
    for (const auto& fname : _filenames) {
        int fname_len = fname.size();
        add_int(buff, fname_len);
        for (char c : fname) {
            buff.push_back(c);
        }
    }
    return buff;
}
bool PostListResponse::deserialize() {
    // Not needed for this payload
    return false;
}

GetDeleteRequest::GetDeleteRequest(std::vector<char> payload)
    : Payload(GET_DELETE_REQUEST, "") {
    _payload = payload;
}
std::vector<char> GetDeleteRequest::serialize() {
    return std::vector<char>();
}
bool GetDeleteRequest::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_DELETE_REQUEST) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_filename_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_filename_len < 0) return false;
    if (index + _filename_len > _payload.size()) return false;
    _filename.resize(_filename_len);
    for (int i = 0; i < _filename_len; i++) {
        _filename[i] = _payload[index++];
    }
    return true;
}

IPayload* PayloadCreator::create_post_download_response_payload(
    std::string user, std::vector<char> filename, std::vector<char> data) {
    return new PostDownload(filename, data, user);
}
IPayload* PayloadCreator::create_post_list_response_payload(
    std::string user, std::vector<std::vector<char>> filenames) {
    return new PostListResponse(user, filenames);
}
IPayload* PayloadCreator::create_get_download_request_payload(
    std::vector<char> payload) {
    return new GetDownloadRequest(payload);
}
IPayload* PayloadCreator::create_get_list_request_payload(
    std::vector<char> payload) {
    return new GetListRequest(payload);
}
IPayload* PayloadCreator::create_get_delete_request_payload(
    std::vector<char> payload) {
    return new GetDeleteRequest(payload);
}
IPayload* PayloadCreator::create_get_upload_payload(
    std::vector<char> payload) {
    return new GetUpload(payload);
}

IPayload* PayloadCreator::create_post_auth_response_payload(
    std::string user, bool auth_response) {
    return new PostAuthResponsePayload(user, auth_response);
}

IPayload* PayloadCreator::create_post_challenge_payload(
    std::string user, std::vector<char> challenge) {
    return new PostChallengePayload(user, challenge);
}

IPayload* PayloadCreator::create_get_salt_payload(
    std::vector<char> payload) {
    return new GetSaltPayload(payload);
}

IPayload* PayloadCreator::create_post_salt_payload(
    std::string user, std::vector<char> salt) {
    return new PostSaltPayload(user, salt);
}

IPayload* PayloadCreator::create_get_response_payload(
    std::vector<char> payload) {
    return new GetResponsePayload(payload);
}

IPayload* PayloadCreator::create_get_salt_request_payload(
    std::vector<char> payload) {
    return new GetSaltRequestPayload(payload);
}

IPayload* PayloadCreator::create_get_challenge_request_payload(
    std::vector<char> payload) {
    return new GetChallengeRequest(payload);
}

IPayload* PayloadCreator::interpret_payload(std::vector<char> payload) {
    if (payload.empty()) return nullptr;
    switch (static_cast<OPCODE>(payload[0])) {
        case GET_SALT:
            return PayloadCreator::create_get_salt_payload(payload);
        case GET_SALT_REQUEST:
            return PayloadCreator::create_get_salt_request_payload(
                payload);
        case GET_RESPONSE:
            return PayloadCreator::create_get_response_payload(payload);
        case GET_CHALLENGE_REQUEST:
            return PayloadCreator::create_get_challenge_request_payload(
                payload);
        default:
            return nullptr;
    }
}