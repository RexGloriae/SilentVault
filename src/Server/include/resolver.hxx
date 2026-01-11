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
    IPayload* solve(const GetPubAndSaltRequestPayload& request);
    IPayload* solve(const GetPublicAndSaltPayload& request);
    IPayload* solve(const GetResponsePayload& request);
    IPayload* solve(const GetChallengeRequest& request);
};

#endif  // RESOLVER