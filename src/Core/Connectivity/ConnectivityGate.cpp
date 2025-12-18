#include "Contracts/Connectivity/ConnectivityGate.h"
#include "Contracts/Connectivity/IEventLatch.h"

namespace iotsmartsys::core {

ConnectivityGate& ConnectivityGate::instance() {
  static ConnectivityGate g;
  return g;
}

void ConnectivityGate::init(IEventLatch& latch) {
  instance()._latch = &latch;
}

uint32_t ConnectivityGate::bits() const {
  return _latch ? _latch->get() : 0u;
}

bool ConnectivityGate::isSet(uint32_t requiredBits) const {
  const auto b = bits();
  return (b & requiredBits) == requiredBits;
}

bool ConnectivityGate::waitBits(uint32_t requiredBits, int32_t timeoutMs) {
  return _latch ? _latch->waitAll(requiredBits, timeoutMs) : false;
}

void ConnectivityGate::setBits(uint32_t bitsToSet) {
  if (_latch) _latch->set(bitsToSet);
}

void ConnectivityGate::clearBits(uint32_t bitsToClear) {
  if (_latch) _latch->clear(bitsToClear);
}

} // namespace iotsmartsys::core