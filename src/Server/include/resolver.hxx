#ifndef RESOLVER
#define RESOLVER

#include "../include/payload.hxx"

class Resolver {
   private:
    Resolver() = default;
    ~Resolver() = default;
    Resolver(const Resolver&) = delete;
    Resolver& operator=(const Resolver&) = delete;
    Resolver(Resolver&&) = delete;
    Resolver& operator=(Resolver&&) = delete;

   public:
    static Resolver& get() {
        static Resolver instance;
        return instance;
    }

    IPayload* solve(IPayload* request, std::string client_id);
    IPayload* solve(const GetSaltRequestPayload& request,
                    std::string                  client_id);
    IPayload* solve(const GetSaltPayload& request, std::string client_id);
    IPayload* solve(const GetResponsePayload& request,
                    std::string               client_id);
    IPayload* solve(const GetChallengeRequest& request,
                    std::string                client_id);
    IPayload* solve(const GetDeleteRequest& request,
                    std::string             client_id);
    IPayload* solve(const GetDownloadRequest& request,
                    std::string               client_id);
    IPayload* solve(const GetListRequest& request, std::string client_id);
    IPayload* solve(const GetUpload& request, std::string client_id);
};

#endif  // RESOLVER