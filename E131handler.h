#ifndef E131_HANDLER_H
#define E131_HANDLER_H

#include "E131.h"

#include <vector>

class E131Handler {
public:

  std::queue < packet > packetQueue;
}

#endif /* ifndef E131_HANDLER_H */
