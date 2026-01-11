#ifndef PAYLOAD
#define PAYLOAD

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#define interface class

enum OPCODE : uint8_t {
    GET_SALT = 0x01,
    POST_SALT = 0x02,
    POST_CHALLENGE = 0x03,
    GET_RESPONSE = 0x04,
    POST_AUTH_RESPONSE = 0x05,
    GET_SALT_REQUEST = 0x06,
    GET_CHALLENGE_REQUEST = 0x07,
    GET_UPLOAD = 0x08,
    POST_DOWNLOAD = 0x09,
    GET_DOWNLOAD_REQUEST = 0x0A,
    GET_LIST_REQUEST = 0x0B,
    POST_LIST = 0x0C,
    GET_DELETE_REQUEST = 0x0D
};

void add_int(std::vector<char>& vec, int value);

interface IPayload {
   public:
    IPayload() = default;
    virtual ~IPayload() = default;
    virtual std::vector<char> serialize() = 0;
    virtual bool              deserialize() = 0;
    virtual std::string       user() const = 0;
    virtual std::vector<char> pub() const = 0;
    virtual std::vector<char> salt() const = 0;
    virtual std::vector<char> challenge() const = 0;
    virtual std::vector<char> r_bytes() const = 0;
    virtual std::vector<char> response() const = 0;
    virtual bool              auth() const = 0;
    virtual IPayload*         solve() = 0;
    virtual OPCODE            opcode() const = 0;
    virtual std::vector<char> filename() const = 0;
    virtual std::vector<char> data() const = 0;
};

class Payload : public IPayload {
   protected:
    OPCODE            _opcode;
    int               _user_len;
    std::vector<char> _user;

   public:
    Payload(OPCODE opcode, std::string user);
    ~Payload() = default;
    virtual std::vector<char> serialize() = 0;
    virtual bool              deserialize() = 0;
    virtual std::string       user() const override {
        return std::string(_user.begin(), _user.end());
    }
    virtual std::vector<char> pub() const {
        throw std::runtime_error("Not implemented");
    };
    virtual std::vector<char> salt() const {
        throw std::runtime_error("Not implemented");
    };
    virtual std::vector<char> challenge() const {
        throw std::runtime_error("Not implemented");
    };
    virtual std::vector<char> r_bytes() const {
        throw std::runtime_error("Not implemented");
    };
    virtual std::vector<char> response() const {
        throw std::runtime_error("Not implemented");
    };
    virtual bool auth() const {
        throw std::runtime_error("Not implemented");
    };
    virtual IPayload* solve() {
        throw std::runtime_error("Not implemented");
    };
    virtual OPCODE            opcode() const override { return _opcode; }
    virtual std::vector<char> filename() const {
        throw std::runtime_error("Not implemented");
    };
    virtual std::vector<char> data() const {
        throw std::runtime_error("Not implemented");
    };
};

class GetUpload : public Payload {
   private:
    int               _data_len;
    std::vector<char> _data;
    int               _filename_len;
    std::vector<char> _filename;
    std::vector<char> _payload;

   public:
    GetUpload(std::vector<char> payload);
    ~GetUpload() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;

    std::vector<char> filename() const override { return _filename; }
    std::vector<char> data() const override { return _data; }
};

class PostDownload : public Payload {
   private:
    int               _data_len;
    std::vector<char> _data;
    int               _filename_len;
    std::vector<char> _filename;

   public:
    PostDownload(std::vector<char> filename,
                 std::vector<char> data,
                 std::string       user);
    ~PostDownload() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;
};

class GetDownloadRequest : public Payload {
   private:
    int               _filename_len;
    std::vector<char> _filename;
    std::vector<char> _payload;

   public:
    GetDownloadRequest(std::vector<char> payload);
    ~GetDownloadRequest() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;

    std::vector<char> filename() const override { return _filename; }
};

class GetListRequest : public Payload {
   private:
    std::vector<char> _payload;

   public:
    GetListRequest(std::vector<char> payload);
    ~GetListRequest() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;
};

class PostListResponse : public Payload {
    int                            _filenames_count;
    std::vector<std::vector<char>> _filenames;

   public:
    PostListResponse(std::string                    user,
                     std::vector<std::vector<char>> filenames);
    std::vector<char> serialize();
    bool              deserialize();
};

class GetDeleteRequest : public Payload {
   private:
    int               _filename_len;
    std::vector<char> _filename;
    std::vector<char> _payload;

   public:
    GetDeleteRequest(std::vector<char> payload);
    ~GetDeleteRequest() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;

    std::vector<char> filename() const override { return _filename; }
};

class GetChallengeRequest : public Payload {
   private:
    std::vector<char> _payload;

   public:
    GetChallengeRequest(std::vector<char> payload);
    ~GetChallengeRequest() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;
};

class PostSaltPayload : public Payload {
   private:
    int               _salt_len;
    std::vector<char> _salt;

   public:
    PostSaltPayload(std::string user, std::vector<char> salt);
    ~PostSaltPayload() = default;
    std::vector<char> serialize() override;
    bool              deserialize() override;
    std::vector<char> salt() const override { return _salt; };
};

class GetSaltPayload : public Payload {
   private:
    int               _salt_len;
    std::vector<char> _salt;
    std::vector<char> _payload;

   public:
    GetSaltPayload(std::vector<char> payload);
    ~GetSaltPayload() = default;
    bool              deserialize() override;
    std::vector<char> serialize() override;
    std::vector<char> salt() const override { return _salt; };
};

class GetResponsePayload : public Payload {
   private:
    int               _pub_len;
    std::vector<char> _pub;
    int               _response_len;
    std::vector<char> _response;
    int               _r_bytes_len;
    std::vector<char> _r_bytes;
    int               _challenge_len;
    std::vector<char> _challenge;

    std::vector<char> _payload;

   public:
    GetResponsePayload(std::vector<char> payload);
    ~GetResponsePayload() = default;

    bool              deserialize() override;
    std::vector<char> serialize() override;
    std::vector<char> pub() const override { return _pub; };
    std::vector<char> response() const override { return _response; };
    std::vector<char> r_bytes() const override { return _r_bytes; };
    std::vector<char> challenge() const override { return _challenge; };
};

class PostChallengePayload : public Payload {
   private:
    std::vector<char> _challenge;
    int               _challenge_len;

   public:
    PostChallengePayload(std::string user, std::vector<char> challenge);
    ~PostChallengePayload() = default;
    std::vector<char> serialize() override;
    bool              deserialize() override;
    std::vector<char> challenge() const override { return _challenge; };
};

class PostAuthResponsePayload : public Payload {
   private:
    bool _auth_response;

   public:
    PostAuthResponsePayload(std::string user, bool auth_response);
    ~PostAuthResponsePayload() = default;
    std::vector<char> serialize() override;
    bool              deserialize() override;
    bool              auth() const override { return _auth_response; };
};

class GetSaltRequestPayload : public Payload {
   private:
    std::vector<char> _payload;

   public:
    GetSaltRequestPayload(std::vector<char> payload);
    ~GetSaltRequestPayload() = default;

    std::vector<char> serialize() override;
    bool              deserialize() override;
};

class PayloadCreator {
   public:
    PayloadCreator() = default;
    ~PayloadCreator() = default;

    static IPayload* interpret_payload(std::vector<char> payload);

    static IPayload* create_post_salt_payload(std::string       user,
                                              std::vector<char> salt);

    static IPayload* create_get_response_payload(
        std::vector<char> payload);

    static IPayload* create_get_salt_payload(std::vector<char> payload);

    static IPayload* create_post_challenge_payload(
        std::string user, std::vector<char> challenge);

    static IPayload* create_post_auth_response_payload(std::string user,
                                                       bool);

    static IPayload* create_get_salt_request_payload(
        std::vector<char> payload);

    static IPayload* create_get_challenge_request_payload(
        std::vector<char> payload);

    static IPayload* create_get_list_request_payload(
        std::vector<char> payload);
    static IPayload* create_get_delete_request_payload(
        std::vector<char> payload);
    static IPayload* create_get_upload_payload(std::vector<char> payload);
    static IPayload* create_get_download_request_payload(
        std::vector<char> payload);
    static IPayload* create_post_download_response_payload(
        std::string       user,
        std::vector<char> filename,
        std::vector<char> data);
    static IPayload* create_post_list_response_payload(
        std::string user, std::vector<std::vector<char>> filenames);
};

#endif  // PAYLOAD