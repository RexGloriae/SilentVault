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

PostDeleteRequest::PostDeleteRequest(std::vector<char> filename,
                                     std::string       user)
    : Payload(POST_DELETE_REQUEST, user) {
    _filename = filename;
    _filename_len = filename.size();
}
std::vector<char> PostDeleteRequest::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _filename_len);
    for (char c : _filename) {
        buff.push_back(c);
    }
    return buff;
}

PostListRequest::PostListRequest(std::string user)
    : Payload(POST_LIST_REQUEST, user) {}
std::vector<char> PostListRequest::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    return buff;
}

GetListResponse::GetListResponse(std::vector<char> payload)
    : Payload(GET_LIST, "") {
    _payload = payload;
}
bool GetListResponse::deserialize() {
    size_t index = 0;
    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_LIST) {
        return false;
    }
    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }
    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_filenames_count, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_filenames_count < 0) return false;
    for (int i = 0; i < _filenames_count; i++) {
        if (index + sizeof(int) > _payload.size()) return false;
        int fname_len;
        memcpy(&fname_len, &_payload[index], sizeof(int));
        index += sizeof(int);
        if (fname_len < 0) return false;
        if (index + fname_len > _payload.size()) return false;
        std::vector<char> fname(fname_len);
        for (int j = 0; j < fname_len; j++) {
            fname[j] = _payload[index++];
        }
        _filenames.push_back(fname);
    }
    return true;
}

PostUploadFile::PostUploadFile(std::vector<char> data,
                               std::vector<char> filename,
                               std::string       user)
    : Payload(POST_UPLOAD, user) {
    _data = data;
    _data_len = data.size();
    _filename = filename;
    _filename_len = filename.size();
}
std::vector<char> PostUploadFile::serialize() {
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

PostDownloadRequest::PostDownloadRequest(std::vector<char> filename,
                                         std::string       user)
    : Payload(POST_DOWNLOAD_REQUEST, user) {
    _filename = filename;
    _filename_len = filename.size();
}
std::vector<char> PostDownloadRequest::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _filename_len);
    for (char c : _filename) {
        buff.push_back(c);
    }
    return buff;
}

GetDownloadFile::GetDownloadFile(std::vector<char> payload)
    : PostUploadFile({}, {}, "") {
    _payload = payload;
}
bool GetDownloadFile::deserialize() {
    size_t index = 0;

    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_DOWNLOAD) {
        return false;
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_data_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_data_len < 0) return false;
    if (index + _data_len > _payload.size()) return false;
    _data.resize(_data_len);
    for (int i = 0; i < _data_len; i++) {
        _data[i] = _payload[index++];
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_filename_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_filename_len < 0) return false;
    if (index + _filename_len > _payload.size()) return false;
    _filename.resize(_filename_len);
    for (int i = 0; i < _filename_len; i++) {
        _filename[i] = _payload[index++];
    }

    return true;
}

PostChallengeRequestPayload::PostChallengeRequestPayload(std::string user)
    : Payload(POST_CHALLENGE_REQUEST, user) {}

std::vector<char> PostChallengeRequestPayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    return buff;
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

PostResponsePayload::PostResponsePayload(std::vector<char> pub,
                                         std::vector<char> response,
                                         std::vector<char> r_bytes,
                                         std::vector<char> challenge,
                                         std::string       user)
    : Payload(POST_RESPONSE, user) {
    _pub = pub;
    _response = response;
    _r_bytes = r_bytes;
    _response_len = response.size();
    _pub_len = pub.size();
    _r_bytes_len = r_bytes.size();
    _challenge_len = challenge.size();
    _challenge = challenge;
}

std::vector<char> PostResponsePayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    add_int(buff, _pub_len);
    for (char c : _pub) {
        buff.push_back(c);
    }
    add_int(buff, _response_len);
    for (char c : _response) {
        buff.push_back(c);
    }
    add_int(buff, _r_bytes_len);
    for (char c : _r_bytes) {
        buff.push_back(c);
    }
    add_int(buff, _challenge_len);
    for (char c : _challenge) {
        buff.push_back(c);
    }
    return buff;
}

GetSaltPayload::GetSaltPayload(std::vector<char> payload)
    : PostSaltPayload({}, {}) {
    _payload = payload;
}

bool GetSaltPayload::deserialize() {
    size_t index = 0;

    if (_payload.empty()) return false;
    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_SALT) {
        return false;
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_salt_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_salt_len < 0) return false;
    if (index + _salt_len > _payload.size()) return false;
    _salt.resize(_salt_len);
    for (int i = 0; i < _salt_len; i++) {
        _salt[i] = _payload[index++];
    }

    return true;
}

GetChallengePayload::GetChallengePayload(std::vector<char> payload)
    : Payload({}, {}) {
    _payload = payload;
}

bool GetChallengePayload::deserialize() {
    size_t index = 0;

    if (_payload.empty()) return false;

    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_CHALLENGE) return false;

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }

    if (index + sizeof(int) > _payload.size()) return false;
    memcpy(&_challenge_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_challenge_len < 0) return false;
    if (index + _challenge_len > _payload.size()) return false;
    _challenge.resize(_challenge_len);
    for (int i = 0; i < _challenge_len; i++) {
        _challenge[i] = _payload[index++];
    }

    return true;
}

GetAuthResponsePayload::GetAuthResponsePayload(std::vector<char> payload)
    : Payload({}, {}) {
    _payload = payload;
}

bool GetAuthResponsePayload::deserialize() {
    size_t index = 0;

    if (_payload.empty()) return false;

    _opcode = static_cast<OPCODE>(_payload[index++]);
    if (_opcode != GET_AUTH_RESPONSE) return false;

    if (index + sizeof(int) > _payload.size()) return false;
    std::memcpy(&_user_len, &_payload[index], sizeof(int));
    index += sizeof(int);
    if (_user_len < 0) return false;
    if (index + _user_len > _payload.size()) return false;
    _user.resize(_user_len);
    for (int i = 0; i < _user_len; i++) {
        _user[i] = _payload[index++];
    }

    if (index >= _payload.size()) return false;
    _response = _payload[index++];
    return true;
}

PostSaltRequestPayload::PostSaltRequestPayload(std::string user)
    : Payload(POST_SALT_REQUEST, user) {}

std::vector<char> PostSaltRequestPayload::serialize() {
    std::vector<char> buff;
    buff.push_back(_opcode);
    add_int(buff, _user_len);
    for (char c : _user) {
        buff.push_back(c);
    }
    return buff;
}