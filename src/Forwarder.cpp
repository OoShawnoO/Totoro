#include "Forwarder.h"

namespace totoro {

    int HttpForwarder::InitForwarder(const std::string &ip, unsigned int port) {
        return 0;
    }

    Connection::CallbackReturnType HttpForwarder::ForwarderReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpForwarder::ForwarderAfterWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType totoro::HttpForwarder::ReadCallback() {
        return HttpBase::ReadCallback();
    }

    Connection::CallbackReturnType totoro::HttpForwarder::AfterReadCallback() {
        return HttpBase::AfterReadCallback();
    }

    Connection::CallbackReturnType totoro::HttpForwarder::WriteCallback() {
        return HttpBase::WriteCallback();
    }

    Connection::CallbackReturnType totoro::HttpForwarder::AfterWriteCallback() {
        return HttpBase::AfterWriteCallback();
    }

    int HttpsForwarder::InitForwarder(const std::string &ip, unsigned int port) {
        return 0;
    }

    Connection::CallbackReturnType HttpsForwarder::ForwarderReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwarder::ForwarderAfterReadCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwarder::ForwarderWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwarder::ForwarderAfterWriteCallback() {
        return SUCCESS;
    }

    Connection::CallbackReturnType HttpsForwarder::ReadCallback() {
        return HttpsBase::ReadCallback();
    }

    Connection::CallbackReturnType HttpsForwarder::AfterReadCallback() {
        return HttpsBase::AfterReadCallback();
    }

    Connection::CallbackReturnType HttpsForwarder::WriteCallback() {
        return HttpsBase::WriteCallback();
    }

    Connection::CallbackReturnType HttpsForwarder::AfterWriteCallback() {
        return HttpsBase::AfterWriteCallback();
    }
} // totoro