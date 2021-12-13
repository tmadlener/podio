#include "podio/Frame.h"
#include "podio/ROOTRawData.h"
#include "podio/UnpackingPolicies.h"

#include "catch2/catch_test_macros.hpp"

TEST_CASE("Frame", "[event][basics]") {
  auto d = std::make_unique<podio::ROOTRawData>();
  auto event = podio::Frame(std::move(d), podio::LazyUnpacking{});

  auto event2 = podio::Frame(podio::LazyUnpacking{});

  auto d2 = std::make_unique<podio::ROOTRawData>();
  auto event3 = podio::Frame(std::move(d2), podio::EagerUnpacking{});

  REQUIRE(true);
}
