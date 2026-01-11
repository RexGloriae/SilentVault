#ifndef PAYLOAD
#define PAYLOAD

#include <cstdint>
#include <string>
#include <vector>

enum OPCODE : uint8_t {
    POST_SALT = 0x01,
    GET_SALT = 0x02,
    GET_CHALLENGE = 0x03,
    POST_RESPONSE = 0x04,
    GET_AUTH_RESPONSE = 0x05,
    POST_SALT_REQUEST = 0x06,
    POST_CHALLENGE_REQUEST = 0x07,
    POST_UPLOAD = 0x08,
    GET_DOWNLOAD = 0x09,
    POST_DOWNLOAD_REQUEST = 0x0A,
    POST_LIST_REQUEST = 0x0B,
    GET_LIST = 0x0C,
    POST_DELETE_REQUEST = 0x0D
};

void add_int(std::vector<char>& vec, int value);

class Payload {
   protected:
    OPCODE            _opcode;
    int               _user_len;
    std::vector<char> _user;

   public:
    Payload(OPCODE opcode, std::string user);
};

class PostDeleteRequest : public Payload {
   protected:
    int               _filename_len;
    std::vector<char> _filename;

   public:
    PostDeleteRequest(std::vector<char> filename, std::string user);
    std::vector<char> serialize();
};

class PostListRequest : public Payload {
   public:
    PostListRequest(std::string user);
    std::vector<char> serialize();
};

class GetListResponse : public Payload {
   private:
    int                            _filenames_count;
    std::vector<std::vector<char>> _filenames;
    std::vector<char>              _payload;

   public:
    GetListResponse(std::vector<char> payload);
    bool deserialize();

    std::vector<std::vector<char>> filenames();
};

class PostUploadFile : public Payload {
   protected:
    int               _data_len;
    std::vector<char> _data;
    int               _filename_len;
    std::vector<char> _filename;

   public:
    PostUploadFile(std::vector<char> data,
                   std::vector<char> filename,
                   std::string       user);
    std::vector<char> serialize();
};

class PostDownloadRequest : public Payload {
   protected:
    int               _filename_len;
    std::vector<char> _filename;

   public:
    PostDownloadRequest(std::vector<char> filename, std::string user);
    std::vector<char> serialize();
};

class GetDownloadFile : PostUploadFile {
   private:
    std::vector<char> _payload;

   public:
    GetDownloadFile(std::vector<char> payload);
    bool deserialize();

    std::vector<char> file_data() { return _data; }
};

class PostChallengeRequestPayload : Payload {
   public:
    PostChallengeRequestPayload(std::string user);
    std::vector<char> serialize();
};

class PostSaltPayload : protected Payload {
   protected:
    int               _salt_len;
    std::vector<char> _salt;

   public:
    PostSaltPayload(std::string user, std::vector<char> salt);
    std::vector<char> serialize();

    std::vector<char> salt() { return _salt; }
};

class PostResponsePayload : Payload {
   private:
    int               _pub_len;
    std::vector<char> _pub;
    int               _response_len;
    std::vector<char> _response;
    int               _r_bytes_len;
    std::vector<char> _r_bytes;
    int               _challenge_len;
    std::vector<char> _challenge;

   public:
    PostResponsePayload(std::vector<char> pub,
                        std::vector<char> response,
                        std::vector<char> r_bytes,
                        std::vector<char> challenge,
                        std::string       user);

    std::vector<char> serialize();
};

class GetSaltPayload : public PostSaltPayload {
   private:
    std::vector<char> _payload;

   public:
    GetSaltPayload(std::vector<char> payload);
    bool deserialize();
};

class GetChallengePayload : Payload {
   private:
    std::vector<char> _payload;
    std::vector<char> _challenge;
    int               _challenge_len;

   public:
    GetChallengePayload(std::vector<char> payload);
    bool deserialize();

    std::vector<char> challenge() { return _challenge; }
};

class GetAuthResponsePayload : Payload {
   private:
    std::vector<char> _payload;
    bool              _response;

   public:
    GetAuthResponsePayload(std::vector<char> payload);
    bool deserialize();
    bool success() { return _response; }
};

class PostSaltRequestPayload : Payload {
   public:
    PostSaltRequestPayload(std::string user);
    std::vector<char> serialize();
};

#endif  // PAYLOAD