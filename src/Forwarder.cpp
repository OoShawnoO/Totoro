#include "Forwarder.h"

namespace totoro {
    HttpForwarder::HttpForwarder(HttpBase &_client):client(_client) {}

    int HttpForwarder::WakeClient() {
        return 0;
    }

    int totoro::HttpForwarder::ReadCallback() {
        return HttpBase::ReadCallback();
    }

    int totoro::HttpForwarder::AfterReadCallback() {
        return HttpBase::AfterReadCallback();
    }

    int totoro::HttpForwarder::WriteCallback() {
        return HttpBase::WriteCallback();
    }

    int totoro::HttpForwarder::AfterWriteCallback() {
        return HttpBase::AfterWriteCallback();
    }

    HttpsForwarder::HttpsForwarder(HttpsBase &_client):client(_client) {}

    int HttpsForwarder::WakeClient() {
        return 0;
    }

    int HttpsForwarder::ReadCallback() {
        return HttpsBase::ReadCallback();
    }

    int HttpsForwarder::AfterReadCallback() {
        return HttpsBase::AfterReadCallback();
    }

    int HttpsForwarder::WriteCallback() {
        return HttpsBase::WriteCallback();
    }

    int HttpsForwarder::AfterWriteCallback() {
        return HttpsBase::AfterWriteCallback();
    }
} // totoro