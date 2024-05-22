/**
  ******************************************************************************
  * @file           : Server.h
  * @author         : huzhida
  * @brief          : None
  * @date           : 2024/5/21
  ******************************************************************************
  */

#ifndef TOTORO_SERVER_H
#define TOTORO_SERVER_H

#include "Acceptor.h"   /* Acceptor */

namespace totoro {

    template<class ConnType>
    using Server = Acceptor<Epoller<ConnType>>;

}


#endif //TOTORO_SERVER_H
