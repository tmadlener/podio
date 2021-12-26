#include "podio/Frame.h"
#include "podio/ROOTRawData.h"
#include "podio/UnpackingPolicies.h"

#include "catch2/catch_test_macros.hpp"

#include "datamodel/ExampleHitCollection.h"

struct LazyUnpackingDefaults : podio::FrameDefaultPolicies {
  using UnpackingPolicy = podio::LazyUnpacking;
};

TEST_CASE("Frame", "[event][basics]") {
  auto d = std::make_unique<podio::ROOTRawData>();
  auto event = podio::Frame(std::move(d), LazyUnpackingDefaults{});

  auto event2 = podio::Frame(LazyUnpackingDefaults{});

  auto d2 = std::make_unique<podio::ROOTRawData>();
  auto event3 = podio::Frame(std::move(d2));

  auto event4 = podio::Frame();

  REQUIRE(true);
}

TEST_CASE("Frame::put", "[event][basics]") {
  auto event = podio::Frame(LazyUnpackingDefaults{});
  auto hitColl = ExampleHitCollection();
  for (size_t i = 0; i < 10; ++i) {
    auto hit = hitColl.create();
    hit.energy(i);
  }
  auto& hits = event.put(std::move(hitColl), "hits");
  for (size_t i = 0; i < 10; ++i) {
    REQUIRE(hits[i].energy() == i);
  }

  auto& getHits = event.get<ExampleHitCollection>("hits");
  for (size_t i = 0; i < 10; ++i) {
    REQUIRE(getHits[i].energy() == i);
  }
}
