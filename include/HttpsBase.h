#ifndef TOTORO_HTTPSBASE_H
#define TOTORO_HTTPSBASE_H

#include "Connection.h"
#include "HttpBase.h"
#include "core/SSLSocket.h"

namespace totoro {

    class HttpsBase : public HttpBase,public SSLSocket{
    public:
        int Init(const ConnectionInitParameter &connectionInitParameter) override;

    protected:
        int ReadCallback() override;

        int AfterReadCallback() override;

        int WriteCallback() override;

        int AfterWriteCallback() override;

    public:
        int Close() override;
    };

} // totoro

#endif //TOTORO_HTTPSBASE_H
