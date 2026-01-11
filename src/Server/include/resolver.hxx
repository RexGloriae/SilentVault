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

    IPayload* solve(IPayload* request);
    IPayload* solve(const GetSaltRequestPayload& request);
    IPayload* solve(const GetSaltPayload& request);
    IPayload* solve(const GetResponsePayload& request);
    IPayload* solve(const GetChallengeRequest& request);
    IPayload* solve(const GetDeleteRequest& request);
    IPayload* solve(const GetDownloadRequest& request);
    IPayload* solve(const GetListRequest& request);
    IPayload* solve(const GetUpload& request);
};

#endif  // RESOLVER